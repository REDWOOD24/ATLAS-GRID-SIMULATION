// ==============================================                                                                                
// Author: Raees Khan                                                                                                            
// Email: rak177@pitt.edu                                                                                                        
// Created Date: 2024-08-15                                                                                                      
// Description: Class to parse json containing information about ATLAS grid sites.                                               
// Version: 1.0                                                                                                                  
// ==============================================   


#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include "nlohmann/json.hpp"


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
  Parser(const std::string& _inputFile);
  ~Parser(){};

 std::map<std::string, std::pair<double, double>> getSiteConnInfo();
 std::map<std::string, std::map<std::string,CPUInfo>> getSiteNameCPUInfo();
 std::set<std::string> getSiteNames();
 DiskInfo getDiskInfo(const std::string site_cpu_name);
 private:
    std::string inputFile;
};

#endif

