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
#include "panda_dispatcher.h"
#include "version.h"


int main(int argc, char** argv)
{
   //Usage 
   std::string usage = std::string("usage: ") + argv[0] + " ../data/site_conn_info.json ../data/site_info.json ../data/jobs.csv -1 ../job_logs/log.h5";
   if(argc != 6){std::cout << usage << std::endl; exit(-1);}

   //Parse json with information about ATLAS sites
   const std::string         siteConnInfoFile  = std::string(argv[1]);
   const std::string         siteInfoFile      = std::string(argv[2]);
   const std::string         jobInfoFile       = std::string(argv[3]);
   const int                 num_of_jobs       = std::stoi(argv[4]);
   const std::string         outputFile        = std::string(argv[5]);
   
   
   std::unique_ptr<Parser>   parser            = std::make_unique<Parser>(siteConnInfoFile,siteInfoFile,jobInfoFile);
   auto                      siteNameCPUInfo   = parser->getSiteNameCPUInfo();
   auto                      siteConnInfo      = parser->getSiteConnInfo();
   auto                      siteNameGLOPS     = parser->getSiteNameGFLOPS();
   

   //Initialize Simulation Engine
   sg4::Engine e(&argc, argv);

   //Create the SimGrid platform
   std::unique_ptr<Platform> pf = std::make_unique<Platform>();
   auto* platform = pf->create_platform("ATLAS-GRID");

   //Initialize the Plugins used
   pf->initialize_plugins();
   
   //Create the Sites
   auto sites = pf->create_sites(platform, siteNameCPUInfo, siteNameGLOPS);

   //Setup Connections between sites
   pf->initialize_site_connections(platform,siteConnInfo,sites);
   
   //Create Jobs
   std::unique_ptr<JOB_MANAGER> jm = std::make_unique<JOB_MANAGER>();
   jm->set_parser(std::move(parser));
   auto jobs = jm->get_jobs(num_of_jobs);
   
   //Pass to Dispatcher
   std::unique_ptr<PANDA_DISPATCHER>  dispatcher = std::make_unique<PANDA_DISPATCHER>(&e,outputFile);
   // dispatcher->dispatch_tasks(tasks,platform);
   dispatcher->dispatch_jobs(jobs,platform);
   
   //Run Simulation
   platform->seal();
   
   std::cout << "Simulation in Progress ..." << std::endl;
   auto start = std::chrono::system_clock::now();
   e.run();
   auto finish  = std::chrono::system_clock::now();
   std::chrono::duration<double> time = finish - start;
   std::cout  << "Simulation Finished Succesfully in " << time.count() << " seconds! Information written out at: " << outputFile << std::endl;
   std::cout << "No of Jobs sucessfully executed ..." << jobs.size() <<std::endl;

   //Print simulator name and current version
   std::cout << "\nSimATLAS version: " << MAJOR_VERSION << "." << MINOR_VERSION << "." << BUILD_NUMBER << std::endl;

   return 0;
}
