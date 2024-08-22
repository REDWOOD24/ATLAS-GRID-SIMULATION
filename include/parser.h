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

using namespace nlohmann;

class Parser
{


public:
  Parser(const std::string& _inputFile);
  ~Parser(){};

 std::map<std::string, std::pair<double, double>> getSiteConnInfo();
 std::map<std::string, std::map<std::string,int>> getSiteNameCPUInfo();
 std::set<std::string> getSiteNames();

 private:
    std::string inputFile;
};

#endif

