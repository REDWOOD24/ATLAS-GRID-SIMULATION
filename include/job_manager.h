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


using JobQueue = std::priority_queue<Job*>;


class JOB_MANAGER
{


public:
  JOB_MANAGER()= default;
 ~JOB_MANAGER()= default;

  JobQueue create_jobs(int num_of_jobs);
  JobQueue get_jobs();
  JobQueue get_jobs(int num_of_jobs);
  void set_parser(std::unique_ptr<Parser> new_parser);

private:
  std::unique_ptr<Parser> p = std::make_unique<Parser>();

};

#endif