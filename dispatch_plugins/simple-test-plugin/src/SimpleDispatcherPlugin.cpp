#include "DispatcherPlugin.h"
#include "simple_dispatcher.h"


class SimpleDispatcherPlugin:public DispatcherPlugin {

public:
    SimpleDispatcherPlugin();
    virtual Job* assignJob(Job* job) final override;
    virtual void assignResources(simgrid::s4u::NetZone* platform) final override;
    virtual void onSimulationEnd() final override;

private:
    std::unique_ptr<SIMPLE_DISPATCHER> sd = std::make_unique<SIMPLE_DISPATCHER>();

};

SimpleDispatcherPlugin::SimpleDispatcherPlugin()
{
  std::cout << "Setting the Job Dispatcher as Simple Dispatcher" << std::endl;
}

void SimpleDispatcherPlugin::assignResources(simgrid::s4u::NetZone* platform)
{
  sd->setPlatform(platform);
}

Job* SimpleDispatcherPlugin::assignJob(Job* job)
{
  return sd->assignJobToResource(job);
}

void SimpleDispatcherPlugin::onSimulationEnd()
{
  sd->cleanup();
}

extern "C" SimpleDispatcherPlugin* createSimpleDispatcherPlugin()
{
    return new SimpleDispatcherPlugin;
}

