#include "parser.h"
#include <iostream>

Parser::Parser(const std::string& _inputFile){inputFile = _inputFile;}


//I don't have information about CPUs and cores at various sites, so assign randomly.                                                         
int Parser::genRandNum(int lower, int upper) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(lower, upper);
    return distrib(gen);
}

double Parser::GaussianDistribution(double mean, double stddev) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(mean, stddev);
    return d(gen);
}


DiskInfo Parser::getDiskInfo(const std::string disk_name)
{
  DiskInfo disk;
  disk.name        = disk_name;
  disk.read_bw     = this->genRandNum(150,250);
  disk.write_bw    = this->genRandNum(40,120);
  disk.size        = std::to_string(this->genRandNum(40,120))+"GiB";
  if(disk_name.find("SCRATCH") != std::string::npos) disk.mount = "/scratch";
  else if(disk_name.find("LOCAL") != std::string::npos) disk.mount = "/local";
  else {std::cout << "Mount point not specified ... Exiting now ..." << std::endl; exit(-1);}
  return disk;
}

std::set<std::string> Parser::getSiteNames()
{
  std::ifstream in(inputFile);
  auto j=json::parse(in);

  //Get all Site Names
  std::set<std::string> site_names;
  for (auto it = j.begin(); it != j.end(); ++it) {
    std::string site = it.key().substr(0, it.key().find(':'));
    site_names.insert(site);}
    
    return site_names;
}

std::map<std::string, std::map<std::string, CPUInfo>> Parser::getSiteNameCPUInfo()
{
  std::set<std::string> site_names = this->getSiteNames();
  std::map<std::string, std::map<std::string, CPUInfo>> siteNameCPUInfo;
  
  for(const auto& site: site_names){
    for(int cpu_num = 0; cpu_num < this->genRandNum(50,150); cpu_num++){
      CPUInfo cpu;
      cpu.cores    = this->genRandNum(20,100);
      cpu.speed    = this->genRandNum(10,20)*1e8;
      cpu.BW_CPU   = this->genRandNum(10,20)*1e11;
      cpu.LAT_CPU  = 0;
      cpu.ram      = std::to_string(this->genRandNum(8,16))+"GiB";
      // cpu.disk_info.push_back(this->getDiskInfo("CALIBDISK"));
      // cpu.disk_info.push_back(this->getDiskInfo("DATADISK"));
      cpu.disk_info.push_back(this->getDiskInfo("SCRATCHDISK"));
      cpu.disk_info.push_back(this->getDiskInfo("LOCALGROUPDISK"));
      siteNameCPUInfo[site][site+"_cpu-"+std::to_string(cpu_num)] = cpu;
    }
  }
  return siteNameCPUInfo;
}



std::map<std::string, std::pair<double, double>> Parser::getSiteConnInfo()
 {
  std::ifstream in(inputFile);
  auto j=json::parse(in);

  //Defining Latency in ms based on 'closeness' value in json file
  std::map<int, int> closeness_latency_map = {
        {0, 0}, {1, 10}, {2, 20}, {3, 30}, {4, 40},
        {5, 50}, {6, 60}, {7, 70}, {8, 80}, {9, 90},
        {10, 100}, {11, 110}, {12, 120}};

  //Get all Site Names
  std::set<std::string> site_names = this->getSiteNames();

  //Store Site Info
  std::map<std::string, std::pair<double, double>> siteConnInfo;
  for(auto it1 = site_names.begin(); it1 != site_names.end(); ++it1) {
    for(auto it2 = std::next(it1); it2 != site_names.end(); ++it2) {
      std::string site1 = *it1;
      std::string site2 = *it2;
      
      std::string connection = site1 + ":" + site2;
      
      //Check Information for connection exists
      bool info_exists = j[connection].contains("mbps") && j[connection]["closeness"].contains("latest")  && j[connection]["mbps"]["dashb"].contains("1w");

      //Store if exists
      if (info_exists){
        double latency             = closeness_latency_map[j[connection]["closeness"]["latest"]];
        double bandwidth           = j[connection]["mbps"]["dashb"]["1w"]; 
        siteConnInfo[connection]   = std::make_pair(latency,bandwidth);}}}
 
 return siteConnInfo;
 }

