#include "host_extensions.h"

simgrid::xbt::Extension<simgrid::s4u::Host, HostExtensions> HostExtensions::EXTENSION_ID;

void HostExtensions::registerJob(Job* j) {
    simgrid::kernel::actor::simcall_answered([this, j] {
        job_ids.insert(j->id);
        cores_used      += j->cores;
        cores_available -= j->cores;
    });
    //LOG_DEBUG("Job {} Cores {} registered on host {}. Cores used: {}. Cores available: {}.",
    //j->id, j->cores, name, cores_used, cores_available);
}

void HostExtensions::onJobFinish(Job* j) {
    simgrid::kernel::actor::simcall_answered([this, j] {
        job_ids.erase(j->id);
        cores_used      -= j->cores;
        cores_available += j->cores;
    });
    //LOG_DEBUG("Job {} finished on host {}. Cores used: {}. Cores available: {}.",
    //          j->id, name, cores_used, cores_available);
}

unsigned int HostExtensions::get_cores_used() const { return cores_used; }
unsigned int HostExtensions::get_cores_available() const { return cores_available; }

static void on_host_creation(simgrid::s4u::Host& h) {
    h.extension_set<HostExtensions>(new HostExtensions(&h));
}

void simatlas_host_extension_init() {
    if (not HostExtensions::EXTENSION_ID.valid()) {
        HostExtensions::EXTENSION_ID = simgrid::s4u::Host::extension_create<HostExtensions>();
        simgrid::s4u::Host::on_creation_cb(&on_host_creation);
    }
}
