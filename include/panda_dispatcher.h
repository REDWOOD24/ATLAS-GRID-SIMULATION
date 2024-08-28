// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-27
// Description: Class to distribute Jobs on grid.
// Version: 1.0
// ==============================================


#ifndef PANDA_DISPATCHER_H
#define PANDA_DISPATCHER_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include <simgrid/s4u.hpp>
#include "job_manager.h"
#include "actions.h"
namespace sg4 = simgrid::s4u;

//A job is made of many subJobs. Information needed to a specify a subJob                                  
struct subJob {
  std::string                       id{};
  double                            flops{};
  std::map<std::string, size_t>     input_files{};
  std::map<std::string, size_t>     output_files{};
  std::string                       read_host{};
  std::string                       comp_host{};
  std::string                       write_host{};
  int                               cores{};
  std::string                       disk{};
};


//Need basic characterization of hosts and disks to find the optimal one for each job.
struct disk_params {
  std::string name{};
  size_t storage{};  // in MB
  double read_bw{};  // in GB/s
  double write_bw{}; // in GB/s
};

struct Host {
  std::string name{};
  double speed{};    // in GHz
  int cores{};
  bool is_taken{};
  std::vector<disk_params> disks{};
  std::string best_disk{};
};  // Disks by their ID


class PANDA_DISPATCHER
{


public:
  PANDA_DISPATCHER(sg4::Engine* _e);
 ~PANDA_DISPATCHER(){};


  static void execute_subjob(const subJob s);
  void create_actors(const std::vector<subJob>& subjobs);
  void dispatch_jobs(std::vector<Job>& jobs, sg4::NetZone* platform);
  void update_all_disks_content(std::vector<subJob>& subjobs);
  void update_disk_content(sg4::Disk* d, const std::string content);
  std::string get_disk_content(const std::map<std::string, size_t>& inputMap);
  void getHostsINFO(sg4::NetZone* platform, std::vector<Host>& cpus);

  //Functions needed to specify hosts for jobs
  double calculateWeightedScore(Host& cpu, subJob& sj, const std::map<std::string, double>& weights, std::string& best_disk_name);
  double getTotalSize(const std::map<std::string, size_t>& files);
  Host* findBestAvailableCPU(std::vector<Host>& cpus, subJob& sj, const std::map<std::string, double>& weights);
  std::vector<subJob> splitJobIntoSubjobs(Job& job, size_t max_flops_per_subjob, size_t max_storage_per_subjob);
  void allocateResourcesToSubjobs(std::vector<Host>& cpus, std::vector<Job>& jobs, const std::map<std::string, double>& weights, size_t max_flops_per_subjob,  size_t max_storage_per_subjob, std::vector<subJob>& all_subjobs);
  void printJobInfo(subJob& subjob);  
  
private:
  sg4::Engine* e;

  const std::map<std::string, double> weights = {
        {"speed", 1.0},
        {"cores", 1.0},
	{"disk", 1.0},
        {"disk_storage", 1.0},
        {"disk_read_bw", 1.0},
        {"disk_write_bw", 1.0}
    };
  size_t max_flops_per_subjob = 1e6;
  size_t max_storage_per_subjob = 1e8;
};

#endif
