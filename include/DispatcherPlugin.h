// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2025-01-15
// Description: Abstract class to develop job allocation plugins.
// ==============================================


#ifndef DISPATCHERPLUGIN_H
#define DISPATCHERPLUGIN_H

#include "job.h"
#include <simgrid/s4u.hpp>

class DispatcherPlugin {
public:
    // Constructor
    DispatcherPlugin(){};

    // Destructor
    virtual ~DispatcherPlugin() = default;

    // Pure virtual function to be implemented by derived classes to assign Jobs
    virtual Job* assignJob(Job* job) = 0;

    // Pure virtual function to be implemented by derived classes to assign Resources
    virtual void assignResources(simgrid::s4u::NetZone* platform) = 0;

    // Delete copy constructor and copy assignment operator
    DispatcherPlugin(const DispatcherPlugin&) = delete;
    DispatcherPlugin& operator=(const DispatcherPlugin&) = delete;

    // Delete move constructor and move assignment operator
    DispatcherPlugin(DispatcherPlugin&&) = delete;
    DispatcherPlugin& operator=(DispatcherPlugin&&) = delete;

};

#endif //DISPATCHERPLUGIN_H
