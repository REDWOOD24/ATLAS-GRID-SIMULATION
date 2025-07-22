# Plugins

Plugins can be used to define custom workload allocation algorithms in the simulator to test their performance and optimize them by collecting output statistics. 

## Plugin Support

`SimATLAS` provides the user support with writing plugins by providing the abstract class which one has to override. This abstract class serves as a link to interface custom user designed work allocation algorithms to the simulator.

### CMake Support

A header files containing the relevant abstract class is automatically installed with SimATLAS installation. To use this in your project you can simply use the find_package command in your CMakeLists.txt

```cmake
find_package(SimATLAS)
```

Then one can simply link as
```cmake
target_include_directories( YourPlugin PUBLIC ${SimATLAS_INCLUDE_DIR})
```

### Abstract Class
<!---->
The header file which contains the abstract class to help create plugins is `DispatcherPlugin.h`.

```c++
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
```

It contains two purely virtual methods which the user must implement and two which are optional. One of the methods the user ***must*** implement is

```c++
    virtual void getResourceInformation(simgrid::s4u::NetZone* platform);
```

This method provides the user with a SimGrid platform which contains information about the resources available on the grid. Using this one can incorporate resource information in their custom allocation algorithm. The second virtual method the user ***must*** implement is

```c++
   virtual Job* assignJob(Job* job);
```

This method provides the user with access to a Job which contains information about the workload, i.e. number of FLOPs to be executed, number of files to be read or written, etc. A job is defined in the simulator as shown below

```c++
struct Job {
    int                                         _id{};
    std::string                                  id{};
    int                                          flops{};
    std::unordered_map<std::string, size_t>     input_files{};
    std::unordered_map<std::string, size_t>     output_files{};
    size_t                                      input_storage{};
    size_t                                      output_storage{};
    int                                         priority{};
    int                                         cores{};
    std::string                                 mount{};
    std::string                                 disk{};
    std::string                                 comp_host{};
    std::string                                 comp_site{};
    bool operator<(const Job& other) const {if(priority == other.priority){return _id > other._id;} return priority < other.priority;}
};
```
The goal of any workload allocation algorithm is to take information about the resources and workload and assign the Job a comp_site, comp_host, disk. This is then the output of the method which goes out for execution in SimGrid.

The final two optional methods the user can implement are

```c++
   virtual void onJobEnd();
```
This will be called on the Job end.

```c++
   virtual void onSimulationEnd();
```
This will be called on the simulation end.

## Writing Plugins

As an example `SimATLAS` comes with a simple dispatcher plugin called `SimpleDispatcherPlugin` which uses a simple algorithm to assign jobs to resources. This example serves as a blueprint to write further more complicated algorithms. Before proceeding let us set up our project. Navigating to the directory of choice we can run on the terminal

```bash
mkdir simple-test-plugin
cd simple-test-plugin
touch CMakeLists.txt
mkdir build
mkdir src
mkdir include
mkdir CMakeModules
cd src
touch SimpleDispatcherPlugin.cpp
touch simple_dispatcher.cpp
cd ../include
touch simple_dispatcher.h 
```
In the `CMakeModules` directory the `FindSimGrid.cmake` `FindFSMod.cmake` files should be copied. These files can be found in the plugin example in the `SimATLAS` project. [Click Here!](https://github.com/REDWOOD24/ATLAS-GRID-SIMULATION/tree/development/dispatch_plugins/simple-test-plugin/CMakeModules) The directory structure should then look like

```bash
simple-test-plugin
├── CMakeLists.txt
├── CMakeModules
│   ├── FindFSMod.cmake
│   └── FindSimGrid.cmake
├── build
├── include
│   └── simple_dispatcher.h
└── src
    ├── SimpleDispatcherPlugin.cpp
    └── simple_dispatcher.cpp
```

Opening up the CMakeLists.txt file, we call our project SimpleDispatcherPlugin and configure the file as follows:


```cmake
# Set up the project.
cmake_minimum_required( VERSION 3.2 )
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project( "SimpleDispatcherPlugin" )

# Disable annoying warnings
add_definitions("-DBOOST_ALLOW_DEPRECATED_HEADERS")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

# Find and set the source file.
include_directories(include/)
file(GLOB SOURCES src/*.cpp )

# Set up the library.
add_library(SimpleDispatcherPlugin SHARED ${SOURCES})

# Find SimGrid, SimATLAS, Boost, and FSMOD
find_package(SimGrid REQUIRED)
find_package(FSMOD REQUIRED)
find_package(Boost REQUIRED)
find_package(SimATLAS REQUIRED)

#Link libraries and Include Directories
target_link_libraries ( SimpleDispatcherPlugin PUBLIC ${SimGrid_LIBRARY} ${Boost_LIBRARIES} ${FSMOD_LIBRARY})
target_include_directories( SimpleDispatcherPlugin PUBLIC ${SimGrid_INCLUDE_DIR} ${SimATLAS_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${FSMOD_INCLUDE_DIR})
```

Now we are ready to write our Plugin. This will require at least two classes and one additional function at the end of the `SimpleDispatcherPlugin.cpp` file.

```c++
class SIMPLE_DISPATCHER;
class SimpleDispatcherPlugin;
extern "C" SimpleDispatcherPlugin* createSimpleDispatcherPlugin()
```

- The first class `SIMPLE_DISPATCHER` is the actual implementation class which is defined in the `simple_dispatcher.h` and `simple_dispatcher.cpp` files, This class contains the custom workload allocation algorithm (see the one written in the code repo as a reference) [Click Here!](https://github.com/REDWOOD24/ATLAS-GRID-SIMULATION/blob/development/dispatch_plugins/simple-test-plugin/include/simple_dispatcher.h)

- The second class `SimpleDispatcherPlugin` is necessary for defining the plugin and ***must have the same name as the name specified within the add_library command in the CMakeLists.txt file.*** This class will inherit from the `DispatcherPlugin` class, which is the abstract class provided by SimATLAS to allow it to interface with the simulator. This class must always be defined, and override the purely virtual functions in the abstract class.

- Finally the function `createSimpleDispatcherPlugin` is neccesary for properly loading in the plugin to the simulator and ***must always have the name, create + name of plugin class.*** This function must always be defined.

The SimpleDispatcherPlugin class and createSimpleDispatcherPlugin function are mostly boiler plate code and are defined in the `SimpleDispatcherPlugin.cpp` file. Being mostly boiler plate they can be taken from the example in the code shown below with minimal adjustments

```c++
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
```

The virtual functions are overrided with custom functionality from the implementation class. With all this completed, we can now move into the build folder and run.

```bash
cmake ..
make
```

In the build folder you will see:

```bash
libSimpleDispatcherPlugin.dylib (Mac)
libSimpleDispatcherPlugin.so (Linux)
```

The path to these shared libraries can then be specified in the simulator configuration and the custom allocation algorithm can be tested!



