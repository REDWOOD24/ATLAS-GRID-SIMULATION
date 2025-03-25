#include "simple_dispatcher.h"


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
            std::cerr << "Error: Failed to convert 'gflops' to integer. Exception: " << e.what() << std::endl;
        }
    }
    // auto property_map = site->get_properties();
    // for (const auto& i : *property_map) {
    //   std::cout << i.first << "    " << i.second << std::endl;
    // }
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
    Host* best_cpu   = nullptr;
    std::string best_disk;
    double best_score = std::numeric_limits<double>::lowest();

    std::priority_queue<Host*> cpu_queue;
    for (auto* cpu : cpus)
    {
        cpu_queue.push(cpu);
    }

    int candidatesExamined = 0;
    const int maxCandidates = 40;

    while (!cpu_queue.empty() && candidatesExamined < maxCandidates)
    {
        Host* current = cpu_queue.top();
        cpu_queue.pop(); // Popping the element
        ++candidatesExamined;
        if (current->cores_available < j->cores )
        {
            continue;
        }

        // Calculate the weighted score and select a disk.
        std::string current_disk = "";
        // double score = calculateWeightedScore(current, j, current_disk);
        double score = 1;
        // std::cout << "Score "<< score <<std::endl;
        // if (current_disk.empty()) // Not enough disk space.
        // {
        //     continue;
        // }
        size_t total_required_storage = (this->getTotalSize(j->input_files) + this->getTotalSize(j->output_files));
        std::cout << "Total Storage Required" << total_required_storage <<std::endl;
        for (const auto& d : current->disks) {
            // std::cout << "Current Disk Name"<<d->name << std::endl;
            // std::cout << "Current Disk Storage "<<d->storage << std::endl;
            // std::cout << "Disk Storage > Required Storage"<<(d->storage >= total_required_storage) << std::endl;
            if (d->storage >= total_required_storage) 
            {
                current_disk = d->name;
                // std::cout << "Assigning Current Disk "<<d->name << std::endl;

            }
        }
        if (current_disk == ""){
            continue; // unable to find a disk that meets the storage requirements of job
        

        }
        if (score > best_score)
        {
            best_score = score;
            best_cpu = current;
           
        }
        best_cpu = current;
        best_disk = current_disk;
    }

    

    if (best_cpu)
    {
        // Deduct CPU cores and assign job.
        best_cpu->jobs.insert(j->id);
        best_cpu->cores_available -= j->cores;

        // Deduct storage from the chosen disk.
        const auto totalSize = getTotalSize(j->input_files) + getTotalSize(j->output_files);
        for (auto& disk : best_cpu->disks)
        {
            if (disk->name == best_disk)
            {
                disk->storage -= totalSize;
                break;
            }
        }

        j->disk      = best_disk;
        j->comp_host = best_cpu->name;
        std::cout << "Found the best CPU" << best_cpu->name <<std::endl;
        std::cout << "Best CPU Jobs" << best_cpu->jobs.size() <<std::endl;
        std::cout << "Best CPU Speed" <<  best_cpu->speed <<std::endl;
        std::cout << "Disk " <<  j->disk <<std::endl;
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
//   std::cout << " Waiting to assign job resources .........." << std::endl;
  Host*  best_cpu    = nullptr;
  std::cout << " Waiting to assign job resources .........." <<job->comp_site <<std::endl;
  std::string site_name = job->comp_site;
//   std::cout << " Computing Site Name .........." <<site_name<< std::endl;
  auto site = findSiteByName(_sites, site_name);


   // computing the flops with an approximation
//   std::cout << " Gflops .........." <<site->gflops<< std::endl;
//   std::cout << " Jop CPu Consumption time .........." <<job->cpu_consumption_time<< std::endl;
//   std::cout << " Jop Core count .........." <<job->cores<< std::endl;

  job->flops = site->gflops*job->cpu_consumption_time*job->cores;
//   std::cout << " Jop gflops .........." <<job->flops<< std::endl;
//   std::cout << " Site Gflops .........." <<site->gflops<< std::endl;
//   std::cout << " Site CPU Count .........." <<site->cpus.size()<< std::endl;
//   std::cout << " Site CPU Speed .........." <<site->cpus.at(0)->speed<< std::endl;
//   std::cout << " Site CPU Speed .........." <<site->cpus.at(1)->speed<< std::endl;
  best_cpu           = findBestAvailableCPU(site->cpus, job);
  if(best_cpu) {
    site->cpus_in_use++; 
    job->comp_site = site->name; 
    job->status = "assigned"; 
    std::cout << "Job Status changed to assigned" <<std::endl;
}
  else{
  job->status = "pending";
  std::cout << "Job Status changed to pend" <<std::endl;

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
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      std::cout << "\033[32m" << "Submitting .. "          <<  job->id     << std::endl;
      // std::cout << "\033[37m" << "FLOPs to be executed: "  <<  job->flops  << std::endl;
      // std::cout << "\033[33m" << "Files to be read: "      <<  std::endl;
      // for(const auto& file: job->input_files){
      // std::cout << "File: " << std::setw(40) << std::left << file.first
      // << " Size: " << std::setw(10) << std::right << file.second
      // << std::endl;}
      // std::cout << "\033[36m" << "Files to be written: " <<  std::endl;
      // for(const auto& file: job->output_files){
      // std::cout << "File: " << std::setw(40) << std::left << file.first
      // << " Size: " << std::setw(10) << std::right << file.second
      // << std::endl;}
      // std::cout << "\033[35m" << "Cores Used: "  << job->cores      <<  std::endl;
      // std::cout << "\033[0m"  << "Disk Used :  " << job->disk       <<  std::endl;
      // std::cout << "\033[34m" << "Host : "       << job->comp_host  << "\033[0m" << std::endl;

      // std::cout << "----------------------------------------------------------------------" << std::endl;
}

void SIMPLE_DISPATCHER::cleanup()
{
	for(auto& s : _sites){for(auto& h : s->cpus){for(auto&d : h->disks){delete d;}h->disks.clear(); delete h;}s->cpus.clear();delete s;}
	_sites.clear();
}