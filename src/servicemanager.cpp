#include "servicemanager.h"

ServiceManager& ServiceManager::instance() {
    static ServiceManager instance;
    return instance;
}

Service *ServiceManager::initService(const std::string &name, const ServiceCreationContext *context)
{
    auto it = creators.find(name);
    if (it == creators.end())
    {
        return nullptr;
    }
    Service *service = it->second(context);
    if (service)
    {
        services.insert({name, service});
    }
    return service;
}

Service *ServiceManager::getService(const std::string &name)
{
    auto it = services.find(name);
    if (it == services.end())
    {
        return nullptr;
    }
    return it->second;
}

bool ServiceManager::registerService(const std::string &name, Service *(*createFunc)(const ServiceCreationContext *))
{
    return creators.insert({name, createFunc}).second;
}
