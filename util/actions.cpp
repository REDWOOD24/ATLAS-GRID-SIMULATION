#include "actions.h"

sg4::ActivityPtr Actions::exec_task_multi_thread_async(double flops, int cores, Job* j)
{
  auto host = sg4::this_actor::get_host();
  sg4::ExecPtr exec_activity = sg4::Exec::init()->set_flops_amount(flops)->set_thread_count(cores)->set_host(host);
  exec_activity->start();
  exec_activity->on_this_completion_cb([j](simgrid::s4u::Exec const & ex) {
    j->EXEC_time_taken += ex.get_finish_time() - ex.get_start_time();
  });
  return exec_activity;
}

sg4::ActivityPtr Actions::read_file_async(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, const std::string& filename, Job* j)
{
  auto file = fs->open(filename,"r");
  const sg_size_t file_size = fs->file_size(filename);
  file->seek(0);
  auto read_activity = file->read_async(file_size);
  read_activity->on_this_completion_cb([file,j](simgrid::s4u::Io const & io) {
    file->close();
    j->IO_time_taken      += io.get_finish_time() - io.get_start_time();
    j->IO_size_performed  += io.get_performed_ioops();
  });
  return read_activity;
}

sg4::ActivityPtr Actions::write_file_async(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, const std::string& filepath, size_t file_size, Job* j)
{
  auto file = fs->open(filepath,"w");
  auto write_activity = file->write_async(file_size*1000); //File size multiplied by 1000 to convert kB to B
  write_activity->on_this_completion_cb([file,j](simgrid::s4u::Io const & io) {
    file->close();
    j->IO_time_taken      += io.get_finish_time() - io.get_start_time();
    j->IO_size_performed  += io.get_performed_ioops();
  });
  return write_activity;
}


  
