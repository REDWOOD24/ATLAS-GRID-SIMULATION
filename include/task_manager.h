// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-27
// Description: Class to create and manage Tasks.
// Version: 1.0
// ==============================================


#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

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

//Information needed to a specify a Task                                                                                         
struct Task {
  int                              _id{};
  std::string                       id{};
  int                               flops{};
  std::unordered_map<std::string, size_t>     input_files{};
  std::unordered_map<std::string, size_t>     output_files{};
  size_t                            input_storage{};
  size_t                            output_storage{};
  int                               priority{};    
  bool operator<(const Task& other) const {if(priority == other.priority){return _id > other._id;} return priority < other.priority;}
};

using TaskQueue = std::priority_queue<Task, std::vector<Task>>;


class TASK_MANAGER
{


public:
  TASK_MANAGER(){};
 ~TASK_MANAGER(){};

  TaskQueue create_tasks(int num_of_tasks);

private:
  std::unique_ptr<Parser> p = std::make_unique<Parser>();

};

#endif

