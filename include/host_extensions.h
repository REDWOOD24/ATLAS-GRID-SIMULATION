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
#include "job.h"


class HostExtensions {

public:
    static simgrid::xbt::Extension<simgrid::s4u::Host, HostExtensions> EXTENSION_ID;
    explicit HostExtensions(const simgrid::s4u::Host* h);
    HostExtensions(const HostExtensions&) = delete;
    HostExtensions& operator=(const HostExtensions&) = delete;

    void registerJob(Job* j);
    void onJobFinish(Job* j);
    [[nodiscard]] unsigned int get_cores_used()      const {return cores_used;}
    [[nodiscard]] unsigned int get_cores_available() const {return cores_available;}

private:
    unsigned int cores_used;
    unsigned int cores_available;
    std::set<std::string> job_ids;
    std::string name;
};

void simatlas_host_extension_init();

#endif //HOST_EXTENSIONS_H