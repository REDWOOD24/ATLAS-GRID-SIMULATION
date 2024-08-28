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

//Information needed to a specify a Job                                                                                         
struct Job {
  std::string                       id{};
  double                            flops{};
  std::map<std::string, size_t>     input_files{};
  std::map<std::string, size_t>     output_files{};
  size_t                            input_storage{};
  size_t                            output_storage{};
};



class JOB_MANAGER
{


public:
  JOB_MANAGER(){};
 ~JOB_MANAGER(){};

  std::vector<Job> create_jobs(int num_of_jobs);

private:
  std::unique_ptr<Parser> p = std::make_unique<Parser>();

};

#endif

