#include "job_manager.h"


std::vector<Job> JOB_MANAGER::create_jobs(int num_of_jobs)
{
  std::vector<Job> jobs;

  for(int i = 1; i <= num_of_jobs; i++){
    Job job;
    job.id = "Job-"+std::to_string(i);
    job.flops  = p->GaussianDistribution(1e7,10000);
    job.input_storage = 0;
    job.output_storage = 0;
    
    //Random number of input files with size pulled from a Gaussian Distribution.
    std::string prefix = "/input/user.input.00000";
    std::string suffix = std::to_string(i)+".root";
    int input_files = p->genRandNum(100,200);
    for(int file = 0; file < input_files; file++){
      std::string name = prefix + std::to_string(file)+suffix;
      size_t      size = static_cast<size_t>(std::round(p->GaussianDistribution(1e7,10000)));
      job.input_files[name] = size;
      job.input_storage += size;
    }
    

    //Random number of output files with size pulled from a Gaussian Distribution.
    prefix = "/output/user.output.00000";
    suffix = std::to_string(i)+".root";
    int	output_files = p->genRandNum(100,200);
    for(int file = 0; file < output_files; file++){
      std::string name = prefix + std::to_string(file)+suffix;
      size_t      size = static_cast<size_t>(std::round(p->GaussianDistribution(1e7,10000)));
      job.output_files[name] = size;
      job.output_storage += size;
    }

    
    jobs.push_back(job);
  }
  
  return jobs;
}
