// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Class to send simple data packets between sites and hosts.
// Version: 1.0
// ==============================================


#ifndef DATA_H
#define DATA_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include <simgrid/s4u.hpp>
namespace sg4 = simgrid::s4u;

class Data
{


public:
  Data(){};
  ~Data(){};

  static void sender(int data_count, size_t payload_size);

  static void receiver();

 
    
  
};

#endif

