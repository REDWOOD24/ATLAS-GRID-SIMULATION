#include "job_manager.h"


JobQueue JOB_MANAGER::create_jobs(int num_of_jobs)
{
  JobQueue jobs;

  for(int i = 1; i <= num_of_jobs; i++){

    Job job;
    job._id            = i;
    job.id             = "Job-"+std::to_string(i);
    job.flops          = std::round(p->GaussianDistribution(1e7,10000));
    job.input_storage  = 0;
    job.output_storage = 0;
    job.priority       = 2; //p->genRandNum(1,10);
    
    //Random number of input files with size pulled from a Gaussian Distribution.
    std::string prefix = "/input/user.input."+std::to_string(i)+".00000";
    std::string suffix = ".root";
    int input_files    = p->genRandNum(100,200);

    for(int file = 1; file <= input_files; file++){
      std::string name      = prefix + std::to_string(file)+suffix;
      size_t      size      = static_cast<size_t>(std::round(p->GaussianDistribution(1e7,10000)));
      job.input_files[name] = size;
      job.input_storage    += size;
    }
    

    //Random number of output files with size pulled from a Gaussian Distribution.
    prefix           = "/output/user.output."+std::to_string(i)+".00000";
    suffix           = ".root";
    int	output_files = p->genRandNum(100,200);

    for(int file = 1; file <= output_files; file++){
      std::string name       = prefix + std::to_string(file)+suffix;
      size_t      size       = static_cast<size_t>(std::round(p->GaussianDistribution(1e7,10000)));
      job.output_files[name] = size;
      job.output_storage     += size;
    }
    
    jobs.push(job);
  }
  
  return jobs;
}
