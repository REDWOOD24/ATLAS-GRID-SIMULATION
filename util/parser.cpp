#include "parser.h"
#include <iostream>
#include <string>


std::ostream& operator<<(std::ostream& os, const JobInfo& job) {
    os << "JobInfo {"
       << "\n  jobid: " << job.jobid
       << "\n  creation_time: " << job.creation_time
       << "\n  job_status: " << job.job_status
       << "\n  job_name: " << job.job_name
       << "\n  cpu_consumption_time: " << job.cpu_consumption_time
       << "\n  computing_site: " << job.computing_site
       << "\n  destination_dataset_name: " << job.destination_dataset_name
       << "\n  destination_SE: " << job.destination_SE
      //  << "\n  grid: " << job.grid
       << "\n  source_site: " << job.source_site
      //  << "\n  destination_site: " << job.destiantion_site
       << "\n  transfer_type: " << job.transfer_type
       << "\n}";
    return os;
}

Parser::Parser(const std::string& _siteConnInfoFile, const std::string& _siteInfoFile, const std::string& _jobInfoFile)
{
  siteConnInfoFile = _siteConnInfoFile;
  siteInfoFile     = _siteInfoFile;
  jobInfoFile      = _jobInfoFile;
  this->setSiteCPUCount(); //Reason this is set before site-names is because is site has 0 cpus (no info) I don't include it.
  this->setSiteNames();
  this->setSiteGFLOPS();
}

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

void Parser::setSiteGFLOPS()
{
  std::ifstream in(siteInfoFile);
  auto j=json::parse(in);
  for (auto it = j.begin(); it != j.end(); ++it) {
    //500 flops per core and 32 cores per host (assumption)
    int gflops =  (j[it.key()]["GFLOPS"]).get<int>(); 
    siteNameGFLOPS[it.key()] =  gflops;
  }
}

std::unordered_map<std::string,int>  Parser::getSiteNameGFLOPS()
{
  return siteNameGFLOPS;
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


std::unordered_map<std::string, std::unordered_map<std::string, CPUInfo>> Parser::getSiteNameCPUInfo()
{

  std::unordered_map<std::string, std::unordered_map<std::string, CPUInfo>> siteNameCPUInfo;
  
  for(const auto& site: site_names){

    int                   num_of_cpus = siteCPUCount[site] + this->genRandNum(-3,3); //Little random offset added.
    std::vector<DiskInfo> disks       = this->getDisksInfo(site,num_of_cpus);

    //Info Printout
    std::cout << "----------------------------------------------------"     << std::endl;
    std::cout << "\033[32m" << "Adding Site .... " << site                  << std::endl;
    std::cout << "\033[35m" << "CPUS          : "  << num_of_cpus           << std::endl;
    std::cout << "\033[33m" << "DISK-INFO-CPU : "  <<  std::endl;
    for (const auto& d : disks) {
    std::cout << "NAME          : "         << std::setw(30)  << std::left  << d.name 
              << " : "     << std::setw(15) << std::right     << d.size     << std::endl;}
    std::cout << "\033[0m" << "----------------------------------------------------" << std::endl;
    
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



std::unordered_map<std::string, std::pair<double, double>> Parser::getSiteConnInfo()
 {
  std::ifstream in(siteConnInfoFile);
  auto j=json::parse(in);

  //Defining Latency in ms based on 'closeness' value in json file
  std::unordered_map<int, int> closeness_latency_map = {
        {0, 0}, {1, 10}, {2, 20}, {3, 30}, {4, 40},
        {5, 50}, {6, 60}, {7, 70}, {8, 80}, {9, 90},
        {10, 100}, {11, 110}, {12, 120}};

  //Store Site Info
  std::unordered_map<std::string, std::pair<double, double>> siteConnInfo;
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
std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> row;
    std::string cell;
    bool in_quotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];

        if (c == '"') {
            in_quotes = !in_quotes; 
        } else if (c == ',' && !in_quotes) {
           
            row.push_back(cell);
            cell.clear();
        } else {
            cell += c;
        }
    }
    row.push_back(cell);
    for (auto& field : row) {
        if (!field.empty() && field.front() == '"' && field.back() == '"') {
            field = field.substr(1, field.size() - 2); 
        }
        field.erase (std::remove_if (field.begin(), field.end(),
                                [](unsigned char c){
                                    return !std::isprint(c);
                                }),
                                field.end());
    }

    return row;
}

 std::vector<JobInfo> Parser::getJobsInfo(long max_jobs) {
    std::vector<JobInfo> jobs; 
    std::ifstream file(jobInfoFile);


    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + jobInfoFile);
    }

    std::string line;
    std::map<std::string, int> column_map;
    bool header_parsed = false;

   
    while (std::getline(file, line)) {
         std::vector<std::string> row = parseCSVLine(line);
         if (max_jobs != -1 && static_cast<long>(jobs.size()) >= max_jobs) {
            break;
        }
        
        if (!header_parsed) {
            header_parsed = true;
            int column_index = 0;
            for (std::string& column_name : row) {
                std::transform(column_name.begin(), column_name.end(), column_name.begin(), ::toupper);
                column_map.insert({column_name,column_index}); 
                column_index = column_index+1;
            }
            continue;
        }
        
        try {
            JobInfo job;
            job.jobid = std::stol(row[column_map.at("PANDAID")]);
            job.creation_time = row[column_map.at("CREATIONTIME")];
            job.job_status = row[column_map.at("JOBSTATUS")];
            job.job_name = row[column_map.at("JOBNAME")];
            job.cpu_consumption_time = std::stod(row[column_map.at("CPUCONSUMPTIONTIME")]);
            job.computing_site = row[column_map.at("COMPUTINGSITE")];
            job.destination_dataset_name = row[column_map.at("DESTINATIONDBLOCK")];
            job.destination_SE = row[column_map.at("DESTINATIONSE")];
            job.source_site = row[column_map.at("SOURCESITE")];
            job.transfer_type = row[column_map.at("TRANSFERTYPE")];
            job.core_count = row[column_map.at("CORECOUNT")].empty() ? 0 : std::stol(row[column_map.at("CORECOUNT")]);
            job.no_of_inp_files = std::stoi(row[column_map.at("NINPUTDATAFILES")]);
            job.inp_file_bytes = std::stod(row[column_map.at("INPUTFILEBYTES")]);
            job.no_of_out_files = std::stoi(row[column_map.at("NOUTPUTDATAFILES")]);
            job.out_file_bytes = std::stod(row[column_map.at("OUTPUTFILEBYTES")]);
            job.pilot_error_code = row[column_map.at("PILOTERRORCODE")];
            job.exe_error_code = row[column_map.at("EXEERRORCODE")];
            job.ddm_error_code = row[column_map.at("DDMERRORCODE")];
            job.dispatcher_error_code = row[column_map.at("JOBDISPATCHERERRORCODE")];
            job.taskbuffer_error_code = row[column_map.at("TASKBUFFERERRORCODE")];

            std::string prefix = "/input/user.input."+std::to_string(job.jobid)+".00000";
            std::string suffix = ".root";
            size_t      size_per_inp_file = job.inp_file_bytes/job.no_of_inp_files;
            for(int file = 1; file <= job.no_of_inp_files; file++){
              std::string name       = prefix + std::to_string(file)+suffix;
              job.input_files[name] = size_per_inp_file;
            }
            
            prefix           = "/output/user.output."+std::to_string(job.jobid)+".00000";
            suffix           = ".root";
            size_t      size_per_out_file = job.out_file_bytes/job.no_of_out_files;
            for(int file = 1; file <= job.no_of_out_files; file++){
              std::string name         = prefix + std::to_string(file)+suffix;
              job.output_files[name]  = size_per_out_file;
            }
    

            jobs.push_back(job);  
        } catch (const std::exception& e) {
            std::cerr << "Skipping invalid row: " << line << "\n";
            std::cerr << "Reason: " << e.what() << "\n";
        }
    }

    file.close();
    return jobs;
}



