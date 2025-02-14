#include "job_executor.h"


JOB_EXECUTOR::JOB_EXECUTOR(const std::string& _outputFile){outputFile = _outputFile;}

H5::H5File			  JOB_EXECUTOR::h5_file;
H5::CompType		  JOB_EXECUTOR::datatype;
std::vector<output*>  JOB_EXECUTOR::outputs;

void JOB_EXECUTOR::h5init()
{
	h5_file = H5::H5File(this->outputFile.c_str() , H5F_ACC_TRUNC);
	datatype = sizeof(output);                                                 
	datatype.insertMember("ID",                  HOFFSET(output, id),                         H5::StrType(H5::PredType::C_S1, H5T_VARIABLE));
	datatype.insertMember("FLOPS EXECUTED",      HOFFSET(output,IO_size),			  H5::PredType::NATIVE_UINT);
	datatype.insertMember("FILES READ SIZE",     HOFFSET(output,IO_time),			  H5::PredType::NATIVE_UINT);
	datatype.insertMember("FILES WRITTEN SIZE",  HOFFSET(output,EXEC_time),			  H5::PredType::NATIVE_UINT);
}


void JOB_EXECUTOR::set_dispatcher(const std::string& dispatcherPath, sg4::NetZone* platform){
	PluginLoader<DispatcherPlugin> plugin_loader;
	dispatcher = plugin_loader.load(dispatcherPath);
	dispatcher->assignResources(platform);
}

void JOB_EXECUTOR::execute_jobs(JobQueue jobs)
{
	const auto* e = sg4::Engine::get_instance();
	while (!jobs.empty())
	{
	  Job* _job = jobs.top();
	  Job* job = dispatcher->assignJob(_job);
	  std::unordered_map<std::string, size_t>  input_files   = job->input_files;
	  auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(e->netzone_by_name_or_null(job->comp_site)).at(job->comp_site+job->comp_host+job->disk+"filesystem");
	  update_disk_content(fs,input_files,job);
	  sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(job->comp_host+"-MQ");
	  mqueue->put(job);
	  jobs.pop();
	}
}


void JOB_EXECUTOR::execute_job(Job* job)
{

        //Parse Job Info
        std::string                              id            = job->id;
        int                                      flops         = job->flops;
        std::unordered_map<std::string, size_t>  input_files   = job->input_files;
        std::unordered_map<std::string, size_t>  output_files  = job->output_files;
        std::string				     site          = job->comp_site;
        std::string                              read_host     = job->comp_host;
        std::string                              comp_host     = job->comp_host;
        std::string                              write_host    = job->comp_host;
        std::string				     read_mount    = job->mount;
        std::string				     write_mount   = job->mount;
        int                                      cores         = job->cores;
        std::string                              disk          = job->disk;
        delete                                                   job;

	//Create Output
        output* o;
	outputs.push_back(o);
	o->id        = new char[id.size() + 1];
	o->IO_size   = 0.0;
	o->IO_time   = 0.0;
	o->EXEC_time = 0.0;
	std::strcpy(o->id, id.c_str());

	//Get Engine Instance
	const auto* e = sg4::Engine::get_instance();

	//Find FileSystem to Read and Write (same for now)
	auto fs = simgrid::fsmod::FileSystem::get_file_systems_by_netzone(e->netzone_by_name_or_null(site)).at(site+comp_host+disk+"filesystem");

	//Update output whenever an io or compute operation finishes.
	sg4::Io::on_completion_cb([&](const simgrid::s4u::Io& io) {
	  o->IO_size     += io.get_performed_ioops();
	  o->IO_time     += io.get_start_time() - io.get_finish_time();});

	sg4::Exec::on_completion_cb([&](const simgrid::s4u::Exec& ex) {
		o->EXEC_time     += ex.get_start_time() - ex.get_finish_time();});

	//Read, Compute, Write
	for(const auto& inputfile: input_files){Actions::read_file_async(fs,read_mount+inputfile.first)->start();}
	Actions::exec_task_multi_thread_async(flops,cores)->start();
	for(const auto& outputfile: output_files){Actions::write_file_async(fs,read_mount+outputfile.first,outputfile.second)->start();}

}

void JOB_EXECUTOR::receiver(const std::string& MQ_name)
{

  sg4::MessageQueue* mqueue = sg4::MessageQueue::by_name(MQ_name);
  for (bool cont = true; cont;)
  {
      Job* job;
      sg4::MessPtr mess = mqueue->get_async<Job>(&job);
      mess->wait();
      execute_job(job);
   }
}

void JOB_EXECUTOR::start_receivers()
{
	const auto* eng = sg4::Engine::get_instance();
	auto hosts      = eng->get_all_hosts();
	for(const auto& host: hosts)
	{
	  sg4::Actor::create(host->get_name()+"-actor", host, this->receiver, host->get_name()+"-MQ");
	}
	eng->run();
}

void JOB_EXECUTOR::kill_simulation()
{
  const auto* eng = sg4::Engine::get_instance();
  auto hosts      = eng->get_all_hosts();
  for(const auto& host: hosts)
    {
      host->turn_off();
    }
}



void JOB_EXECUTOR::update_disk_content(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, const std::unordered_map<std::string, size_t>&  input_files, Job* j)
{
  const auto* e = sg4::Engine::get_instance();
  for(const auto& d: e->host_by_name(j->comp_host)->get_disks())
  {if(std::string(d->get_cname()) == j->disk){j->mount = d->get_property("mount"); break;}}
  if(j->mount.empty()){throw std::runtime_error("Read Disk mount point not found.");}
  for(const auto& inputfile: input_files){fs->create_file(j->mount + inputfile.first,  std::to_string(inputfile.second)+"kB");}
}

void JOB_EXECUTOR::print_output()
{
for (output* o: outputs)
  {
    std::cout << o->IO_time << " " << o->IO_size << std::endl;
  }
}
