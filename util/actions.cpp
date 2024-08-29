#include "actions.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(ATLAS_SIMULATION, "Messages specific for this s4u example");

void Actions::exec_task_multi_thread(double flops, int cores, std::string exec_host)
{
   sg4::this_actor::exec_init(flops)->set_thread_count(cores)->start()->wait();
   XBT_INFO("Finished executing '%s' flops' on host '%s'", std::to_string(flops).c_str(), exec_host.c_str());
  
}

void Actions::exec_task_multi_thread_async(double flops, int cores, std::string exec_host)
{
    sg4::this_actor::exec_async(flops)->set_thread_count(cores);
}

void Actions::read(std::string filename, const_sg_host_t exec_host)
{
  auto* file = sg4::File::open(filename,exec_host ,nullptr);
  const sg_size_t file_size = file->size();
  file->seek(0);
  const sg_size_t read_bits =file->read(file_size);
  XBT_INFO("Finished reading file '%s' of size '%s' on host '%s'", filename.c_str(), std::to_string(read_bits).c_str(), exec_host->get_cname());
  file->close();
}

void Actions::write(std::string filename, size_t file_size, const_sg_host_t exec_host)
{
  auto* file = sg4::File::open(filename, exec_host ,nullptr);
  const sg_size_t write_bits = file->write(file_size);
  XBT_INFO("Finished writing file '%s' of size '%s' on host '%s'", filename.c_str(), std::to_string(write_bits).c_str(), exec_host->get_cname());
  file->close();
}

sg_size_t Actions::size(std::string filename, const_sg_host_t exec_host)
{
  auto* file = sg4::File::open(filename, exec_host, nullptr);
  sg_size_t file_size = file->size();
  file->close();
  return file_size;
}

void Actions::remove(std::string filename, const_sg_host_t exec_host)
{
  auto* file = sg4::File::open(filename, exec_host, nullptr);
  file->unlink();
  file->close();
}



  
