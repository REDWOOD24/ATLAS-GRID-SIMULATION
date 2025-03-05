#include "job_executor.h"

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

  //Get Job server to dispatch jobs
  const auto* e           = sg4::Engine::get_instance();
  sg4::Host*  job_server  = nullptr;
  const auto  all_hosts   = e->get_all_hosts();
  for (auto& h : all_hosts){if (h->get_name() == std::string("JOB-SERVER_cpu-0")){ job_server = h; break;}}
  if (!job_server){std::cerr << "Job Server Not Initialized properly" << std::endl; exit(-1);}

  //This will create a SimGrid on the Job server to send out all jobs
  auto actor    = sg4::Actor::create("JOB-EXECUTOR-actor", job_server, this->start_server, std::move(jobs));

  //Start Simulation
  e->run();
}

void JOB_EXECUTOR::start_server(JobQueue jobs)
{
  const auto* e = sg4::Engine::get_instance();
  sg4::ActivitySet job_activities;

  while (!jobs.empty())
  {
      saver->saveJob(jobs.top());
      Job* job = dispatcher->assignJob(jobs.top());
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

      auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(e->netzone_by_name_or_null(job->computing_site)).at(job->computing_site+job->comp_host+job->disk+"filesystem");
      update_disk_content(fs,job->input_files,job);
      sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(job->comp_host+"-MQ");
      job_activities.push(mqueue->put_async(job));
      jobs.pop();
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
  }
  job_activities.wait_all();
}


void JOB_EXECUTOR::execute_job(Job* j, sg4::ActivitySet& pending_activities)
{
  //Set Job Status to Running
  j->status = std::string("running");
  saver->updateJob(j);

  //Get Engine Instance
  const auto* e = sg4::Engine::get_instance();
  
  //Find FileSystem to Read and Write (same for now)
  auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(e->netzone_by_name_or_null(j->computing_site)).at(j->computing_site+j->comp_host+j->disk+"filesystem");

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
    if (job->id == "kill"){delete job; pending_activities.wait_all(); break;}
    execute_job(job,pending_activities);
  }
}

void JOB_EXECUTOR::start_receivers()
{
  const auto* eng = sg4::Engine::get_instance();
  auto hosts      = eng->get_all_hosts();
  for(const auto& host: hosts)
    {
    if(host->get_name() == std::string("JOB-SERVER_cpu-0")) continue;
    sg4::Actor::create(host->get_name()+"-actor", host, receiver, host->get_name()+"-MQ");
    }
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
  sg4::Engine::on_simulation_end_cb([]()  {std::cout << "Simulation finished, SIMULATED TIME: " << sg4::Engine::get_clock() << std::endl; dispatcher->onSimulationEnd();});
}