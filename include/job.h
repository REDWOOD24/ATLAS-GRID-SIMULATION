// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2025-02-15
// Description: Class defining basic outline of a job.
// ==============================================

#ifndef JOB_H
#define JOB_H
#include <string>
#include <unordered_map>

//Information needed to a specify a Job
struct Job {
    int                                        _id{};
    std::string                                 id{};
    std::string                                 status{};
    int                                         error_code{};
    long long                                   flops{};
    std::unordered_map<std::string, size_t>     input_files{};
    std::unordered_map<std::string, size_t>     output_files{};
    size_t                                      input_storage{};
    size_t                                      output_storage{};
    int                                         priority{};
    int                                         cores{};
    std::string                                 mount{};
    std::string                                 disk{};
    std::string                                 comp_host{};
    std::string                                 comp_site{};
    size_t                                      IO_size_performed{};
    double                                      IO_time_taken{};
    double                                      EXEC_time_taken{};
    int                                         files_read{};
    int                                         files_written{};
    double                                      memory_usage{};
    bool operator<(const Job& other) const {if(priority == other.priority){return _id > other._id;} return priority < other.priority;}
};

#endif //JOB_H
