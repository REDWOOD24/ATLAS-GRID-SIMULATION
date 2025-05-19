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
// std::vector<Job*> JOB_EXECUTOR::pending_jobs;
sg4::ActivitySet JOB_EXECUTOR::pending_activities;

void JOB_EXECUTOR::set_dispatcher(const std::string& dispatcherPath, sg4::NetZone* platform)
{
  PluginLoader<DispatcherPlugin> plugin_loader;
  dispatcher = plugin_loader.load(dispatcherPath);

  // printNetZone(platform);
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

    LOG_INFO("Server started. Initial jobs count: {}", jobs.size());


    // Transfer all jobs from the queue into a vector for central polling.
    std::vector<Job*> pending_jobs;
    while (!jobs.empty()) {
        Job* job = jobs.top();
        jobs.pop();
        saver->saveJob(job);
        // LOG_INFO("Job saved to DB: {}", job->id);

        // Attempt a one‑time assignment.
        Job* result = dispatcher->assignJob(job);
        saver->updateJob(result);
        if (result->status == "assigned") {
            // If assigned immediately, dispatch it.
            auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(
                          e->netzone_by_name_or_null(job->comp_site))
                          .at(job->comp_site + job->comp_host + job->disk + "filesystem");
            update_disk_content(fs, job->input_files, job);
            sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(job->comp_host + "-MQ");
            job_activities.push(mqueue->put_async(job));
            LOG_DEBUG("Job {} dispatched immediately to host {}", job->id, job->comp_host);
        } else {
            // Set status and add to pending list.
            // 
            job->status = "pending";
            pending_jobs.push_back(job);
        }
    }
    // if (!JOB_EXECUTOR::pending_jobs.empty()) {
    //     LOG_INFO("No Cores available. Suspending server...");
    //     job_activities.wait_all();
    //     JOB_EXECUTOR::suspend_server();
    //     LOG_INFO("Server suspended. Pending jobs count: {}", JOB_EXECUTOR::pending_jobs.size());
    // }
    // suspend the server

    std::cout << "Simulator time Before retrying pending jobs" << sg4::Engine::get_clock() << std::endl;
    // Use a retry counter map for each pending job.
    std::unordered_map<Job*, int> retry_counts;
    for (Job* job : pending_jobs) {
        retry_counts[job] = 0;
    }

    // Poll the pending jobs list until none remain.
    while (!pending_jobs.empty()) {
        for (auto it = pending_jobs.begin(); it != pending_jobs.end(); ) {
            Job* job = *it;
            dispatcher->assignJob(job);
            saver->updateJob(job);
            retry_counts[job]++;
            if (job->status == "assigned") {
                auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(
                              e->netzone_by_name_or_null(job->comp_site))
                              .at(job->comp_site + job->comp_host + job->disk + "filesystem");
                update_disk_content(fs, job->input_files, job);
                sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(job->comp_host + "-MQ");
                job_activities.push(mqueue->put_async(job));
                LOG_DEBUG("Job {} dispatched after {} retries to host {}", job->id, retry_counts[job], job->comp_host);
                it = pending_jobs.erase(it);
            }
            //  else if (retry_counts[job] > MAX_RETRIES) {
            //     LOG_DEBUG("Job {} exhausted max retries. Removing from pending jobs.", job->id);
            //     it = pending_jobs.erase(it);
            // } 
            else {
                ++it;
            }
        }
        // Yield control so that other asynchronous events (e.g. receivers freeing resources) can occur.
        // std::cout << "Simulator time Before Sleep" << sg4::Engine::get_clock() << std::endl;
        // sg4::this_actor::sleep_for(RETRY_INTERVAL);
        if (!JOB_EXECUTOR::pending_activities.empty()) {
              LOG_INFO("Pending Activities count: {}", JOB_EXECUTOR::pending_activities.size());
              
              sg4::ActivityPtr activityPtr =  JOB_EXECUTOR::pending_activities.wait_any();
              LOG_INFO("Updated Pending Activities count: {}", JOB_EXECUTOR::pending_activities.size());
              LOG_INFO("Activity completed: {}", activityPtr->get_name());
        }
        LOG_INFO("Pending jobs count: {}", pending_jobs.size());
        // std::cout << "Simulator time After Sleep" << sg4::Engine::get_clock() << std::endl;
    }
    
    // Shutdown: send kill messages to all hosts (except the job server).
    for (const auto& host : e->get_all_hosts()) {
        if (host->get_name() == "JOB-SERVER_cpu-0") continue;
        Job* killJob = new Job;
        killJob->id = "kill";
        sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(host->get_name() + "-MQ");
        job_activities.push(mqueue->put_async(killJob));
    }
    LOG_INFO("Waiting for all job activities to complete...");
    // while(){


    // }
    // JOB_EXECUTOR::pending_activities.wait_all();
    // LOG_INFO("All pending job activities completed. Exiting server.");
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
  // sg4::this_actor::get_host()->extension<HostExtensions>()->registerJob(j);  
  Actions::read_file_async(fs, j, pending_activities, dispatcher);
  Actions::exec_task_multi_thread_async(j, pending_activities, saver, dispatcher);
  Actions::write_file_async(fs, j, pending_activities, dispatcher);

  // pending_activities.wait_all();
  
}

void JOB_EXECUTOR::receiver(const std::string& MQ_name)
{
  sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(MQ_name);
  // sg4::ActivitySet pending_activities;
  while (true) {
    sg4::MessPtr mess = mqueue->get_async();
    mess->wait();
    auto* job = static_cast<Job*>(mess->get_payload());

    if (job->id == "kill") {
      delete job;
      JOB_EXECUTOR::pending_activities.wait_all();
      break;
    }

    LOG_DEBUG("Received job on queue {}: {}", MQ_name, job->id);
    execute_job(job, JOB_EXECUTOR::pending_activities);
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
