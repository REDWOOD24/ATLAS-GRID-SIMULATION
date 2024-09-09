// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Class to create actions that actors (jobs) will execute.
// Version: 1.0
// ==============================================


#ifndef ACTIONS_H
#define ACTIONS_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include <simgrid/s4u.hpp>
#include "ATLAS_FileSystem.h"
#include "simgrid/plugins/energy.h"

namespace sg4 = simgrid::s4u;

class Actions
{


public:
  Actions(){};
 ~Actions(){};

  int          exec_task_multi_thread(int flops, int cores, std::string exec_host);
  void         exec_task_multi_thread_async(double flops, int cores, std::string exec_host);

  sg_size_t    read(std::string filename, const_sg_host_t exec_host);
  sg_size_t    write(std::string filename, size_t file_size, const_sg_host_t exec_host);
  sg_size_t    size(std::string filename, const_sg_host_t exec_host);
  void         remove(std::string filename, const_sg_host_t exec_host);

  
};

#endif

