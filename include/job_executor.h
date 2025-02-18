// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2025-02-15
// Description: Class to execute Jobs.
// ==============================================

#ifndef JOB_EXECUTOR_H
#define JOB_EXECUTOR_H

#include "DispatcherPlugin.h"
#include "PluginLoader.h"
#include "job_manager.h"
#include "actions.h"

class JOB_EXECUTOR
{


public:
    explicit JOB_EXECUTOR(const std::string& _outputFile);
    ~JOB_EXECUTOR()= default;


    static void set_dispatcher(const std::string& dispatcherPath, sg4::NetZone* platform);
    static void update_disk_content(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, const std::unordered_map<std::string, size_t>&  input_files, Job* j);
    static void execute_jobs(JobQueue jobs);
    static void execute_job(Job* j);
    void        start_job_execution(JobQueue jobs);
    static void receiver(const std::string& MQ_name);
    static void start_receivers();
    static void kill_simulation();
    static void print_output(JobQueue jobs);
    static void attach_callbacks();


private:
    static std::unique_ptr<DispatcherPlugin>    dispatcher;
    std::string                                 outputFile;
};

#endif //JOB_EXECUTOR_H
