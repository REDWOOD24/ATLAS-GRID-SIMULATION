// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2025-02-26
// Description: Class to extend the notion of a SimGrid Host to carry information about "Job slots".
// ==============================================

#ifndef HOST_EXTENSIONS_H
#define HOST_EXTENSIONS_H

#include <simgrid/s4u.hpp>
#include <xbt/Extendable.hpp>
#include <simgrid/simcall.hpp>
#include <set>
#include <string>
#include "job.h"
#include "logger.h"

class HostExtensions {
public:
    static simgrid::xbt::Extension<simgrid::s4u::Host, HostExtensions> EXTENSION_ID;

    explicit HostExtensions(const simgrid::s4u::Host* h)
        : cores_used(0), cores_available(h->get_core_count()), name(h->get_name()) {}

    HostExtensions(const HostExtensions&) = delete;
    HostExtensions& operator=(const HostExtensions&) = delete;

    void registerJob(Job* j) {
        simgrid::kernel::actor::simcall_answered([this, j] {
            job_ids.insert(j->id);
            cores_used      += j->cores;
            cores_available -= j->cores;
        });
        LOG_DEBUG("Job {} Cores {} registered on host {}. Cores used: {}. Cores available: {}.",
                  j->id, j->cores, name, cores_used, cores_available);
    }

    void onJobFinish(Job* j) {
        simgrid::kernel::actor::simcall_answered([this, j] {
            job_ids.erase(j->id);
            cores_used      -= j->cores;
            cores_available += j->cores;
        });
        LOG_DEBUG("Job {} finished on host {}. Cores used: {}. Cores available: {}.",
                  j->id, name, cores_used, cores_available);
    }

    [[nodiscard]] unsigned int get_cores_used() const { return cores_used; }
    [[nodiscard]] unsigned int get_cores_available() const { return cores_available; }

private:
    unsigned int cores_used;
    unsigned int cores_available;
    std::set<std::string> job_ids;
    std::string name;
};

inline simgrid::xbt::Extension<simgrid::s4u::Host, HostExtensions> HostExtensions::EXTENSION_ID;

inline static void on_host_creation(simgrid::s4u::Host& h) {
    h.extension_set<HostExtensions>(new HostExtensions(&h));
}

inline void simatlas_host_extension_init() {
    if (not HostExtensions::EXTENSION_ID.valid()) {
        HostExtensions::EXTENSION_ID = simgrid::s4u::Host::extension_create<HostExtensions>();
        simgrid::s4u::Host::on_creation_cb(&on_host_creation);
    }
}

#endif // HOST_EXTENSIONS_H
