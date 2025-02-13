// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-27
// Description: Class to create and manage Jobs.
// ==============================================


#ifndef JOB_MANAGER_H
#define JOB_MANAGER_H

#include "job.h"
#include "parser.h"
#include <queue>


using JobQueue = std::priority_queue<Job*>;


class JOB_MANAGER
{


public:
  JOB_MANAGER()= default;
 ~JOB_MANAGER()= default;

  JobQueue create_jobs(int num_of_jobs);

private:
  std::unique_ptr<Parser> p = std::make_unique<Parser>();

};

#endif

