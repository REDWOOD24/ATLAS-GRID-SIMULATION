#include "storage.h"
#include <iostream>

XBT_LOG_NEW_DEFAULT_CATEGORY(Storage, "Messages specific for this simulation");

void Storage::get_disk_info()
{
  std::vector<sg4::Disk*> const& disks = sg4::Host::current()->get_disks();
  XBT_INFO("Storage info on %s:", sg4::Host::current()->get_cname());

  // for (auto const& d : disks){
  //	 XBT_INFO(" %s (%s) Used: %llu; Free: %llu; Total: %llu.", d->get_cname(), sg_disk_get_mount_point(d), sg_disk_get_size_used(d), sg_disk_get_size_free(d), sg_disk_get_size(d));}
}

