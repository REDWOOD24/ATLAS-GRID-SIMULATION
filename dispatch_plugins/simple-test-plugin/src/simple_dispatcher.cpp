#include "simple_dispatcher.h"


void SIMPLE_DISPATCHER::setPlatform(sg4::NetZone* platform)
{
  std::priority_queue<Site*> site_queue;
  auto all_sites = platform->get_children();
  for(const auto& site: all_sites)
    {
    Site* _site = new Site;
    _site->name = site->get_cname();
    for(const auto& host: site->get_all_hosts())
      {
        Host* cpu = new Host;
        cpu->name            = host->get_cname();
        cpu->cores           = host->get_core_count();
        cpu->speed           = host->get_available_speed();
		cpu->flops_available = cpu->cores*50*1e9;

        for(const auto& disk: host->get_disks())
	  {
          disk_params* d = new disk_params;
          d->name        = disk->get_cname();
          d->mount       = disk->get_property("mount");
          d->storage     = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(site).at(_site->name+cpu->name+d->name+"filesystem")->get_free_space_at_path(d->mount);
          d->read_bw     = disk->get_read_bandwidth();
          d->write_bw    = disk->get_write_bandwidth();
          cpu->disks.push_back(d);
	  }
        _site->cpus.push_back(cpu);

	//Site priority is determined by quality of cpus available
	_site->priority += cpu->speed/1e8 * this->weights.at("speed") + cpu->cores * this->weights.at("cores");
      }
      _site->priority    = std::round(_site->priority/_site->cpus.size()); //Normalize
      _site->cpus_in_use = 0;
      site_queue.push(_site);
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
    Host*          best_cpu       = nullptr;
    std::string    best_disk;
    double         best_score     = std::numeric_limits<double>::lowest();
    int           _search_depth   = 0;
    
    std::priority_queue<Host*> cpu_queue;
    for (const auto& cpu : cpus) { cpu_queue.push(cpu);}
 
    while(!cpu_queue.empty())
      {
	Host* cpu = cpu_queue.top();
        std::string current_best_disk;
        double score = calculateWeightedScore(cpu, j, current_best_disk);
        if (score > best_score && !current_best_disk.empty())
	  {
            best_score          = score;
            best_cpu            = cpu;
	    	best_disk           = current_best_disk;
	    	if(_search_depth++ > 20) break; //Optimization to not loop over too many CPUs
	  }
	cpu_queue.pop();
      }

    if(best_cpu) //Found a CPU. Deduct storage from the selected disk.                                                
      {
	best_cpu->jobs.push_back(j->id);
	best_cpu->flops_available -= j->flops;
	
	for(auto& d: best_cpu->disks)
	  {if(d->name == best_disk){d->storage -= (this->getTotalSize(j->input_files) + this->getTotalSize(j->output_files));}}
	
	j->disk       =  best_disk;
	j->comp_host  =  best_cpu->name;
	j->cores      =  best_cpu->cores;
      }
    
    return best_cpu;
}


Job* SIMPLE_DISPATCHER::assignJobToResource(Job* job) {

  int   current_site_index   = 0;
  Host*  best_cpu    = nullptr;

  for(int i = 0; i < _sites.size(); ++i)
    {
      current_site_index = use_round_robin ? (current_site_index + 1) % _sites.size() : current_site_index;
      auto site          = _sites[current_site_index];
      use_round_robin    = !use_round_robin && (site->cpus_in_use >= site->cpus.size() / 2);
      best_cpu           = findBestAvailableCPU(site->cpus, job);
      if(best_cpu) {site->cpus_in_use++; job->comp_site = site->name; break;}
    }
  if(!best_cpu){throw std::runtime_error("Failed to find suitable Resources for job.");}
  this->printJobInfo(job);
  return job;
  
}

void SIMPLE_DISPATCHER::printJobInfo(Job* job)
{
      std::cout << "----------------------------------------------------------------------" << std::endl;
      std::cout << "\033[32m" << "Submitting .. "          <<  job->id     << std::endl;
      std::cout << "\033[37m" << "FLOPs to be executed: "  <<  job->flops  << std::endl;
      std::cout << "\033[33m" << "Files to be read: "      <<  std::endl;
      for(const auto& file: job->input_files){
      std::cout << "File: " << std::setw(40) << std::left << file.first
      << " Size: " << std::setw(10) << std::right << file.second
      << std::endl;}
      std::cout << "\033[36m" << "Files to be written: " <<  std::endl;
      for(const auto& file: job->output_files){
      std::cout << "File: " << std::setw(40) << std::left << file.first
      << " Size: " << std::setw(10) << std::right << file.second
      << std::endl;}
      std::cout << "\033[35m" << "Cores Used: "  << job->cores      <<  std::endl;
      std::cout << "\033[0m"  << "Disk Used :  " << job->disk       <<  std::endl;
      std::cout << "\033[34m" << "Host : "       << job->comp_host  << "\033[0m" << std::endl;

      std::cout << "----------------------------------------------------------------------" << std::endl;

}

void SIMPLE_DISPATCHER::cleanup()
{
	for(auto& s : _sites){for(auto& h : s->cpus){for(auto&d : h->disks){delete d;}h->disks.clear(); delete h;}s->cpus.clear();delete s;}
	_sites.clear();
}