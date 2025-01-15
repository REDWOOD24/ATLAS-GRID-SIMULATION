// ==============================================
// Author: Raees Khan
// Email: rak177@pitt.edu
// Created Date: 2024-08-15
// Description: Class to load plugins.
// ==============================================


#include <iostream>
#include <dlfcn.h>
#include <libgen.h>
#include <string>

template <class Plugin> class PluginLoader
{
public:

    // Constructor:
    PluginLoader();

    // Destructor:
    ~PluginLoader();

    // load plugin
    Plugin *load(const std::string & path) const;

private:

    PluginLoader            (const PluginLoader &)=delete;
    PluginLoader & operator=(const PluginLoader &)=delete;

};



template <class Plugin>
PluginLoader<Plugin>::PluginLoader () {
}


template <class Plugin>
PluginLoader<Plugin>::~PluginLoader () {
}

template <class Plugin>
Plugin *PluginLoader<Plugin>::load(const std::string & pString) const {
    std::string bNameString=basename ((char *) pString.c_str());  // Strip off the directory
    bNameString=bNameString.substr(3);                            // Strip off leading "lib"
    bNameString=bNameString.substr(0,bNameString.find("."));      // Strip off extensions

    std::string createFunctionName = std::string("create")+bNameString;

    //
    // Loads the library:
    //
    void *handle = dlopen(pString.c_str(),RTLD_NOW);
    if (!handle) {
        std::cerr << dlerror() << std::endl;
        return nullptr;
    }

    //
    // Gets the function
    //
    void *f = dlsym(handle,createFunctionName.c_str());
    if (!f) {
        std::cerr << dlerror() << std::endl;
        return nullptr;
    }

    typedef void * (*CreationMethod) ();
    CreationMethod F = (CreationMethod) f;
    Plugin * factory = (Plugin *) F();
    return factory;

}

