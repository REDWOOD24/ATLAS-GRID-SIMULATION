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

    // Pure virtual function must be implemented by derived classes to assign Jobs
    virtual Job* assignJob(Job* job) = 0;

    // Pure virtual function must be implemented by derived classes to assign Resources
    virtual void getResourceInformation(simgrid::s4u::NetZone* platform) = 0;

    // Virtual function can be implemented by derived classes when a job finishes
    virtual void onJobEnd(Job* job){};

    // Virtual function can be implemented by derived classes if they want to execute code on simulation end
    virtual void onSimulationEnd(){};

    // Delete copy constructor and copy assignment operator
    DispatcherPlugin(const DispatcherPlugin&) = delete;
    DispatcherPlugin& operator=(const DispatcherPlugin&) = delete;

    // Delete move constructor and move assignment operator
    DispatcherPlugin(DispatcherPlugin&&) = delete;
    DispatcherPlugin& operator=(DispatcherPlugin&&) = delete;

};

#endif //DISPATCHERPLUGIN_H
