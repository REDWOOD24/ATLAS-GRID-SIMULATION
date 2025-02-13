#include "actions.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(ATLAS_SIMULATION, "Messages specific for this s4u example");


sg4::ExecPtr Actions::exec_task_multi_thread_async(double flops, int cores)
{
    return sg4::this_actor::exec_async(flops)->set_thread_count(cores);
}

sg4::IoPtr Actions::read_file_async(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, const std::string& filename)
{
  auto file = fs->open(filename,"r");
  const sg_size_t file_size = fs->file_size(filename);
  file->seek(0);
  return file->read_async(file_size);
}

sg4::IoPtr Actions::write_file_async(const std::shared_ptr<simgrid::fsmod::FileSystem>& fs, const std::string& filepath, size_t file_size)
{
  auto file = fs->open(filepath,"w");
  return file->write_async(file_size);
}


  
