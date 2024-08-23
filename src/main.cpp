// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Entry point for the application.
// Version: 1.0
// ==============================================




#include <iostream>
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
#include "actors.h"


int main(int argc, char** argv)
{
   //Usage 
   std::string usage = std::string("usage: ") + argv[0] + " -J data.json";
   if(argc < 3 || std::string(argv[1])!=std::string("-J")){std::cout << usage << std::endl; exit(-1);}

   //Parse json with information about ATLAS sites
   const std::string         inputFile        = std::string(argv[2]);
   std::unique_ptr<Parser>   parser           = std::make_unique<Parser>(inputFile);
   auto                      siteNameCPUInfo  = parser->getSiteNameCPUInfo();
   auto                      siteConnInfo     = parser->getSiteConnInfo();

   //Initialize Simulation Engine
   sg4::Engine e(&argc, argv);
   
   //Create the SimGrid platform
   std::unique_ptr<Platform> pf = std::make_unique<Platform>();
   auto* platform = pf->create_platform("ATLAS-GRID");
   
   //Create the Sites
   auto sites = pf->create_sites(platform, siteNameCPUInfo);

   //Setup Connections between sites
   pf->initialize_site_connections(platform,siteConnInfo,sites);
   platform->seal();
   
   //Setup Actor
   std::unique_ptr<Actors> actors = std::make_unique<Actors>(&e);
   
   //Submit Jobs
   //for(const auto& site: site_names){actors->create_storage(site+"_cpu-0","Disk1");}
   //    actors->send_data("BEIJING-LCG2_cpu-6","ifae_cpu-0",5,100);
    actors->send_data("ifae_cpu-2","ifae_cpu-0",5,100);

    actors->get_disk_info("BEIJING-LCG2_cpu-3");
   //Run Simulation
   e.run();
   

   return 0;
}
