#include "simple_dispatcher.h"
// #include "logger.h"


void SIMPLE_DISPATCHER::setPlatform(sg4::NetZone* platform)
{
  std::priority_queue<Site*> site_queue;
  auto all_sites = platform->get_children();
  for(const auto& site: all_sites)
    {
        if(site->get_name() == std::string("JOB-SERVER")) continue; //No computation on Job server
        Site* _site = new Site;
        _site->name = site->get_cname();
        const char* gflops_str = site->get_property("gflops");
        if (gflops_str) { // Ensure it's not nullptr
            try {
                _site->gflops = std::stol(gflops_str);
            } catch (const std::exception& e) {
                // LOG_ERROR("Error: Failed to convert 'gflops' to integer. Exception: {}", e.what());
            }
        }
        for(const auto& host: site->get_all_hosts())
        {
            Host* cpu = new Host;
            cpu->name            = host->get_cname();
            cpu->cores           = host->get_core_count();
            cpu->speed           = host->get_speed();
            cpu->cores_available = host->get_core_count(); 
            for(const auto& disk: host->get_disks())
            {
            Disk* d        = new Disk;
            d->name        = disk->get_cname();
            d->mount       = disk->get_property("mount");
            d->storage     = (simgrid::fsmod::FileSystem::get_file_systems_by_netzone(site).at(_site->name+cpu->name+d->name+"filesystem")->get_free_space_at_path(d->mount))/1000;
            d->read_bw     = disk->get_read_bandwidth();
            d->write_bw    = disk->get_write_bandwidth();
            cpu->disks.push_back(d);
            cpu->disks_map[d->name] =d;
            }
            _site->cpus.push_back(cpu);
            _site->cpus_map[cpu->name] = cpu;

            //Site priority is determined by quality of cpus available
            _site->priority += cpu->speed/1e8 * this->weights.at("speed") + cpu->cores * this->weights.at("cores");
        }
        _site->priority    = std::round(_site->priority/_site->cpus.size()); //Normalize
        _site->cpus_in_use = 0;
        site_queue.push(_site);
        _sites_map[_site->name] = _site;
    }

  while (!site_queue.empty()) {_sites.push_back(site_queue.top()); site_queue.pop();}
}



double SIMPLE_DISPATCHER::calculateWeightedScore(Host* cpu, Job* j, std::string& best_disk_name)
{
    double score = cpu->speed/1e8 * weights.at("speed") + cpu->cores * weights.at("cores");
    double best_disk_score = std::numeric_limits<double>::lowest();
    size_t total_required_storage = (this->getTotalSize(j->input_files) + this->getTotalSize(j->output_files));
    for (const auto& d : cpu->disks) {
        if (d->storage >= total_required_storage) {
	  double disk_score = (d->read_bw/10) * weights.at("disk_read_bw") + (d->write_bw/10) * weights.at("disk_write_bw") + (d->storage/1e10) * weights.at("disk_storage");
            if (disk_score > best_disk_score) {
                best_disk_score = disk_score;
                best_disk_name = d->name;
            }
        }
    }
    score += best_disk_score * weights.at("disk");
    return score;
}

double SIMPLE_DISPATCHER::getTotalSize(const std::unordered_map<std::string, size_t>& files)
{
  size_t total_size = 0;
  for (const auto& file : files) {total_size += file.second;}
  return total_size;
}

Host* SIMPLE_DISPATCHER::findBestAvailableCPU(std::vector<Host*>& cpus, Job* j)
{
    Host* best_cpu = nullptr;
    std::string best_disk;
    double best_score = std::numeric_limits<double>::lowest();

    // Create a priority queue from the CPU candidates.
    std::priority_queue<Host*> cpu_queue;
    for (auto* cpu : cpus)
    {
        if (!cpu) {
            LOG_DEBUG("Warning: Encountered a null CPU pointer.");
            continue;
        }
        cpu_queue.push(cpu);
    }

    int candidatesExamined = 0;
    const int maxCandidates = 40;

    while (!cpu_queue.empty() && candidatesExamined < maxCandidates)
    {
        Host* current = cpu_queue.top();
        cpu_queue.pop();
        ++candidatesExamined;

        if (current->cores_available < j->cores)
        {
            continue;
        }

        // For now, using a dummy score.
        double score = 1;
        std::string current_disk = "";
        size_t total_required_storage = this->getTotalSize(j->input_files) 
                                        + this->getTotalSize(j->output_files);

        for (const auto& d : current->disks) {
            if (d->storage >= total_required_storage) {
                current_disk = d->name;
                break;
            }
        }
        if (current_disk == "") {
            continue;
        }
        if (score > best_score)
        {
            best_score = score;
            best_cpu = current;
            best_disk = current_disk;
        }
        // NOTE: Original code had additional assignments that may be unintended;
        // they are commented out here.
        // best_cpu = current;
        // best_disk = current_disk;
    }

    if (best_cpu)
    {
        // Deduct CPU cores and assign job.
        best_cpu->jobs.insert(j->id);
        best_cpu->cores_available -= j->cores;

        // Deduct storage from the chosen disk.
        const auto totalSize = this->getTotalSize(j->input_files) 
                             + this->getTotalSize(j->output_files);
        for (auto& disk : best_cpu->disks)
        {
            if (disk->name == best_disk)
            {
                disk->storage -= totalSize;
                break;
            }
        }
        j->disk = best_disk;
        j->comp_host = best_cpu->name;
    }
    else {
        LOG_DEBUG("Could not find a suitable CPU for job {}", j->jobid );
    }

    return best_cpu;
}




// Job* SIMPLE_DISPATCHER::assignJobToResource(Job* job)
// {
//   Host*  best_cpu    = nullptr;

//   for(int i = 0; i < _sites.size(); ++i)
//     {
//       current_site_index = use_round_robin ? (current_site_index + 1) % _sites.size() : current_site_index;
//       auto site          = _sites[current_site_index];
//       //Add new metric called Site pressure site->num_of_jobs_cores/site->num_of_cpus_cores, if pressure > 80% skip
//       use_round_robin    = !use_round_robin && (site->cpus_in_use >= site->cpus.size() / 2);
//       best_cpu           = findBestAvailableCPU(site->cpus, job);
//       if(best_cpu) {site->cpus_in_use++; job->comp_site = site->name; job->status = "assigned"; break;}
//     }
//   if(!best_cpu){job->status = "pending";}
//   this->printJobInfo(job);
//   return job;
  
// }

Job* SIMPLE_DISPATCHER::assignJobToResource(Job* job)
{
  Host*  best_cpu    = nullptr;
  
  LOG_DEBUG(" Waiting to assign job resources : {}", job->comp_site);
  std::string site_name = job->comp_site;
  auto site = findSiteByName(_sites, site_name);
  LOG_DEBUG(" Found the site {}", job->comp_site);
  if (job == nullptr) {
    LOG_DEBUG("JOB pointer null");
       
    }
    if (site == nullptr) {
        job->status = "failed";
        LOG_DEBUG("Computing Site is not found: Site pointer NULL :{}", job->comp_site);
        return job;
    }
  job->flops = site->gflops*job->cpu_consumption_time*job->cores;
  best_cpu           = findBestAvailableCPU(site->cpus, job);
  if(best_cpu) {
    site->cpus_in_use++; 
    job->comp_site = site->name; 
    job->status = "assigned"; 
    LOG_DEBUG("Job Status changed to assigned");
}
  else{
  job->status = "pending";
  LOG_DEBUG("Job Status changed to pend");

  }
  this->printJobInfo(job);
  return job;
  
}


void SIMPLE_DISPATCHER::free(Job* job)
{
 Host* cpu               = _sites_map.at(job->comp_site)->cpus_map.at(job->comp_host);
 if(cpu->jobs.count(job->id) > 0)
 {
   Disk* disk              = cpu->disks_map.at(job->disk);
   cpu->cores_available   += job->cores;
   disk->storage          += (this->getTotalSize(job->input_files) + this->getTotalSize(job->output_files));
   cpu->jobs.erase(job->id);
 }
}


Site* SIMPLE_DISPATCHER::findSiteByName(std::vector<Site*>& sites, const std::string& site_name) {
  auto it = std::find_if(sites.begin(), sites.end(),
                         [&site_name](Site* site) {

                             return site->name == site_name;
                         });
  return it != sites.end() ? *it : nullptr;
}



void SIMPLE_DISPATCHER::printJobInfo(Job* job)
{
    LOG_DEBUG("----------------------------------------------------------------------");
    LOG_INFO("Submitting .. {}", job->id);
    LOG_DEBUG("FLOPs to be executed: {}", job->flops);
    LOG_DEBUG("Files to be read:");
    for (const auto& file : job->input_files) {
        LOG_DEBUG("File: {:<40} Size: {:>10}", file.first, file.second);
    }
    LOG_DEBUG("Files to be written:");
    for (const auto& file : job->output_files) {
        LOG_DEBUG("File: {:<40} Size: {:>10}", file.first, file.second);
    }
    LOG_DEBUG("Cores Used: {}", job->cores);
    LOG_DEBUG("Disk Used: {}", job->disk);
    LOG_DEBUG("Host: {}", job->comp_host);
    LOG_DEBUG("----------------------------------------------------------------------");
}

void SIMPLE_DISPATCHER::cleanup()
{
	for(auto& s : _sites){for(auto& h : s->cpus){for(auto&d : h->disks){delete d;}h->disks.clear(); delete h;}s->cpus.clear();delete s;}
	_sites.clear();
}