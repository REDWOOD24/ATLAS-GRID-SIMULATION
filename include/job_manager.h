// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-27
// Description: Class to create and manage Jobs.
// Version: 1.0
// ==============================================


#ifndef JOB_MANAGER_H
#define JOB_MANAGER_H

#include <map>
#include <set>
#include <fstream>
#include <string>
#include <math.h>
#include <random>
#include <vector>
#include "parser.h"
#include <queue>
#include <vector>
#include <functional>

//Information needed to a specify a Job                                                                                         
struct Job {
  int                              _id{};
  std::string                       id{};
  int                               flops{};
  std::map<std::string, size_t>     input_files{};
  std::map<std::string, size_t>     output_files{};
  size_t                            input_storage{};
  size_t                            output_storage{};
  int                               priority{};    
  bool operator<(const Job& other) const {if(priority == other.priority){return _id > other._id;} return priority < other.priority;}
};

using JobQueue = std::priority_queue<Job, std::vector<Job>>;


class JOB_MANAGER
{


public:
  JOB_MANAGER(){};
 ~JOB_MANAGER(){};

  JobQueue create_jobs(int num_of_jobs);

private:
  std::unique_ptr<Parser> p = std::make_unique<Parser>();

};

#endif

