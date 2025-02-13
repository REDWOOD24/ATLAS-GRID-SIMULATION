// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Class to create actions that actors (jobs) will execute.
// ==============================================


#ifndef ACTIONS_H
#define ACTIONS_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include <simgrid/s4u.hpp>
#include "simgrid/plugins/energy.h"
#include "fsmod.hpp"
namespace sg4 = simgrid::s4u;

class Actions
{


public:
  Actions()= default;
 ~Actions()= default;

  static sg4::ExecPtr  exec_task_multi_thread_async(double flops, int cores);
  static sg4::IoPtr    read_file_async(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, const std::string& filename);
  static sg4::IoPtr    write_file_async(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, const std::string& filepath, size_t file_size);


  
};

#endif

