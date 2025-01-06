#include "platform.h"
#include <iostream>

sg4::NetZone* Platform::create_platform(const std::string& platform_name)
{
return sg4::create_full_zone(platform_name);
}

sg4::NetZone* Platform::create_site(sg4::NetZone* platform, const std::string& site_name, std::unordered_map<std::string, CPUInfo>& cpuInfo, int siteGFLOPS)
{
  //Create the Site
  auto* site = sg4::create_star_zone(site_name);
  site->set_parent(platform);
  site->set_property("gflops",std::to_string(siteGFLOPS));
  //Create CPUS and Cores
  for (const auto& cpu: cpuInfo) {
    const std::string cpuname    = cpu.first;
    int               cores      = cpu.second.cores; 
    sg4::Host*        host       = site->create_host(cpuname, cpu.second.speed);
    const double      BW_CPU     = cpu.second.BW_CPU;
    const double      LAT_CPU    = cpu.second.LAT_CPU;
    const auto&       linkname   = "link_" + cpuname;
    const sg4::Link*  link       = site->create_split_duplex_link(linkname, BW_CPU)->set_latency(LAT_CPU)->seal();
    host->set_core_count(cores);
    host->set_property("ram",cpu.second.ram);
    host->set_property("wattage_per_state",std::to_string(2*cores)+":"+std::to_string(cores)+":"+std::to_string(10*cores));
    host->set_property("wattage_off",std::to_string(0.1*cores));
    site->add_route(host->get_netpoint(), nullptr, nullptr, nullptr,{{link, sg4::LinkInRoute::Direction::UP}}, true);
    if(cpuname== std::string(site_name + "_cpu-0")){site->set_gateway(host->get_netpoint());} // Use the first host as a router
    for(const auto& d: cpu.second.disk_info){
      host->create_disk(d.name,d.read_bw,d.write_bw)->set_property("size",d.size)->set_property("mount",d.mount)->set_property("content","")->seal();}
      host->seal();}

  //cleanup
  cpuInfo.clear();
  std::unordered_map<std::string, CPUInfo>().swap(cpuInfo);
  return site;
}

std::unordered_map<std::string, sg4::NetZone*>  Platform::create_sites(sg4::NetZone* platform,  std::unordered_map<std::string, std::unordered_map<std::string, CPUInfo>>& siteNameCPUInfo,std::unordered_map<std::string,int>& siteNameGFLOPS)
{
   std::unordered_map<std::string, sg4::NetZone*> sites;
   std::cout << "Inititalizing SimGrid Platform with all Sites .......";   
   for (auto& sitePair : siteNameCPUInfo) {
     const std::string&              site_name = sitePair.first;
     std::unordered_map<std::string, CPUInfo>& cpuInfo   = sitePair.second;
     sites[site_name] =  this->create_site(platform, site_name, cpuInfo, siteNameGFLOPS[site_name]);
     std::cout << ".";}
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
    const auto& conn           = siteConn.first;
    const auto& src_name       = conn.substr(0, conn.find(':'));
    const auto& dst_name       = conn.substr(conn.find(':')+1);
    const auto& linkname       = "link_" + conn;
    double      latency        = siteConn.second.first;
    double      bandwidth      = siteConn.second.second;

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

void Platform::initialize_plugins()
{
  sg_storage_file_system_init();
  //sg_host_energy_plugin_init();
}


  
