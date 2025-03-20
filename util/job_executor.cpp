#include "job_executor.h"
#include <chrono>

std::unique_ptr<DispatcherPlugin>   JOB_EXECUTOR::dispatcher;
std::unique_ptr<sqliteSaver>        JOB_EXECUTOR::saver = std::make_unique<sqliteSaver>();

void JOB_EXECUTOR::set_dispatcher(const std::string& dispatcherPath, sg4::NetZone* platform)
{
  PluginLoader<DispatcherPlugin> plugin_loader;
  dispatcher = plugin_loader.load(dispatcherPath);
  dispatcher->getResourceInformation(platform);
}

void JOB_EXECUTOR::set_output(const std::string& outputFile)
{
  std::cout << "Output Path Set To: " << outputFile << std::endl;
  saver->setFilePath(outputFile);

  saver->createJobsTable();
}

void JOB_EXECUTOR::start_job_execution(JobQueue jobs)
{

  //Execute code on Simulation Start/End
  attach_callbacks();
  std::cout << "After callbacks .........." << std::endl;
  //Get Job server to dispatch jobs
  const auto* e           = sg4::Engine::get_instance();
  sg4::Host*  job_server  = nullptr;
  const auto  all_hosts   = e->get_all_hosts();
  for (auto& h : all_hosts){if (h->get_name() == std::string("JOB-SERVER_cpu-0")){ job_server = h; break;}}
  if (!job_server){std::cerr << "Job Server Not Initialized properly" << std::endl; exit(-1);}
  
  //This will create a SimGrid on the Job server to send out all jobs
  auto actor    = sg4::Actor::create("JOB-EXECUTOR-actor", job_server, this->start_server, jobs);

  //Start Simulation
  std::cout << " Start simulation .........." << std::endl;
  e->run();
}

void JOB_EXECUTOR::start_server(JobQueue jobs)
{
  const auto* e = sg4::Engine::get_instance();
  sg4::ActivitySet job_activities;
  std::cout << " Server Started .........." << std::endl;
  while (!jobs.empty())
  {
      saver->saveJob(jobs.top());
      std::cout << " Job Saved .........." << std::endl;
      
      if (dispatcher == nullptr) {
        std::cerr << "Error: `dispatcher` is null!" << std::endl;
        return;
      }
      Job* topJob = jobs.top();
      std::cout << " Job top .........." <<jobs.top()->cores <<std::endl;
      Job* job = dispatcher->assignJob(topJob);
      saver->updateJob(job);
      int retries = 0;
      while (job->status != std::string("assigned"))
      {
        sg4::this_actor::sleep_for(RETRY_INTERVAL);
        dispatcher->assignJob(job);
        saver->updateJob(job);
        if (retries++ > MAX_RETRIES) break;
      };
      if (retries > MAX_RETRIES) {jobs.pop(); continue;}

      auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(e->netzone_by_name_or_null(job->comp_site)).at(job->comp_site+job->comp_host+job->disk+"filesystem");
      update_disk_content(fs,job->input_files,job);
      sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(job->comp_host+"-MQ");
      job_activities.push(mqueue->put_async(job));
      jobs.pop();
      std::cout << " Removed the job from queue" <<std::endl;
    }

  //ShutDown
  auto hosts = e->get_all_hosts();
  for (const auto& host : hosts)
  {
    if(host->get_name() == std::string("JOB-SERVER_cpu-0")) continue;

    Job* job = new Job;
    job->id = "kill";
    sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(host->get_name()+"-MQ");
    job_activities.push(mqueue->put_async(job));
    // std::cout << " Update the kill job" <<std::endl;
  }
  std::cout << " Before wait all" <<std::endl;
  job_activities.wait_all();
  std::cout << " After wait all" <<std::endl;
}


void JOB_EXECUTOR::execute_job(Job* j, sg4::ActivitySet& pending_activities)
{
  //Set Job Status to Running
  std::cout << " Executing Job" <<std::endl;
  j->status = std::string("running");
  saver->updateJob(j);
std::cout << " Finished Executing Job" <<std::endl;
  //Get Engine Instance
  const auto* e = sg4::Engine::get_instance();
  
  //Find FileSystem to Read and Write (same for now)
  auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(e->netzone_by_name_or_null(j->comp_site)).at(j->comp_site+j->comp_host+j->disk+"filesystem");

  //Read, Compute, Write
  Actions::read_file_async(fs,j,pending_activities,dispatcher);
  Actions::exec_task_multi_thread_async(j,pending_activities,saver,dispatcher);
  Actions::write_file_async(fs,j,pending_activities,dispatcher);
  
}

void JOB_EXECUTOR::receiver(const std::string& MQ_name)
{
  sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(MQ_name);
  sg4::ActivitySet pending_activities;

  while (true)
  {
    sg4::MessPtr mess = mqueue->get_async();
    mess->wait(); //Jobs come in one by one, not async
    auto* job = static_cast<Job*>(mess->get_payload());
    if (job->id == "kill"){
      // std::cout << "Deleting the job" << std::endl;
       delete job;
       pending_activities.wait_all(); 
       break;
    }
    std::cout << "Call execute the job" << std::endl;
    execute_job(job,pending_activities);
  }
}

void JOB_EXECUTOR::start_receivers()
{
    auto start = std::chrono::high_resolution_clock::now();
    
    const auto* eng = sg4::Engine::get_instance();
    auto hosts = eng->get_all_hosts();
    
    auto host_fetch_time = std::chrono::high_resolution_clock::now();
    std::cout << "Time to fetch hosts: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(host_fetch_time - start).count() 
              << " ms" << std::endl;
    
    for(const auto& host: hosts)
    {
        if(host->get_name() == "JOB-SERVER_cpu-0")
            continue;
        sg4::Actor::create(host->get_name()+"-actor", host, receiver, host->get_name()+"-MQ");
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Finished Creating the Receivers in: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - host_fetch_time).count() 
              << " ms" << std::endl;
    std::cout << "Total time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
              << " ms" << std::endl;
}


void JOB_EXECUTOR::update_disk_content(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, const std::unordered_map<std::string, size_t>&  input_files, Job* j)
{
  const auto* e = sg4::Engine::get_instance();
  for(const auto& d: e->host_by_name(j->comp_host)->get_disks())
  {if(std::string(d->get_cname()) == j->disk){j->mount = d->get_property("mount"); break;}}
  if(j->mount.empty()){throw std::runtime_error("Read Disk mount point not found.");}
  for(const auto& inputfile: input_files){fs->create_file(j->mount + inputfile.first,  std::to_string(inputfile.second)+"kB");}
}

void JOB_EXECUTOR::saveJobs(JobQueue jobs)
{
  while(!jobs.empty()) {
    Job* j = jobs.top();
    saver->updateJob(j);
    jobs.pop();
    delete j;
  }
}

void JOB_EXECUTOR::attach_callbacks()
{
  sg4::Engine::on_simulation_start_cb([](){std::cout << "Simulation starting .........." << std::endl;});
  sg4::Engine::on_simulation_end_cb([]()  {
    std::cout << "Simulation finished, SIMULATED TIME: " << sg4::Engine::get_clock() << std::endl; 
    dispatcher->onSimulationEnd();
    saver->exportJobsToCSV("/home/sairam/ATLASGRIDV2/ATLAS-GRID-SIMULATION/output/simout.csv"); // TODO: Move this to config 
  });
}