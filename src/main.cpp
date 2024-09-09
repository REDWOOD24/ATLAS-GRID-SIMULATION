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
#include <simgrid/s4u.hpp>
#include "parser.h"
#include "platform.h"
#include "job_manager.h"
#include "panda_dispatcher.h"


int main(int argc, char** argv)
{
   //Usage 
   std::string usage = std::string("usage: ") + argv[0] + " ../data/site_conn_info.json ../data/site_info.json ../job_logs/log.h5";
   if(argc != 4){std::cout << usage << std::endl; exit(-1);}

   //Parse json with information about ATLAS sites
   const std::string         siteConnInfoFile  = std::string(argv[1]);
   const std::string         siteInfoFile      = std::string(argv[2]);
   const std::string         outputFile        = std::string(argv[3]);
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
   
   //Create Jobs
   std::unique_ptr<JOB_MANAGER> jm = std::make_unique<JOB_MANAGER>();
   auto jobs = jm->create_jobs(20);

   //Pass to Dispatcher
   std::unique_ptr<PANDA_DISPATCHER> dispatcher = std::make_unique<PANDA_DISPATCHER>(&e,outputFile);
   dispatcher->dispatch_jobs(jobs,platform);
   
   //Run Simulation
   platform->seal();
   std::cout << "Simulation in Progress ...";
   e.run();
   std::cout << std::endl << "Simulation Finished Succesfully! Information written out at: " << outputFile << std::endl;
   return 0;
}
