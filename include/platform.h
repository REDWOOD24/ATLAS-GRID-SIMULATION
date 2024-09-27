// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Class to Instantiate SimGrid Platform which simulates the ATLAS GRID.
// Version: 1.0
// ==============================================


#ifndef PLATFORM_H
#define PLATFORM_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include <simgrid/s4u.hpp>
#include "ATLAS_FileSystem.h"
#include "simgrid/plugins/energy.h"
#include "parser.h"
namespace sg4 = simgrid::s4u;

class Platform
{
public:
  Platform(){};
  ~Platform(){};

/**
 *
 * This function makes a platform.
 */
static sg4::NetZone* create_platform(const std::string& platform_name);

 /**
 *
 * This function creates one site made of nb_cpu CPUs.
 */
static sg4::NetZone* create_site(sg4::NetZone* platform, const std::string& site_name, std::unordered_map<std::string, CPUInfo>& cpuInfo);

/**
 *
 * This function creates a number of sites.
 */
  std::unordered_map<std::string, sg4::NetZone*>  create_sites(sg4::NetZone* platform, std::unordered_map<std::string, std::unordered_map<std::string, CPUInfo>>& siteNameCPUInfo);;

/**
 *
 * This function initializes the connection between various sites in a platform.
 */
void initialize_site_connections(sg4::NetZone* platform, std::unordered_map<std::string, std::pair<double, double>>& siteConnInfo, std::unordered_map<std::string, sg4::NetZone*>& sites);


/**                                                                                                        
 *                                                                                                           
 * This function initializes the various plugins used in the simulation.                             
 */
void initialize_plugins();    
  
};

#endif

