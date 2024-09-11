#include "task_manager.h"


TaskQueue TASK_MANAGER::create_tasks(int num_of_tasks)
{
  TaskQueue tasks;

  for(int i = 1; i <= num_of_tasks; i++){

    Task task;
    task._id            = i;
    task.id             = "Task-"+std::to_string(i);
    task.flops          = std::round(p->GaussianDistribution(1e7,10000));
    task.input_storage  = 0;
    task.output_storage = 0;
    task.priority       = 2; //p->genRandNum(1,10);
    
    //Random number of input files with size pulled from a Gaussian Distribution.
    std::string prefix = "/input/user.input."+std::to_string(i)+".00000";
    std::string suffix = ".root";
    int input_files    = p->genRandNum(100,200);

    for(int file = 1; file <= input_files; file++){
      std::string name      = prefix + std::to_string(file)+suffix;
      size_t      size      = static_cast<size_t>(std::round(p->GaussianDistribution(1e7,10000)));
      task.input_files[name] = size;
      task.input_storage    += size;
    }
    

    //Random number of output files with size pulled from a Gaussian Distribution.
    prefix           = "/output/user.output."+std::to_string(i)+".00000";
    suffix           = ".root";
    int	output_files = p->genRandNum(100,200);

    for(int file = 1; file <= output_files; file++){
      std::string name       = prefix + std::to_string(file)+suffix;
      size_t      size       = static_cast<size_t>(std::round(p->GaussianDistribution(1e7,10000)));
      task.output_files[name] = size;
      task.output_storage     += size;
    }
    
    tasks.push(task);
  }
  
  return tasks;
}
