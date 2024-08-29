#include "panda_dispatcher.h"
#include <iostream>

PANDA_DISPATCHER::PANDA_DISPATCHER(sg4::Engine* _e){e = _e;}

void PANDA_DISPATCHER::dispatch_jobs(std::vector<Job>& jobs, sg4::NetZone* platform)
{
  std::vector<Host> cpus;
  std::vector<subJob> subjobs;
  this->getHostsINFO(platform,cpus);
  this->allocateResourcesToSubjobs(cpus,jobs, weights, max_flops_per_subjob, max_storage_per_subjob, subjobs);
  this->update_all_disks_content(subjobs);
  this->create_actors(subjobs);
}

void PANDA_DISPATCHER::getHostsINFO(sg4::NetZone* platform, std::vector<Host>& _cpus)
{
  std::vector<Host> cpus;
  auto sites = platform->get_children();
  for(const auto& site: sites)
    {
    for(const auto& host: site->get_all_hosts())
      {
        Host cpu;
        cpu.name  =  host->get_cname();
        cpu.cores =  host->get_core_count();
        cpu.speed =  host->get_available_speed();

        for(const auto& disk: host->get_disks())
	  {
          disk_params d;
          d.name     = disk->get_cname();
          d.storage  = disk->extension<sg4::FileSystemDiskExt>()->get_size();
          d.read_bw  = disk->get_read_bandwidth();
          d.write_bw = disk->get_write_bandwidth();
          cpu.disks.push_back(d);
	  }
        cpus.push_back(cpu);
      }
    }
  std::move(std::begin(cpus), std::end(cpus), std::back_inserter(_cpus));

}


void PANDA_DISPATCHER::execute_subjob(const subJob s)
{
  //Parse Job Info
  double                         flops         = s.flops;
  std::map<std::string, size_t>  input_files   = s.input_files;
  std::map<std::string, size_t>  output_files  = s.output_files;
  std::string                    read_host     = s.read_host;
  std::string                    comp_host     = s.comp_host;
  std::string                    write_host    = s.write_host;
  int                            cores         = s.cores;
  std::string                    disk          = s.disk;

 
  //Get Engine Instance
  const auto* e = simgrid::s4u::Engine::get_instance();

  //Find Read Disk Mount Point
  std::string read_mount;
  for(const auto& d: e->host_by_name(read_host)->get_disks())
    {if(std::string(d->get_cname()) == disk){read_mount = d->get_property("mount"); break;}}
  if(read_mount.empty()){throw std::runtime_error("Read Disk mount point not found.");}

  //Find Write Disk Mount Point                                                                                                                             
  std::string write_mount;
  for(const auto& d: e->host_by_name(write_host)->get_disks())
    {if(std::string(d->get_cname()) == disk){write_mount = d->get_property("mount"); break;}}
  if(write_mount.empty()){throw std::runtime_error("Write Disk mount point not found.");}
  
  //Define Actions
  std::unique_ptr<Actions> actions = std::make_unique<Actions>();
  
  //Read in files
  for(const auto& inputfile: input_files)
    {
  actions->read(read_mount+inputfile.first,e->host_by_name(s.read_host));
    }

  //Execute FLOPS
  actions->exec_task_multi_thread(flops,cores,comp_host);

  //Write out files
  for(const auto& outputfile: output_files)
    {
  actions->write(write_mount+outputfile.first, outputfile.second, e->host_by_name(s.write_host));
    }

}


void PANDA_DISPATCHER::create_actors(const std::vector<subJob>& subjobs)
{
  for(const auto& s: subjobs)
    {sg4::Actor::create(s.id, e->host_by_name(s.comp_host), this->execute_subjob, s);}
}


void PANDA_DISPATCHER::update_all_disks_content(std::vector<subJob>& subjobs)
{
  for(const auto& subjob: subjobs)
    {
      sg4::Disk* disk = nullptr;
      for(const auto& d: e->host_by_name(subjob.read_host)->get_disks())
	{if(std::string(d->get_cname()) == subjob.disk){disk = d; break;}}
      if(!disk) throw std::runtime_error("Disk not found, while setting input files.");
      this->update_disk_content(disk, this->get_disk_content(subjob.input_files));
    }
}

void PANDA_DISPATCHER::update_disk_content(sg4::Disk* d, const std::string content)
{
  d->set_property("content",content);
  d->seal();
}

std::string PANDA_DISPATCHER::get_disk_content(const std::map<std::string, size_t>& inputMap)
{
    std::string result;
    bool first = true;
    for (const auto& entry : inputMap) {
        if (!first) {result += ",";}
        result += entry.first + ":" + std::to_string(entry.second);
        first = false;}
    return result;
}


double PANDA_DISPATCHER::calculateWeightedScore(Host& cpu, subJob& sj, const std::map<std::string, double>& weights, std::string& best_disk_name)
{
    double score = cpu.speed/1e8 * weights.at("speed") + cpu.cores * weights.at("cores");
    double best_disk_score = std::numeric_limits<double>::lowest();
    size_t total_required_storage = (this->getTotalSize(sj.input_files) + this->getTotalSize(sj.output_files));

    for (const auto& d : cpu.disks) {
        if (d.storage >= total_required_storage) {
	  double disk_score = (d.read_bw/10) * weights.at("disk_read_bw") + (d.write_bw/10) * weights.at("disk_write_bw") + (d.storage/1e10) * weights.at("disk_storage");
            if (disk_score > best_disk_score) {
                best_disk_score = disk_score;
                best_disk_name = d.name;
            }
        }
    }
    score += best_disk_score * weights.at("disk");

    //If job is computation or storage intensive, this should impact the score
    if(sj.flops > 10*max_flops_per_subjob)                  score  += cpu.speed/1e8;
    if(total_required_storage > 50*max_storage_per_subjob)  score  += total_required_storage/1e10;

    return score;
}

double PANDA_DISPATCHER::getTotalSize(const std::map<std::string, size_t>& files)
{
  size_t total_size = 0;
  for (const auto& file : files) {total_size += file.second;}
  return total_size;
}

Host* PANDA_DISPATCHER::findBestAvailableCPU(std::vector<Host>& cpus, subJob& sj, const std::map<std::string, double>& weights) {
    Host* best_cpu = nullptr;
    std::string best_disk;
    double best_score = std::numeric_limits<double>::lowest();

    for (auto& cpu : cpus) {
        if (cpu.is_taken) continue;
        
        std::string current_best_disk;
        double score = calculateWeightedScore(cpu, sj, weights, current_best_disk);
	//std::cout << score << std::endl;
        if (score > best_score) {
            best_score = score;
            best_cpu = &cpu;
            best_disk = current_best_disk;
        }
    }

    if (!best_cpu || best_disk.empty()) {
        throw std::runtime_error("Failed to find a suitable Resources for job.");
    }

    best_cpu->best_disk = best_disk;
    return best_cpu;
}


std::vector<subJob> PANDA_DISPATCHER::splitJobIntoSubjobs(Job& job, size_t max_flops_per_subjob, size_t max_storage_per_subjob) {
    // Start with creating subjobs
    std::vector<subJob> subjobs;

    size_t total_read_size = job.input_storage;
    size_t total_write_size = job.output_storage;
    size_t total_flops = job.flops;

    // Calculate the total number of subjobs needed based on FLOPs and storage requirements
    size_t num_subjobs = std::max(
        static_cast<size_t>(1),
        std::max(
            (static_cast<size_t>(job.flops)) / max_flops_per_subjob,
            (total_read_size + total_write_size) / max_storage_per_subjob
        )+1
    );

    // Calculate FLOPs and storage per subjob
    size_t flops_per_subjob = total_flops / num_subjobs;
    size_t leftover_flops = total_flops % num_subjobs;

    size_t read_storage_per_subjob = (total_read_size) / num_subjobs;
    size_t read_leftover_storage = (total_read_size) % num_subjobs;

    size_t write_storage_per_subjob = (total_write_size) / num_subjobs;
    size_t write_leftover_storage = (total_write_size) % num_subjobs;

      
    std::map<std::string, size_t> read_files = job.input_files;
    std::map<std::string, size_t> write_files = job.output_files;

    for (size_t i = 0; i < num_subjobs; ++i) {
        subJob subjob;
        subjob.id = job.id + "-subJob-" + std::to_string(i);

        // Allocate FLOPs to the subjob
        subjob.flops = flops_per_subjob + (i < leftover_flops ? 1 : 0);  // Distribute leftover FLOPs
        job.flops -= subjob.flops;

        size_t read_current_storage = 0;
        size_t write_current_storage = 0;

	
        // Allocate storage to the subjob (equal distribution)
        size_t read_storage_limit = read_storage_per_subjob + (i < read_leftover_storage ? 1 : 0);
        size_t write_storage_limit = write_storage_per_subjob + (i < write_leftover_storage ? 1 : 0);

        // Allocate read files to the subjob
        for (auto it = read_files.begin(); it != read_files.end();) {
            if (read_current_storage + it->second <= read_storage_limit) {
                subjob.input_files[it->first] = it->second;
                read_current_storage += it->second;
                it = read_files.erase(it);
            } else {
                ++it;
            }
        }

        // Allocate write files to the subjob
        for (auto it = write_files.begin(); it != write_files.end();) {
            if (write_current_storage + it->second <= write_storage_limit) {
                subjob.output_files[it->first] = it->second;
                write_current_storage += it->second;
                it = write_files.erase(it);
            } else {
                ++it;
            }
        }

        subjobs.push_back(subjob);

        // Early exit if all tasks are completed
        if (read_files.empty() && write_files.empty() && job.flops <= 0) {
            break;
        }
    }

    return subjobs;
}


void PANDA_DISPATCHER::allocateResourcesToSubjobs(std::vector<Host>& cpus, std::vector<Job>& jobs, const std::map<std::string, double>& weights, size_t max_flops_per_subjob, size_t max_storage_per_subjob, std::vector<subJob>& all_subjobs) {

  
    for (auto& job : jobs) {
        auto subjobs = splitJobIntoSubjobs(job, max_flops_per_subjob, max_storage_per_subjob);

        for (auto& subjob : subjobs) {
            Host* best_cpu = findBestAvailableCPU(cpus, subjob, weights);

            // Mark the selected CPU and deduct storage from the selected disk
            best_cpu->is_taken = true;
            for(auto& d: best_cpu->disks)
	      {if(d.name == best_cpu->best_disk ) {d.storage -= (this->getTotalSize(subjob.input_files) + this->getTotalSize(subjob.output_files));}}

	    subjob.read_host  =  best_cpu->name;
	    subjob.write_host =	 best_cpu->name;
            subjob.comp_host  =	 best_cpu->name;
            subjob.cores      =  best_cpu->cores;
	    subjob.disk       =  best_cpu->best_disk;

	    this->printJobInfo(subjob);
        }

      std::move(std::begin(subjobs), std::end(subjobs), std::back_inserter(all_subjobs));

    }

} 

void PANDA_DISPATCHER::printJobInfo(subJob& subjob)
{
      std::cout << "----------------------------------------------------------------------" << std::endl;
      std::cout << "Submitting .. "        <<  subjob.id     << std::endl;
      std::cout << "FLOPs to be executed: "    <<  subjob.flops  << std::endl;
      std::cout << "Files to be read: "    <<  std::endl;
      for(const auto& file: subjob.input_files){
      std::cout << "File: " << std::setw(40) << std::left << file.first
      << " Size: " << std::setw(10) << std::right << file.second
      << std::endl;}
      std::cout << "Files to be written: " <<  std::endl;
      for(const auto& file: subjob.output_files){
      std::cout << "File: " << std::setw(40) << std::left << file.first
      << " Size: " << std::setw(10) << std::right << file.second
      << std::endl;}
      std::cout << "Cores Used: " << subjob.cores <<  std::endl;
      std::cout << "Disks Used: " << subjob.disk <<  std::endl;
      std::cout << "Read Host : " << subjob.read_host <<  std::endl;
      std::cout << "Write Host: " << subjob.write_host <<  std::endl;
      std::cout << "Comp Host : " << subjob.comp_host <<  std::endl;

      std::cout << "----------------------------------------------------------------------" << std::endl;

}
