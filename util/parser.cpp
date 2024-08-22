#include "parser.h"
#include <random>

//I don't have information about CPUs and cores at various sites, so assign randomly.
int genRandNum(int lower, int upper) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(lower, upper);
    return distrib(gen);
}

Parser::Parser(const std::string& _inputFile){inputFile = _inputFile;}


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

std::map<std::string, std::map<std::string,int>> Parser::getSiteNameCPUInfo()
{
  std::set<std::string> site_names = this->getSiteNames();
  std::map<std::string, std::map<std::string,int>> siteNameCPUInfo;
  
  for(const auto& site: site_names){
    for(int cpu = 0; cpu < genRandNum(5,15); cpu++){
        for(int core = 0; core < genRandNum(20,100); core++){
            siteNameCPUInfo[site][site+"_cpu-"+std::to_string(cpu)] = core;}}}
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

