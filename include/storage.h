// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Class to access information about storage at various sites.
// Version: 1.0
// ==============================================

#ifndef STORAGE_H
#define STORAGE_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include <simgrid/s4u.hpp>
#include "simgrid/plugins/file_system.h"

namespace sg4 = simgrid::s4u;

class Storage
{


public:
  Storage(){};
  ~Storage(){};

  static void get_disk_info();
 
    
  
};

#endif

