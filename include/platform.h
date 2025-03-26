// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Class to Instantiate SimGrid Platform which simulates the ATLAS GRID.
// ==============================================


#ifndef PLATFORM_H
#define PLATFORM_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include <simgrid/s4u.hpp>
#include "simgrid/plugins/energy.h"
#include "parser.h"
#include "fsmod.hpp"
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
static sg4::NetZone* create_site(sg4::NetZone* platform, const std::string& site_name, std::unordered_map<std::string, CPUInfo>& cpuInfo,  long siteGFLOPS);

/**
 *
 * This function creates a number of sites.
 */
  std::unordered_map<std::string, sg4::NetZone*>  create_sites(sg4::NetZone* platform, std::unordered_map<std::string, std::unordered_map<std::string, CPUInfo>>& siteNameCPUInfo, std::unordered_map<std::string,int>& siteNameGFLOPS);
/**
 *
 * This function creates only specified sites.
 */
  std::unordered_map<std::string, sg4::NetZone*>  create_sites(sg4::NetZone* platform, const std::list<std::string>& filteredSiteList, std::unordered_map<std::string, std::unordered_map<std::string, CPUInfo>>& siteNameCPUInfo, std::unordered_map<std::string,int>& siteNameGFLOPS);
/**
 *
 * This function initializes the connection between various sites in a platform.
 */
void initialize_site_connections(sg4::NetZone* platform, std::unordered_map<std::string, std::pair<double, double>>& siteConnInfo, std::unordered_map<std::string, sg4::NetZone*>& sites);


/**                                                                                                        
 *                                                                                                           
 * This function initializes the various simgrid plugins used in the simulation.
 */
void initialize_simgrid_plugins();

  
/**                                                                                                                                                                   
 *                                                                                                                                                                    
 * This function creates the server that will send out jobs to sites.
 */
void initialize_job_server(sg4::NetZone* platform,  std::unordered_map<std::string, std::unordered_map<std::string, CPUInfo>>& siteNameCPUInfo, std::unordered_map<std::string, sg4::NetZone*>& sites);
  
};

#endif

