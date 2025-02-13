//
// Created by Raees Khan on 10.02.2025.
//

#ifndef JOB_H
#define JOB_H
#include <string>
#include <unordered_map>

//Information needed to a specify a Job
struct Job {
    int                                         _id{};
    std::string                                  id{};
    int                                          flops{};
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
    bool operator<(const Job& other) const {if(priority == other.priority){return _id > other._id;} return priority < other.priority;}
};

#endif //JOB_H
