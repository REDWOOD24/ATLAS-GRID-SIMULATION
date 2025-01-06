// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-27
// Description: Class to distribute Jobs on grid.
// ==============================================


#ifndef PANDA_DISPATCHER_H
#define PANDA_DISPATCHER_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include <simgrid/s4u.hpp>
#include "H5Cpp.h"
#include "job_manager.h"
#include "actions.h"
namespace sg4 = simgrid::s4u;


//Need basic characterization of sites, hosts and disks to find the optimal one for each job.
struct disk_params {
  std::string                name{};
  size_t                     storage{};  
  double                     read_bw{}; 
  double                     write_bw{};
};

struct Host {
  std::string                name{};
  double                     speed{};
  int                        cores{};
  size_t                     flops_available{};
  std::vector<disk_params*>  disks{};
  std::vector<JobInfo*>          jobs{};
  bool operator<(const Host& other) const {return flops_available <= other.flops_available;}
};

struct Site {
  std::string                name{};
  int                        priority{};
  std::vector<Host*>         cpus{};
  int                        cpus_in_use{};
  int                        gflops{};
  bool operator<(const Site& other) const {return priority <= other.priority;}
};

//Output to Save
struct output {
  char*                      id{};
  int                        flops_exec{};
  int                        files_read_size{};
  int                        files_written_size{};
  float                      read_IO_time{};
  float                      write_IO_time{};
  float                      flops_exec_time{};
};


class PANDA_DISPATCHER
{


public:
  PANDA_DISPATCHER(sg4::Engine* _e, const std::string& _outputFile);
 ~PANDA_DISPATCHER(){};
  // void dispatch_tasks(TaskQueue& tasks, sg4::NetZone* platform);
  void dispatch_jobs(JobQueue& jobs, sg4::NetZone* platform);
  
protected:
  static void execute_job(const std::vector<JobInfo*>& jobs);

  void create_actors(const std::set<Host*>& hosts_with_jobs);
  void update_all_disks_content(const std::set<Host*>& hosts_with_jobs);
  void update_disk_content(sg4::Disk* d, const std::string& content);
  std::unordered_map<std::string, size_t> merge_content(const std::vector<std::unordered_map<std::string, size_t>>& maps);
  std::string get_disk_content(const std::unordered_map<std::string, size_t>& inputMap);
  void getHostsINFO(sg4::NetZone* platform, std::vector<Site*>& sites);
  void cleanup(std::vector<Site*>& sites);
  void h5init();
  //Functions needed to specify hosts for jobs
  double calculateWeightedScore(Host* cpu, JobInfo* j, const std::unordered_map<std::string, double>& weights, std::string& best_disk_name);
  double getTotalSize(const std::unordered_map<std::string, size_t>& files);
  Host* findBestAvailableCPU(std::vector<Host*>& cpus, JobInfo* j, const std::unordered_map<std::string, double>& weights);
  void allocateResourcesToJobs(std::vector<Site*>& sites, JobQueue& jobs, size_t& max_flops_per_job,  size_t& max_storage_per_job, std::set<Host*>&   hosts_with_jobs);
  void printJobInfo(JobInfo* job); 
  Site*  findSiteByName(std::vector<Site*>& sites, const std::string& site_name);
  
private:
  sg4::Engine*            e;
  std::unique_ptr<Parser> p = std::make_unique<Parser>();
  std::string             outputFile;
  static H5::H5File       h5_file;
  static H5::CompType     datatype;



  const std::unordered_map<std::string, double> weights = {
        {"speed", 1.0},
        {"cores", 1.0},
	{"disk", 1.0},
        {"disk_storage", 1.0},
        {"disk_read_bw", 1.0},
        {"disk_write_bw", 1.0}
    };
  size_t max_flops_per_job   = 1e6;
  size_t max_storage_per_job = 1e8;
};

#endif
