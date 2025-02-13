//
// Created by Raees Khan on 10.02.2025.
//

#ifndef JOB_EXECUTOR_H
#define JOB_EXECUTOR_H

#include "DispatcherPlugin.h"
#include "PluginLoader.h"
#include "job_manager.h"
#include "actions.h"
#include "H5Cpp.h"

//Output to Save
struct output {
    char*                      id{};
    float                      IO_size{};
    float                      IO_time{};
    float                      EXEC_time{};
};

class JOB_EXECUTOR
{


public:
    JOB_EXECUTOR(sg4::Engine* _e, const std::string& _outputFile);
    ~JOB_EXECUTOR()= default;


    void set_dispatcher(const std::string& dispatcherPath, sg4::NetZone* platform);
    void update_disk_content(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, std::unordered_map<std::string, size_t>  input_files);
    void execute_jobs(JobQueue jobs);
    static void execute_job(Job* job);
    static void receiver(const std::string& MQ_name);
    void start_receivers();
    void h5init();
    void kill_simulation();
    void print_output();

private:
    sg4::Engine*                         e;
    std::unique_ptr<DispatcherPlugin>    dispatcher;
    std::string                          outputFile;
    static H5::H5File                    h5_file;
    static H5::CompType                  datatype;
    static std::vector<output*>          outputs;
};

#endif //JOB_EXECUTOR_H
