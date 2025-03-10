#include "platform.h"
#include <iostream>

sg4::NetZone* Platform::create_platform(const std::string& platform_name)
{
return sg4::create_full_zone(platform_name);
}

sg4::NetZone* Platform::create_site(sg4::NetZone* platform, const std::string& site_name, std::unordered_map<std::string, CPUInfo>& cpuInfo, long siteGFLOPS)
{
  //Create the Site
  auto* site = sg4::create_star_zone(site_name);
  site->set_parent(platform);
  site->set_property("gflops",std::to_string(siteGFLOPS));
  
  //Create CPUS and Cores
  for (const auto& cpu: cpuInfo)
    {
    const auto        cpuname    = cpu.first;
    int               cores      = cpu.second.cores;
    sg4::Host*        host       = site->create_host(cpuname, cpu.second.speed);
    const auto        BW_CPU     = cpu.second.BW_CPU;
    const auto        LAT_CPU    = std::to_string(cpu.second.LAT_CPU)+"ms";
    const auto        linkname   = "link_" + cpuname;
    const sg4::Link*  link       = site->create_split_duplex_link(linkname, BW_CPU)->set_latency(LAT_CPU)->seal();

    host->set_core_count(cores);
    host->set_property("ram",cpu.second.ram);
    host->set_property("wattage_per_state",std::to_string(2*cores)+":"+std::to_string(cores)+":"+std::to_string(10*cores));
    host->set_property("wattage_off",std::to_string(0.1*cores));
    site->add_route(host, nullptr,{{link,sg4::LinkInRoute::Direction::UP}}, true);
    if(cpuname== std::string(site_name + "_cpu-0")){site->set_gateway(host->get_netpoint());} // Use the first host as a router

    for(const auto& d: cpu.second.disk_info)
      {
      auto _disk = host->create_disk(d.name,d.read_bw,d.write_bw)->set_property("mount",d.mount);
      auto _ods  = simgrid::fsmod::OneDiskStorage::create(site_name+cpuname+d.name+"storage", _disk);
      auto _fs    = simgrid::fsmod::FileSystem::create(site_name+cpuname+d.name+"filesystem");
      _fs->mount_partition(d.mount, _ods, d.size);
      simgrid::fsmod::FileSystem::register_file_system(site,_fs);
      }
      host->seal();
    }

  //cleanup
  cpuInfo.clear();
  std::unordered_map<std::string, CPUInfo>().swap(cpuInfo);
  return site;
}

std::unordered_map<std::string, sg4::NetZone*>  Platform::create_sites(sg4::NetZone* platform,  std::unordered_map<std::string, std::unordered_map<std::string, CPUInfo>>& siteNameCPUInfo, std::unordered_map<std::string,int>& siteNameGFLOPS)
{
   std::unordered_map<std::string, sg4::NetZone*> sites;
   std::cout << "Inititalizing SimGrid Platform with all Sites .......";   
   for (auto& sitePair : siteNameCPUInfo) {
     const std::string&              site_name = sitePair.first;
     std::unordered_map<std::string, CPUInfo>& cpuInfo   = sitePair.second;
     sites[site_name] =  this->create_site(platform, site_name, cpuInfo, siteNameGFLOPS[site_name]);
     std::cout << ".";
    }
   std::cout << std::endl;
   
   //cleanup
   siteNameCPUInfo.clear();
   std::unordered_map<std::string, std::unordered_map<std::string, CPUInfo>>().swap(siteNameCPUInfo);
   return sites;
}


void Platform::initialize_site_connections(sg4::NetZone* platform, std::unordered_map<std::string, std::pair<double, double>>& siteConnInfo, std::unordered_map<std::string, sg4::NetZone*>& sites)
{

  //Creating Sites and the Connections between them
  for (const auto& siteConn : siteConnInfo) {
    const auto  conn           = siteConn.first;
    const auto  src_name       = conn.substr(0, conn.find(':'));
    const auto  dst_name       = conn.substr(conn.find(':')+1);
    const auto  linkname       = "link_" + conn;
    const auto  latency        = std::to_string(siteConn.second.first)+"ms";
    const auto  bandwidth      = siteConn.second.second*1.25e7; //convert mbps to bytes/s

    //Source
    const sg4::NetZone* src    = sites.at(src_name);

    //Destination
    const sg4::NetZone* dst    = sites.at(dst_name);;

    //Create Link
    const sg4::Link* interzonal_link = platform->create_link(linkname, bandwidth)->set_latency(latency)->seal();

    //Add to Platform
    platform->add_route(src,  dst, {sg4::LinkInRoute(interzonal_link)});
  }
}

void Platform::initialize_simgrid_plugins()
{
  //sg_host_energy_plugin_init();
}

void Platform::initialize_job_server(sg4::NetZone* platform,  std::unordered_map<std::string, std::unordered_map<std::string, CPUInfo>>& siteNameCPUInfo, std::unordered_map<std::string, sg4::NetZone*>& sites)
{
  //Create the JOB SERVER Site
  auto*              JOB_SERVER_site       = sg4::create_star_zone("JOB-SERVER");
  JOB_SERVER_site->set_parent(platform);

  //Create the JOB SERVER host
  const double       JOB_SERVER_CPU_SPEED  = 1e9;   //  FLOPS/s
  const double       JOB_SERVER_BW_CPU     = 1e11;  //  bytes/s
  const double       JOB_SERVER_LAT_CPU    = 0;     //  ms
  const int          JOB_SERVER_cores      = 32;
  const std::string  JOB_SERVER_RAM        = std::string("16GiB");
  const sg4::Link*   JOB_SERVER_link       = JOB_SERVER_site->create_split_duplex_link("link_JOB-SERVER_cpu-0", JOB_SERVER_BW_CPU)->set_latency(JOB_SERVER_LAT_CPU)->seal();
  sg4::Host*         JOB_SERVER_host       = JOB_SERVER_site->create_host("JOB-SERVER_cpu-0", JOB_SERVER_CPU_SPEED);

  JOB_SERVER_host->set_core_count(JOB_SERVER_cores);
  JOB_SERVER_host->set_property("ram",JOB_SERVER_RAM);
  JOB_SERVER_site->add_route(JOB_SERVER_host, nullptr,{{JOB_SERVER_link,sg4::LinkInRoute::Direction::UP}}, true);
  JOB_SERVER_site->set_gateway(JOB_SERVER_host->get_netpoint());
  JOB_SERVER_host->seal();

  //Connect JOB SERVER to rest of Sites
  for (auto& sitePair : siteNameCPUInfo)
    {
    const std::string    site_name          = sitePair.first;
    const auto           linkname           = "link_JOB_SERVER:" + site_name;
    const std::string    latency            = "0ms";
    const double         bandwidth          = 100*1.25e+7; //Bandwidth of 100 Mbps converted into bytes/s
    const sg4::NetZone*  site               = sites.at(sitePair.first);
    const sg4::Link*     server_site_link   = platform->create_link(linkname, bandwidth)->set_latency(latency)->seal();

    platform->add_route(JOB_SERVER_site,  site, {sg4::LinkInRoute(server_site_link)});
    }
}
