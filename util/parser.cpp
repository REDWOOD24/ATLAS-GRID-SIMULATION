#include "parser.h"
#include <iostream>

Parser::Parser(const std::string& _siteConnInfoFile, const std::string& _siteInfoFile)
{
  siteConnInfoFile = _siteConnInfoFile;
  siteInfoFile = _siteInfoFile;
  this->setSiteCPUCount(); //Reason this is set before site-names is because is site has 0 cpus (no info) I don't include it.
  this->setSiteNames();
}


//I don't have information about CPUs and cores at various sites, so assign randomly.
int Parser::genRandNum(int lower, int upper)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(lower, upper);
    return distrib(gen);
}

double Parser::GaussianDistribution(double mean, double stddev)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(mean, stddev);
    return d(gen);
}

void Parser::setSiteNames()
{
  std::ifstream in(siteConnInfoFile);
  auto j=json::parse(in);

  //Get all Site Names
  for (auto it = j.begin(); it != j.end(); ++it) {
    std::string site = it.key().substr(0, it.key().find(':'));
    if(siteCPUCount[site] >0) site_names.insert(site);}
}

void Parser::setSiteCPUCount()
{
  std::ifstream in(siteInfoFile);
  auto j=json::parse(in);
  for (auto it = j.begin(); it != j.end(); ++it) {
    //500 flops per core and 32 cores per host (assumption)
    int gflops =  (j[it.key()]["GFLOPS"]).get<int>(); 
    int cpus   =  std::round(gflops/(32*500.0));
    siteCPUCount[it.key()] =  cpus;
  }
}

std::vector<DiskInfo> Parser::getDisksInfo(const std::string site_name, int num_of_cpus)
{
  std::ifstream in(siteInfoFile);
  auto j=json::parse(in);
  std::vector<DiskInfo> disks;
  auto info = j[site_name]["RSE"];
  
 for (auto it = info.begin(); it != info.end(); ++it)
    {

      if(j[site_name]["RSE"][it.key()] == 0) continue;
      if(it.key().find("DISK") == std::string::npos) continue;

      DiskInfo           disk;
      disk.name        = it.key();
      disk.read_bw     = this->genRandNum(150,250)*1e8; //Assumption
      disk.write_bw    = this->genRandNum(40,120)*1e8;  //Assumption
      disk.size        = std::to_string(std::round(1000*std::stod(j[site_name]["RSE"][it.key()].get<std::string>())/num_of_cpus*1.0)) +"GiB";

      if      (it.key().find("SCRATCH") != std::string::npos)  disk.mount = "/scratch";
      else if (it.key().find("LOCAL")   != std::string::npos)  disk.mount = "/local";
      else if (it.key().find("DATA")    != std::string::npos)  disk.mount = "/data";
      else if (it.key().find("CALIB")   != std::string::npos)  disk.mount = "/calib";
      else                                                     disk.mount = "/other";

      disks.push_back(disk);
    }
  
  return disks;
}


std::map<std::string, std::map<std::string, CPUInfo>> Parser::getSiteNameCPUInfo()
{

  std::map<std::string, std::map<std::string, CPUInfo>> siteNameCPUInfo;
  
  for(const auto& site: site_names){

    int                   num_of_cpus = siteCPUCount[site] + this->genRandNum(-3,3); //Little random offset added.
    std::vector<DiskInfo> disks       = this->getDisksInfo(site,num_of_cpus);

    //Info Printout
    std::cout << "----------------------------------------------------" << std::endl;
    std::cout << "Adding Site .... " << site << std::endl;
    std::cout << "CPUS          : " << num_of_cpus << std::endl;
    std::cout << "DISK-INFO-CPU : " <<  std::endl;
    for (const auto& d : disks) {
    std::cout << "NAME          : " << std::setw(30) << std::left << d.name 
              << " : " << std::setw(15) << std::right << d.size << std::endl;}
    std::cout << "----------------------------------------------------" << std::endl;
    
    for(int cpu_num = 0; cpu_num < num_of_cpus; cpu_num++){
      CPUInfo cpu;
      cpu.cores      = 32;                  //Assumption of 32 cores in calculating cpus. 
      cpu.speed      = this->genRandNum(10,20)*1e8;                  // Assumption
      cpu.BW_CPU     = this->genRandNum(10,20)*1e11;                 // Assumption 
      cpu.LAT_CPU    = 0;                                            // Assumption 
      cpu.ram        = std::to_string(this->genRandNum(8,16))+"GiB"; // Assumption
      cpu.disk_info  = disks;
      siteNameCPUInfo[site][site+"_cpu-"+std::to_string(cpu_num)] = cpu; 
    }
  };
  return siteNameCPUInfo;
}



std::map<std::string, std::pair<double, double>> Parser::getSiteConnInfo()
 {
  std::ifstream in(siteConnInfoFile);
  auto j=json::parse(in);

  //Defining Latency in ms based on 'closeness' value in json file
  std::map<int, int> closeness_latency_map = {
        {0, 0}, {1, 10}, {2, 20}, {3, 30}, {4, 40},
        {5, 50}, {6, 60}, {7, 70}, {8, 80}, {9, 90},
        {10, 100}, {11, 110}, {12, 120}};

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

