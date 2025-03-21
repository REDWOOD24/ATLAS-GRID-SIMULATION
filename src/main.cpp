// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Entry point for the application.
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
   std::string usage = std::string("usage: ") + argv[0] + " -c config.json";
   if(argc != 3){std::cout << usage << std::endl; exit(-1);}
   if(std::string(argv[1]) != std::string("-c")){std::cout << usage << std::endl; exit(-1);}

   //Parse Configuration File
   std::string configFile  = std::string(argv[2]);
   std::cout << "Reading in Configuration From: " << configFile << std::endl;
   std::ifstream in(configFile);
   auto j=json::parse(in);

   const std::string         gridName          = j["Grid name"];
   const std::string         siteInfoFile      = j["Sites Information"];
   const std::string         siteConnInfoFile  = j["Sites Connection Information"];
   const std::string         dispatcherPath    = j["Dispatcher Plugin"];
   const std::string         outputFile        = j["Output DB"];
   const int                 num_of_jobs       = j["Num of Jobs"]; 
   const std::string         jobFile           = j["Historical Job"]; 

   std::unique_ptr<Parser>   parser            = std::make_unique<Parser>(siteConnInfoFile,siteInfoFile,jobFile);
   auto                      siteNameCPUInfo   = parser->getSiteNameCPUInfo();
   auto                      siteConnInfo      = parser->getSiteConnInfo();
   auto                      siteNameGLOPS     = parser->getSiteNameGFLOPS();

   //Initialize Simulation Engine
   sg4::Engine e(&argc, argv);

   //Create the SimGrid platform
   std::unique_ptr<Platform> pf = std::make_unique<Platform>();
   auto* platform = pf->create_platform(gridName);

   //Initialize the SimGrid Plugins used
   pf->initialize_simgrid_plugins();
   
   //Create the Sites
   auto sites = pf->create_sites(platform, siteNameCPUInfo, siteNameGLOPS);

   //Setup Connections between sites
   pf->initialize_site_connections(platform,siteConnInfo,sites);

   //Initialize the Jobs Server // this is more like PANDA Server
   pf->initialize_job_server(platform,siteNameCPUInfo,sites);
   
   //Create Job Executor
   std::unique_ptr<JOB_EXECUTOR> executor = std::make_unique<JOB_EXECUTOR>();
   const std::vector<simgrid::s4u::NetZone*>& children = platform->get_children();
   std::cout << "Network Model" << platform->get_network_model() << std::endl;
   if (!children.empty()) {
       std::cout << "Children:" << std::endl;
       for (const auto child : children) {
         std::cout << "name:" << child->get_name() <<std::endl;
       }
   } 
  
   executor->set_output(outputFile);
   executor->set_dispatcher(dispatcherPath,platform);
   executor->start_receivers();

   //Create Jobs
   std::unique_ptr<JOB_MANAGER> jm = std::make_unique<JOB_MANAGER>();
   jm->set_parser(std::move(parser));
   JobQueue jobs = jm->get_jobs(num_of_jobs);
   std::cout<< "Jobs to be executed" << jobs.size() <<std::endl;
   //Execute Jobs
   executor->start_job_execution(jobs);
   executor->saveJobs(jobs);

   //Print simulator name and current version
   std::cout << "\nSimATLAS version: " << MAJOR_VERSION << "." << MINOR_VERSION << "." << BUILD_NUMBER << std::endl;

   return 0;
}
