// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Class to hold all sorts of actions one want to run during the simulation.
// Version: 1.0
// ==============================================


#ifndef ACTORS_H
#define ACTORS_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include <simgrid/s4u.hpp>
#include <storage.h>
#include <data.h>

namespace sg4 = simgrid::s4u;

class Actors
{


public:
  Actors(sg4::Engine* _e);
  ~Actors(){};

  void get_disk_info(const std::string host);
  void send_data(const std::string sender, const std::string receiver, int num_of_data_messages, size_t size_of_data_message);
 
private:
  sg4::Engine* e;
 
    
  
};

#endif

