#include "DispatcherPlugin.h"
#include "simple_dispatcher.h"
// #include "logger.h"  // <-- Include the logger header

class SimpleDispatcherPlugin : public DispatcherPlugin {

public:
    SimpleDispatcherPlugin();
    virtual Job* assignJob(Job* job) final override;
    virtual sg4::NetZone* getPlatform() final override;
    virtual void getResourceInformation(simgrid::s4u::NetZone* platform) final override;
    virtual void onJobEnd(Job* job) final override;
    virtual void onSimulationEnd() final override;

private:
    std::unique_ptr<SIMPLE_DISPATCHER> sd = std::make_unique<SIMPLE_DISPATCHER>();
};

SimpleDispatcherPlugin::SimpleDispatcherPlugin()
{
    // LOG_INFO("Loading the Job Dispatcher from Simple Dispatcher Plugin ....");
}

void SimpleDispatcherPlugin::getResourceInformation(simgrid::s4u::NetZone* platform)
{
    LOG_INFO("Inside the Resource information");
    
    sd->setPlatform(platform);
    
    LOG_INFO("Finished getting the Resource information");
}

sg4::NetZone* SimpleDispatcherPlugin::getPlatform()
{
    return sd->getPlatform();
}

Job* SimpleDispatcherPlugin::assignJob(Job* job)
{
    LOG_DEBUG("Inside the assign job: {}", job->comp_site);
    return sd->assignJobToResource(job);
}

void SimpleDispatcherPlugin::onJobEnd(Job* job)
{
    sd->free(job);
}

void SimpleDispatcherPlugin::onSimulationEnd()
{
    sd->cleanup();
}

extern "C" SimpleDispatcherPlugin* createSimpleDispatcherPlugin()
{
    return new SimpleDispatcherPlugin;
}