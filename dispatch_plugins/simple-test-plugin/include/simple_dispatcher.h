// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-27
// Description: Class to distribute Jobs on grid.
// ==============================================


#ifndef SIMPLE_DISPATCHER_H
#define SIMPLE_DISPATCHER_H

#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <simgrid/s4u.hpp>
#include "H5Cpp.h"
#include "job.h"
#include "fsmod.hpp"
namespace sg4 = simgrid::s4u;


//Need basic characterization of sites, hosts and disks to find the optimal one for each job.
struct disk_params {
  std::string                name{};
  std::string                mount{};
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
  std::vector<Job*>          jobs{};
  bool operator<(const Host& other) const {return flops_available <= other.flops_available;}
};

struct Site {
  std::string                name{};
  int                        priority{};
  std::vector<Host*>         cpus{};
  int                        cpus_in_use{};
  bool operator<(const Site& other) const {return priority <= other.priority;}
};



class SIMPLE_DISPATCHER
{

public:
  SIMPLE_DISPATCHER(){};
 ~SIMPLE_DISPATCHER(){};

  //Resource Management
  void setPlatform(sg4::NetZone* platform);

  //Functions needed to specify hosts for jobs
  double calculateWeightedScore(Host* cpu, Job* j, std::string& best_disk_name);
  double getTotalSize(const std::unordered_map<std::string, size_t>& files);
  Host*  findBestAvailableCPU(std::vector<Host*>& cpus, Job* j);
  Job*   assignJobToResource(Job* job);
  void   printJobInfo(Job* job);


private:
  bool  use_round_robin{false};
  std::vector<Site*>    _sites{};
  const std::unordered_map<std::string, double> weights =
      {
        {"speed", 1.0},
        {"cores", 1.0},
	      {"disk", 1.0},
        {"disk_storage", 1.0},
        {"disk_read_bw", 1.0},
        {"disk_write_bw", 1.0}
      };
};

#endif
