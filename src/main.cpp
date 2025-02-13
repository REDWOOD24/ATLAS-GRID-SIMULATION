// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Entry point for the application.
// Version: 1.0
// ==============================================




#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <string>
#include <memory>
#include <math.h>
#include <chrono>
#include <simgrid/s4u.hpp>
#include "parser.h"
#include "platform.h"
#include "job_manager.h"
#include "version.h"
#include "job_executor.h"

int main(int argc, char** argv)
{
   //Usage
   std::string usage = std::string("usage: ") + argv[0] + " ../data/site_conn_info.json ../data/site_info.json dispatcher_plugin_path ../job_logs/log.h5";
   if(argc != 5){std::cout << usage << std::endl; exit(-1);}

   //Parse json with information about ATLAS sites
   const std::string         siteConnInfoFile  = std::string(argv[1]);
   const std::string         siteInfoFile      = std::string(argv[2]);
   const std::string         dispatcherPath    = std::string(argv[3]);
   const std::string         outputFile        = std::string(argv[4]);
   std::unique_ptr<Parser>   parser            = std::make_unique<Parser>(siteConnInfoFile,siteInfoFile);
   auto                      siteNameCPUInfo   = parser->getSiteNameCPUInfo();
   auto                      siteConnInfo      = parser->getSiteConnInfo();

   //Initialize Simulation Engine
   sg4::Engine e(&argc, argv);

   //Create the SimGrid platform
   std::unique_ptr<Platform> pf = std::make_unique<Platform>();
   auto* platform = pf->create_platform("ATLAS-GRID");

   //Initialize the Plugins used
   pf->initialize_plugins();
   
   //Create the Sites
   auto sites = pf->create_sites(platform, siteNameCPUInfo);

   //Setup Connections between sites
   pf->initialize_site_connections(platform,siteConnInfo,sites);

   //Create Job Executor
   std::unique_ptr<JOB_EXECUTOR> executor = std::make_unique<JOB_EXECUTOR>(&e,outputFile);
   executor->set_dispatcher(dispatcherPath,platform);
   executor->start_receivers();
   e.run();

   //Create Jobs
   std::unique_ptr<JOB_MANAGER> jm = std::make_unique<JOB_MANAGER>();
   auto jobs = jm->create_jobs(20);

   //Execute Jobs
   executor->execute_jobs(jobs);
   executor->kill_simulation();
   executor->print_output();

   //Print simulator name and current version
   std::cout << "\nSimATLAS version: " << MAJOR_VERSION << "." << MINOR_VERSION << "." << BUILD_NUMBER << std::endl;

   return 0;
}
