#include "panda_dispatcher.h"
#include <iostream>


PANDA_DISPATCHER::PANDA_DISPATCHER(sg4::Engine* _e, const std::string& _outputFile){e = _e; outputFile = _outputFile;}

H5::H5File    PANDA_DISPATCHER::h5_file;
H5::CompType  PANDA_DISPATCHER::datatype;


void PANDA_DISPATCHER::h5init()
{
  h5_file = H5::H5File(this->outputFile.c_str() , H5F_ACC_TRUNC);
  datatype = sizeof(output);
  datatype.insertMember("ID",                  HOFFSET(output, id),                 H5::StrType(H5::PredType::C_S1, H5T_VARIABLE)); 
  datatype.insertMember("FLOPS EXECUTED",      HOFFSET(output,flops_exec),               H5::PredType::NATIVE_UINT);
  datatype.insertMember("FILES READ SIZE",     HOFFSET(output,files_read_size),     H5::PredType::NATIVE_UINT);
  datatype.insertMember("FILES WRITTEN SIZE",  HOFFSET(output,files_written_size),  H5::PredType::NATIVE_UINT);
  datatype.insertMember("READ IO TIME",        HOFFSET(output,read_IO_time),        H5::PredType::NATIVE_FLOAT);
  datatype.insertMember("WRITE IO TIME",       HOFFSET(output,write_IO_time),       H5::PredType::NATIVE_FLOAT);
  datatype.insertMember("FLOPS EXEC TIME",     HOFFSET(output,flops_exec_time),     H5::PredType::NATIVE_FLOAT);
}

void PANDA_DISPATCHER::dispatch_jobs(JobQueue& jobs, sg4::NetZone* platform)
{
  std::vector<Site*>  sites;
  std::set<Host*>     hosts_with_jobs;

  this->h5init();
  this->getHostsINFO(platform,sites);
  this->allocateResourcesToSubjobs(sites,jobs, weights, this->max_flops_per_subjob, this->max_storage_per_subjob, hosts_with_jobs);
  this->update_all_disks_content(hosts_with_jobs);
  this->create_actors(hosts_with_jobs);
  this->cleanup(sites);
}

void PANDA_DISPATCHER::getHostsINFO(sg4::NetZone* platform, std::vector<Site*>& _sites)
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
          d->storage     = disk->extension<sg4::FileSystemDiskExt>()->get_size();
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


void PANDA_DISPATCHER::execute_subjob(const std::vector<subJob*>& subjobs)
{

  std::vector<output> outputs;
  for(const auto& s : subjobs)
    {
      //Parse Job Info
      std::string                    id            = s->id;
      int                            flops         = s->flops;
      std::map<std::string, size_t>  input_files   = s->input_files;
      std::map<std::string, size_t>  output_files  = s->output_files;
      std::string                    read_host     = s->comp_host;
      std::string                    comp_host     = s->comp_host;
      std::string                    write_host    = s->comp_host;
      int                            cores         = s->cores;
      std::string                    disk          = s->disk;
      delete                         s;

      output o;
      o.id = id.c_str();

      
      //Get Engine Instance
      const auto* e = sg4::Engine::get_instance();
      
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
      size_t read_size = 0;
      float  start =  sg4::Engine::get_clock();
      for(const auto& inputfile: input_files)
	{
	  read_size += actions->read(read_mount+inputfile.first,e->host_by_name(read_host));
	}
      o.files_read_size     = read_size;
      o.read_IO_time        = sg4::Engine::get_clock() - start;
      
      //Execute FLOPS
      int flops_exec = 0;
      start =  sg4::Engine::get_clock();
      flops_exec += actions->exec_task_multi_thread(flops,cores,comp_host);
      o.flops_exec          = flops_exec;
      o.flops_exec_time     = sg4::Engine::get_clock() - start;

      //Write out files
      size_t write_size = 0;
      start =  sg4::Engine::get_clock();
      for(const auto& outputfile: output_files)
	{
	  write_size += actions->write(write_mount+outputfile.first, outputfile.second, e->host_by_name(write_host));
	}
      o.files_written_size  = write_size;
      o.write_IO_time       = sg4::Engine::get_clock() - start;
      
      //Save
      outputs.push_back(o);

      //Update Progress
      std::cout << ".";
    }

  hsize_t numberOfOutputs= outputs.size();
  H5::DataSpace dataspace(1,&numberOfOutputs,nullptr);
  const std::string datasetName="HOST-"+std::string(sg4::this_actor::get_host()->get_cname());
  H5::DataSet dataset=h5_file.createDataSet(datasetName, datatype, dataspace);
  dataset.write(outputs.data(), datatype);
  outputs.clear();

}


void PANDA_DISPATCHER::create_actors(const std::set<Host*>& hosts_with_jobs)
{
  for(const auto& h: hosts_with_jobs)
    {sg4::Actor::create(h->name+"-SUBJOBS-INFO", e->host_by_name(h->name), this->execute_subjob, h->subjobs);}
}

void PANDA_DISPATCHER::cleanup(std::vector<Site*>& sites)
{
  for(auto& s : sites){for(auto& h : s->cpus){for(auto&d : h->disks){delete d;}h->disks.clear(); delete h;}s->cpus.clear();delete s;}
  sites.clear();
}

void PANDA_DISPATCHER::update_all_disks_content(const std::set<Host*>& hosts_with_jobs)
{
  for(const auto& h: hosts_with_jobs)
    {
      for(const auto& s: h->subjobs)
      {
	sg4::Disk* disk = nullptr;
	for(const auto& d: e->host_by_name(h->name)->get_disks())
	  {if(std::string(d->get_cname()) == s->disk){disk = d; break;}}
	if(!disk) throw std::runtime_error("Disk not found, while setting input files.");
	this->update_disk_content(disk, this->get_disk_content(s->input_files));
      }
    }
}

void PANDA_DISPATCHER::update_disk_content(sg4::Disk* d, const std::string& content)
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


double PANDA_DISPATCHER::calculateWeightedScore(Host* cpu, subJob* sj, const std::map<std::string, double>& weights, std::string& best_disk_name)
{
    double score = cpu->speed/1e8 * weights.at("speed") + cpu->cores * weights.at("cores");
    double best_disk_score = std::numeric_limits<double>::lowest();
    size_t total_required_storage = (this->getTotalSize(sj->input_files) + this->getTotalSize(sj->output_files));

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

    //If job is computation or storage intensive, this should impact the score
    if(sj->flops > 10*this->max_flops_per_subjob)                  score  += cpu->speed/1e8;
    if(total_required_storage > 50*this->max_storage_per_subjob)   score  += total_required_storage/1e10;

    return score;
}

double PANDA_DISPATCHER::getTotalSize(const std::map<std::string, size_t>& files)
{
  size_t total_size = 0;
  for (const auto& file : files) {total_size += file.second;}
  return total_size;
}

Host* PANDA_DISPATCHER::findBestAvailableCPU(std::vector<Host*>& cpus, subJob* sj, const std::map<std::string, double>& weights)
{
    Host*          best_cpu       = nullptr;
    std::string    best_disk;
    double         best_score     = std::numeric_limits<double>::lowest();
    int           _search_depth   = 0; //Optimization to not loop over too many CPUs
    
    std::priority_queue<Host*> cpu_queue;
    for (const auto& cpu : cpus) { cpu_queue.push(cpu);}
 
    while(!cpu_queue.empty())
      {
	Host* cpu = cpu_queue.top();
        std::string current_best_disk;
        double score = calculateWeightedScore(cpu, sj, weights, current_best_disk);
        if (score > best_score && !current_best_disk.empty())
	  {
            best_score          = score;
            best_cpu            = cpu;
	    best_disk           = current_best_disk;
	    if(_search_depth++ > 20) break; 
	  }
	cpu_queue.pop();
      }

    if(best_cpu) //Found a CPU. Deduct storage from the selected disk.                                                
      {
	best_cpu->subjobs.push_back(sj);
	best_cpu->flops_available -= sj->flops;
	
	for(auto& d: best_cpu->disks)
	  {if(d->name == best_disk){d->storage -= (this->getTotalSize(sj->input_files) + this->getTotalSize(sj->output_files));}}
	
	sj->disk       =  best_disk;
	sj->comp_host  =  best_cpu->name;
	sj->cores      =  best_cpu->cores;
      }
    
    return best_cpu;
}


std::vector<subJob*> PANDA_DISPATCHER::splitJobIntoSubjobs(Job& job, size_t& max_flops_per_subjob, size_t& max_storage_per_subjob) {
    // Start with creating subjobs
    std::vector<subJob*> subjobs;

    size_t total_read_size  = job.input_storage;
    size_t total_write_size = job.output_storage;
    size_t total_flops      = job.flops;

    // Calculate the total number of subjobs needed based on FLOPs and storage requirements
    size_t num_subjobs = std::max(
        static_cast<size_t>(1),
        std::max(
            (static_cast<size_t>(job.flops)) / max_flops_per_subjob,
            (total_read_size + total_write_size) / max_storage_per_subjob
        )+1
    );

    // Calculate FLOPs and storage per subjob
    size_t flops_per_subjob         = total_flops / num_subjobs;
    size_t leftover_flops           = total_flops % num_subjobs;
    size_t read_storage_per_subjob  = (total_read_size) / num_subjobs;
    size_t read_leftover_storage    = (total_read_size) % num_subjobs;
    size_t write_storage_per_subjob = (total_write_size) / num_subjobs;
    size_t write_leftover_storage   = (total_write_size) % num_subjobs;

      
    std::map<std::string, size_t> read_files  = job.input_files;
    std::map<std::string, size_t> write_files = job.output_files;

    for (size_t i = 0; i < num_subjobs; ++i) {
        subJob* subjob = new subJob;
        subjob->id = job.id + "-subJob-" + std::to_string(i);

        // Allocate FLOPs to the subjob
        subjob->flops =  flops_per_subjob + (i < leftover_flops ? 1 : 0);  // Distribute leftover FLOPs
        job.flops     -= subjob->flops;

        size_t read_current_storage = 0;
        size_t write_current_storage = 0;

	
        // Allocate storage to the subjob (equal distribution)
        size_t read_storage_limit = read_storage_per_subjob + (i < read_leftover_storage ? 1 : 0);
        size_t write_storage_limit = write_storage_per_subjob + (i < write_leftover_storage ? 1 : 0);

        // Allocate read files to the subjob
        for (auto it = read_files.begin(); it != read_files.end();) {
            if (read_current_storage + it->second <= read_storage_limit) {
                subjob->input_files[it->first] = it->second;
                read_current_storage += it->second;
                it = read_files.erase(it);
            } else {
                ++it;
            }
        }

        // Allocate write files to the subjob
        for (auto it = write_files.begin(); it != write_files.end();) {
            if (write_current_storage + it->second <= write_storage_limit) {
                subjob->output_files[it->first] = it->second;
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


void PANDA_DISPATCHER::allocateResourcesToSubjobs(std::vector<Site*>& sites, JobQueue& jobs, const std::map<std::string, double>& weights, size_t& max_flops_per_subjob, size_t& max_storage_per_subjob, std::set<Host*>& hosts_with_jobs) {

  bool  use_round_robin      = false;
  int   current_site_index   = 0;
  
  while (!jobs.empty())
    {
      Job  job       = jobs.top();
      auto subjobs   = splitJobIntoSubjobs(job, max_flops_per_subjob, max_storage_per_subjob);
      for (auto& subjob : subjobs)
	{
	  Host*  best_cpu    = nullptr;
	  for(int i = 0; i < sites.size(); ++i)
	    {
	      current_site_index = use_round_robin ? (current_site_index + 1) % sites.size() : current_site_index;
	      auto site          = sites[current_site_index];
	      use_round_robin    = !use_round_robin && (site->cpus_in_use >= site->cpus.size() / 2);
	      best_cpu           = findBestAvailableCPU(site->cpus, subjob, weights);
	      if(best_cpu) {site->cpus_in_use++; break;} 
	    }
	  
	  if(!best_cpu){throw std::runtime_error("Failed to find a suitable Resources for job.");}
	  this->printJobInfo(subjob);
	  hosts_with_jobs.insert(best_cpu);
	  }
      
      jobs.pop();
    }
  
}

void PANDA_DISPATCHER::printJobInfo(subJob* subjob)
{
      std::cout << "----------------------------------------------------------------------" << std::endl;
      std::cout << "\033[32m" << "Submitting .. "          <<  subjob->id     << std::endl;
      std::cout << "\033[37m" << "FLOPs to be executed: "  <<  subjob->flops  << std::endl;
      std::cout << "\033[33m" << "Files to be read: "      <<  std::endl;
      for(const auto& file: subjob->input_files){
      std::cout << "File: " << std::setw(40) << std::left << file.first
      << " Size: " << std::setw(10) << std::right << file.second
      << std::endl;}
      std::cout << "\033[36m" << "Files to be written: " <<  std::endl;
      for(const auto& file: subjob->output_files){
      std::cout << "File: " << std::setw(40) << std::left << file.first
      << " Size: " << std::setw(10) << std::right << file.second
      << std::endl;}
      std::cout << "\033[35m" << "Cores Used: "  << subjob->cores      <<  std::endl;
      std::cout << "\033[0m"  << "Disk Used :  " << subjob->disk       <<  std::endl;
      std::cout << "\033[34m" << "Host : "       << subjob->comp_host  << "\033[0m" << std::endl;

      std::cout << "----------------------------------------------------------------------" << std::endl;

}
