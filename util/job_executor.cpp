#include "job_executor.h"

std::unique_ptr<DispatcherPlugin>                  JOB_EXECUTOR::dispatcher;
std::unique_ptr<sqliteSaver> JOB_EXECUTOR::saver = std::make_unique<sqliteSaver>();


void JOB_EXECUTOR::set_dispatcher(const std::string& dispatcherPath, sg4::NetZone* platform)
{
  PluginLoader<DispatcherPlugin> plugin_loader;
  dispatcher = plugin_loader.load(dispatcherPath);
  dispatcher->assignResources(platform);
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

  //Start Simulation
  const auto* e = sg4::Engine::get_instance();
  auto host    = e->get_all_hosts()[0]; //Panda server
  auto actor = sg4::Actor::create("JOB-EXECUTOR-actor", host, this->execute_jobs, std::move(jobs));
  e->run();
}

void JOB_EXECUTOR::execute_jobs(JobQueue jobs)
{
  const auto* e = sg4::Engine::get_instance();
  while (!jobs.empty())
    {
      Job* job = dispatcher->assignJob(jobs.top());
      auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(e->netzone_by_name_or_null(job->comp_site)).at(job->comp_site+job->comp_host+job->disk+"filesystem");
      update_disk_content(fs,job->input_files,job);
      sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(job->comp_host+"-MQ");
      sg4::MessPtr mess = mqueue->put_async(job);
      mess->wait();
      jobs.pop();
    }

  //ShutDown
  auto hosts = e->get_all_hosts();
  for (const auto& host : hosts)
  {
    Job* job = new Job;
    job->id = "kill";
    sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(host->get_name()+"-MQ");
    sg4::MessPtr mess = mqueue->put_async(job);
    mess->wait();
  }

}


void JOB_EXECUTOR::execute_job(Job* j)
{
  //Get Engine Instance
  const auto* e = sg4::Engine::get_instance();
  
  //Find FileSystem to Read and Write (same for now)
  auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(e->netzone_by_name_or_null(j->comp_site)).at(j->comp_site+j->comp_host+j->disk+"filesystem");

  //Activities (Read, Compute, Write)
  std::vector<sg4::ActivityPtr> activities;

  //Read
  for(const auto& inputfile: j->input_files){activities.push_back(Actions::read_file_async(fs,j->mount+inputfile.first,j));}

  //Compute
  long long flops = j->flops;
  activities.push_back(Actions::exec_task_multi_thread_async(flops,j->cores,j));

  //Write
  for(const auto& outputfile: j->output_files){activities.push_back(Actions::write_file_async(fs,j->mount+outputfile.first,outputfile.second,j));}

  //Wait to finish
  sg4::ActivitySet pending_activities(activities);
  pending_activities.wait_all(); //Still not right, one job at a time.. Add this outside this function

}

void JOB_EXECUTOR::receiver(const std::string& MQ_name)
{
  sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(MQ_name);
  for (bool cont = true; cont;)
  {
    sg4::MessPtr mess = mqueue->get_async();
    mess->wait();
    auto* job = static_cast<Job*>(mess->get_payload());
    if (job->id == "kill"){delete job; break;}
    execute_job(job);
  }
}

void JOB_EXECUTOR::start_receivers()
{
  const auto* eng = sg4::Engine::get_instance();
  auto hosts      = eng->get_all_hosts();
  for(const auto& host: hosts){sg4::Actor::create(host->get_name()+"-actor", host, receiver, host->get_name()+"-MQ");}
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
    saver->saveJob(j);
    jobs.pop();
    delete j;
  }
}

void JOB_EXECUTOR::attach_callbacks()
{
  sg4::Engine::on_simulation_start_cb([](){std::cout << "Simulation starting .........." << std::endl;});
  sg4::Engine::on_simulation_end_cb([](){std::cout << "Simulation finished, SIMULATED TIME: " << sg4::Engine::get_clock() << std::endl; dispatcher->onSimulationEnd();});
}