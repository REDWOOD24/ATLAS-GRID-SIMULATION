#include "job_executor.h"
#include "logger.h"  
#include <chrono>

// Recursive function to print a NetZone and its children.
void printNetZone(const simgrid::s4u::NetZone* zone, int indent = 0) {
  if (!zone) return;

  std::string indentStr(indent, ' ');
  LOG_DEBUG("{}Zone Name: {}", indentStr, zone->get_name());

  const std::vector<simgrid::s4u::NetZone*>& children = zone->get_children();
  if (!children.empty()) {
    LOG_DEBUG("{}Children:", indentStr);
    for (const auto child : children) {
      printNetZone(child, indent + 2);
    }
  } else {
    LOG_DEBUG("{}No children.", indentStr);
  }
}

std::unique_ptr<DispatcherPlugin> JOB_EXECUTOR::dispatcher;
std::unique_ptr<sqliteSaver> JOB_EXECUTOR::saver = std::make_unique<sqliteSaver>();

void JOB_EXECUTOR::set_dispatcher(const std::string& dispatcherPath, sg4::NetZone* platform)
{
  PluginLoader<DispatcherPlugin> plugin_loader;
  dispatcher = plugin_loader.load(dispatcherPath);

  printNetZone(platform);
  dispatcher->getResourceInformation(platform);
}

void JOB_EXECUTOR::set_output(const std::string& outputFile)
{
  LOG_INFO("Output path set to: {}", outputFile);
  saver->setFilePath(outputFile);
  saver->createJobsTable();
}

void JOB_EXECUTOR::start_job_execution(JobQueue jobs)
{
  attach_callbacks();
  LOG_INFO("Callbacks attached. Starting job execution...");
  const auto* e = sg4::Engine::get_instance();
  sg4::Host* job_server = nullptr;
  for (auto& h : e->get_all_hosts()) {
    if (h->get_name() == "JOB-SERVER_cpu-0") {
      job_server = h;
      break;
    }
  }

  if (!job_server) {
    LOG_CRITICAL("Job Server not initialized properly.");
    exit(-1);
  }

  sg4::Actor::create("JOB-EXECUTOR-actor", job_server, this->start_server, jobs);
   
  e->run();
}

void JOB_EXECUTOR::start_server(JobQueue jobs)
{
  const auto* e = sg4::Engine::get_instance();
  sg4::ActivitySet job_activities;

  LOG_INFO("Server started.");
  while (!jobs.empty())
  {
    saver->saveJob(jobs.top());
    LOG_DEBUG("Job saved to DB: {}", jobs.top()->id);

    if (!dispatcher) {
      LOG_CRITICAL("Dispatcher is null!");
      return;
    }

    Job* topJob = jobs.top();
    LOG_DEBUG("Top job cores: {}", topJob->cores);

    Job* job = dispatcher->assignJob(topJob);
    saver->updateJob(job);

    int retries = 0;
    while (job->status != "assigned") {
      sg4::this_actor::sleep_for(RETRY_INTERVAL);
      dispatcher->assignJob(job);
      saver->updateJob(job);
      if (++retries > MAX_RETRIES) break;
    }
    if (retries > MAX_RETRIES) {
      jobs.pop();
      continue;
    }

    LOG_DEBUG("Calling FileSystem for job: {}", job->id);
    LOG_DEBUG("NetZone: {}", e->netzone_by_name_or_null(job->comp_site)->get_name());
    LOG_DEBUG("FileSystem key: {}", job->comp_site + job->comp_host + job->disk + "filesystem");

    auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(
      e->netzone_by_name_or_null(job->comp_site)).at(job->comp_site + job->comp_host + job->disk + "filesystem");

    update_disk_content(fs, job->input_files, job);

    sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(job->comp_host + "-MQ");
    job_activities.push(mqueue->put_async(job));

    jobs.pop();
    LOG_DEBUG("Removed job from queue: {}", job->id);
  }

  // Shutdown: send kill jobs
  for (const auto& host : e->get_all_hosts()) {
    if (host->get_name() == "JOB-SERVER_cpu-0") continue;
    Job* killJob = new Job;
    killJob->id = "kill";
    sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(host->get_name() + "-MQ");
    job_activities.push(mqueue->put_async(killJob));
  }

  LOG_INFO("Waiting for all job activities to complete...");
  job_activities.wait_all();
  LOG_INFO("All job activities completed.");
}

void JOB_EXECUTOR::execute_job(Job* j, sg4::ActivitySet& pending_activities)
{
  LOG_DEBUG("Executing job: {}", j->id);
  j->status = "running";
  saver->updateJob(j);

  const auto* e = sg4::Engine::get_instance();
  auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(
    e->netzone_by_name_or_null(j->comp_site)).at(j->comp_site + j->comp_host + j->disk + "filesystem");

  Actions::read_file_async(fs, j, pending_activities, dispatcher);
  Actions::exec_task_multi_thread_async(j, pending_activities, saver, dispatcher);
  Actions::write_file_async(fs, j, pending_activities, dispatcher);
}

void JOB_EXECUTOR::receiver(const std::string& MQ_name)
{
  sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(MQ_name);
  sg4::ActivitySet pending_activities;

  while (true) {
    sg4::MessPtr mess = mqueue->get_async();
    mess->wait();
    auto* job = static_cast<Job*>(mess->get_payload());

    if (job->id == "kill") {
      delete job;
      pending_activities.wait_all();
      break;
    }

    LOG_DEBUG("Received job on queue {}: {}", MQ_name, job->id);
    execute_job(job, pending_activities);
  }
}

void JOB_EXECUTOR::start_receivers()
{
  auto start = std::chrono::high_resolution_clock::now();

  const auto* eng = sg4::Engine::get_instance();
  auto hosts = eng->get_all_hosts();

  auto host_fetch_time = std::chrono::high_resolution_clock::now();
  LOG_INFO("Time to fetch hosts: {} ms",
           std::chrono::duration_cast<std::chrono::milliseconds>(host_fetch_time - start).count());

  for (const auto& host : hosts) {
    if (host->get_name() == "JOB-SERVER_cpu-0") continue;
    sg4::Actor::create(host->get_name() + "-actor", host, receiver, host->get_name() + "-MQ");
  }

  auto end = std::chrono::high_resolution_clock::now();
  LOG_INFO("Finished creating receivers in: {} ms",
           std::chrono::duration_cast<std::chrono::milliseconds>(end - host_fetch_time).count());
  LOG_INFO("Total receiver setup time: {} ms",
           std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

void JOB_EXECUTOR::update_disk_content(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, const std::unordered_map<std::string, size_t>& input_files, Job* j)
{
  const auto* e = sg4::Engine::get_instance();
  for (const auto& d : e->host_by_name(j->comp_host)->get_disks()) {
    if (std::string(d->get_cname()) == j->disk) {
      j->mount = d->get_property("mount");
      break;
    }
  }
  if (j->mount.empty()) {
    throw std::runtime_error("Read disk mount point not found for job: " + j->id);
  }

  for (const auto& inputfile : input_files) {
    fs->create_file(j->mount + inputfile.first, std::to_string(inputfile.second) + "kB");
  }
}

void JOB_EXECUTOR::saveJobs(JobQueue jobs)
{
  while (!jobs.empty()) {
    Job* j = jobs.top();
    saver->updateJob(j);
    jobs.pop();
    delete j;
  }
}

void JOB_EXECUTOR::attach_callbacks()
{
  sg4::Engine::on_simulation_start_cb([]() {
    LOG_INFO("Simulation starting...");
  });

  sg4::Engine::on_simulation_end_cb([]() {
    LOG_INFO("Simulation finished, SIMULATED TIME: {}", sg4::Engine::get_clock());
    dispatcher->onSimulationEnd();
    saver->exportJobsToCSV();
  });
}
