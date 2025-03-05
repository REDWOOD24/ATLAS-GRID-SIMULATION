#include "job_manager.h"


JobQueue JOB_MANAGER::create_jobs(int num_of_jobs)
{
  JobQueue jobs;

  // for(int i = 1; i <= num_of_jobs; i++){

  //   Job* job = new Job();
  //   job->_id            = i;
  //   job->id             = "Job-"+std::to_string(i);
  //   job->flops          = abs(std::round(p->GaussianDistribution(1e10,1e10)));
  //   job->input_storage  = 0;
  //   job->output_storage = 0;
  //   job->files_read     = 0;
  //   job->files_written  = 0;
  //   job->priority       = 2; //p->genRandNum(1,10);
  //   job->memory_usage   = abs(p->GaussianDistribution(100,100));
  //   job->cores          = 1;
  //   //Random number of input files with size pulled from a Gaussian Distribution.
  //   std::string prefix = "user.input."+std::to_string(i)+".00000";
  //   std::string suffix = ".root";
  //   int input_files    = p->genRandNum(5,10);

  //   for(int file = 1; file <= input_files; file++){
  //     std::string name       = prefix + std::to_string(file)+suffix;
  //     size_t      size       = static_cast<size_t>(std::round(p->GaussianDistribution(1e5,10000)));
  //     job->input_files[name] = size;
  //     job->input_storage    += size;
  //   }
    

  //   //Random number of output files with size pulled from a Gaussian Distribution.
  //   prefix           = "user.output."+std::to_string(i)+".00000";
  //   suffix           = ".root";
  //   int	output_files = 1;

  //   for(int file = 1; file <= output_files; file++){
  //     std::string name         = prefix + std::to_string(file)+suffix;
  //     size_t      size         = static_cast<size_t>(std::round(p->GaussianDistribution(1e5,10000)));
  //     job->output_files[name]  = size;
  //     job->output_storage     += size;
  //   }

  //   //Output metrics set to 0
  //   job->IO_size_performed = 0;
  //   job->IO_time_taken     = 0.0;
  //   job->EXEC_time_taken   = 0.0;

  //   //Add to list of jobs
  //   jobs.push(job);
  // }
  
  
  return jobs;
}

void JOB_MANAGER::set_parser(std::unique_ptr<Parser> new_parser) {
  p = std::move(new_parser);
}

JobQueue JOB_MANAGER::get_jobs() {
  return p->getJobs(-1); 
}

JobQueue JOB_MANAGER::get_jobs(int num_of_jobs)
{
JobQueue jobs = p->getJobs(num_of_jobs);
return jobs;
}

