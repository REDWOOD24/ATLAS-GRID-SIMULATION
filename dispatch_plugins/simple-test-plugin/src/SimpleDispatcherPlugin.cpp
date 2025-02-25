#include "DispatcherPlugin.h"
#include "simple_dispatcher.h"


class SimpleDispatcherPlugin:public DispatcherPlugin {

public:
    SimpleDispatcherPlugin();
    virtual Job* assignJob(Job* job) final override;
    virtual void getResourceInformation(simgrid::s4u::NetZone* platform) final override;
    virtual void onJobEnd(Job* job) final override;
    virtual void onSimulationEnd() final override;

private:
    std::unique_ptr<SIMPLE_DISPATCHER> sd = std::make_unique<SIMPLE_DISPATCHER>();

};

SimpleDispatcherPlugin::SimpleDispatcherPlugin()
{
  std::cout << "Loading the Job Dispatcher from Simple Dispatcher Plugin ...." << std::endl;
}

void SimpleDispatcherPlugin::getResourceInformation(simgrid::s4u::NetZone* platform)
{
  sd->setPlatform(platform);
}

Job* SimpleDispatcherPlugin::assignJob(Job* job)
{
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

