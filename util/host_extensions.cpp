#include "host_extensions.h"
#include <simgrid/simcall.hpp>
#include "logger.h" 

simgrid::xbt::Extension<simgrid::s4u::Host, HostExtensions> HostExtensions::EXTENSION_ID;
static void on_host_creation(simgrid::s4u::Host& h);

HostExtensions::HostExtensions(const simgrid::s4u::Host* h)
{
    cores_used = 0;
    cores_available = h->get_core_count();
    name = h->get_name();
}

void HostExtensions::registerJob(Job* j)
{
    // job_ids.insert(j->id);
    // cores_used        += j->cores;
    // cores_available   -= j->cores;
    //    //@AskFred
    simgrid::kernel::actor::simcall_answered([this, j]
    { 
        job_ids.insert(j->id);
        cores_used        += j->cores;
        cores_available   -= j->cores;
    });
    LOG_DEBUG("Job {} Cores {} registered on host {}. Cores used: {}. Cores available: {}.", j->id, j->cores ,name, cores_used, cores_available);
 
}

void HostExtensions::onJobFinish(Job* j)
{
    // if(job_ids.find(j->id) != job_ids.end())
    // {
       
    // }
    // job_ids.erase(j->id);
    // cores_used        -= j->cores;
    // cores_available   += j->cores;
    
    // @AskFred
    // remove this 
    simgrid::kernel::actor::simcall_answered([this, j]
    {   
        job_ids.erase(j->id);
        cores_used        -= j->cores;
        cores_available   += j->cores;
    });
    LOG_DEBUG("Job {} finished on host {}. Cores used: {}. Cores available: {}.", j->id, name, cores_used, cores_available);
    // auto all_sites = dispatcher->getPlatform()->get_children();
    // for(const auto& site: all_sites)
    // {
    //     if(site->get_name() == std::string("JOB-SERVER")){
    //         LOG_DEBUG( "Job server needs to be resumed ");
            
    //     }

    // }
}

void simatlas_host_extension_init()
{
    if (not HostExtensions::EXTENSION_ID.valid()) {
        HostExtensions::EXTENSION_ID = simgrid::s4u::Host::extension_create<HostExtensions>();
        simgrid::s4u::Host::on_creation_cb(&on_host_creation);
    }
}

static void on_host_creation(simgrid::s4u::Host& h)
{
    h.extension_set<HostExtensions>(new HostExtensions(&h));
}
