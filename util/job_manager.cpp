#include "job_manager.h"

void JOB_MANAGER::set_parser(std::unique_ptr<Parser> new_parser) {
    p = std::move(new_parser);
}

JobQueue JOB_MANAGER::get_jobs() {
    return p->getJobsInfo(-1); 
}

JobQueue JOB_MANAGER::get_jobs(int num_of_jobs)
{
  JobQueue jobs = p->getJobsInfo(num_of_jobs);
  return jobs;
}
