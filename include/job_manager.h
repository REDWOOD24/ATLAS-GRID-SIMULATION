// ==============================================
// Author: Sairam Sri Vatsavai
// Email: ssrivatsa@bnl.gov
// Created Date: 2024-12-11
// Description: Class to create and manage Jobs.
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


using JobQueue = std::vector<JobInfo>;


class JOB_MANAGER
{


public:
  JOB_MANAGER(){};
 ~JOB_MANAGER(){};

  JobQueue create_jobs(int num_of_jobs);
  JobQueue get_jobs();
  JobQueue get_jobs(int num_of_jobs);
  void set_parser(std::unique_ptr<Parser> new_parser);


private:
  std::unique_ptr<Parser> p = std::make_unique<Parser>();
};

#endif

