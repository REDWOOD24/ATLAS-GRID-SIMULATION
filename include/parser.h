// ==============================================                                                                               
// Author: Raees Khan                                                                                                            
// Email: rak177@pitt.edu                                                                                                        
// Created Date: 2024-08-15                                                                                                      
// Description: Class to parse json containing information about ATLAS grid sites.                                               
// ==============================================   


#ifndef PARSER_H
#define PARSER_H

#include <set>
#include <fstream>
#include "nlohmann/json.hpp"
#include <unordered_map>
#include <random>
#include <job.h>
#include <queue>

//Information needed to a specify a Disk                                                      
struct DiskInfo {
  std::string   name{};
  double        read_bw{};
  double        write_bw{};
  std::string   size{};
  std::string   mount{};  
};

//Information needed to a specify a host (CPU)
struct CPUInfo {
  int                    cores{};
  double                 speed{};
  double                 BW_CPU{};
  double                 LAT_CPU{};
  std::string            ram{};
  std::vector<DiskInfo>  disk_info{};
};





using namespace nlohmann;

class Parser
{


public:
  Parser(const std::string& _siteConnInfoFile, const std::string& _siteInfoFile);
  Parser(){}
 ~Parser(){};

 int                                                                       genRandNum(int lower, int upper);
 double                                                                    GaussianDistribution(double mean, double stddev);
 void                                                                      setSiteNames();
 void                                                                      setSiteCPUCount();

 std::vector<DiskInfo>                                                     getDisksInfo(const std::string site_name, int num_of_cpus);
 std::unordered_map<std::string, std::pair<double, double>>                getSiteConnInfo();
 std::unordered_map<std::string, std::unordered_map<std::string,CPUInfo>>  getSiteNameCPUInfo();
 void                                                                      setSiteGFLOPS();
 std::priority_queue<Job*>                                                  getJobs(long max_jobs);
 std::unordered_map<std::string,int>                                       getSiteNameGFLOPS();

 
 
private:
  std::string                          siteConnInfoFile;
  std::string                          siteInfoFile;
  std::set<std::string>                site_names;
  std::unordered_map<std::string,int>  siteCPUCount;
  std::unordered_map<std::string,int>  siteNameGFLOPS;
  std::string                          jobFile;
};

#endif

