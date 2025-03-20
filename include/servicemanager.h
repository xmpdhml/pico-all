#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <unordered_map>
#include <string>

class Service;
class ServiceCreationContext;

class ServiceManager {
public:
    // Get the singleton instance
    static ServiceManager& instance();

    Service* initService(const std::string& name, const ServiceCreationContext* context);
    Service* getService(const std::string& name);

    bool registerService(const std::string& name, Service* (*createFunc)(const ServiceCreationContext*));

    ServiceManager(const ServiceManager&) = delete;
    ServiceManager& operator=(const ServiceManager&) = delete;
    ServiceManager(ServiceManager&&) = delete;
    ServiceManager& operator=(ServiceManager&&) = delete;

private:
    std::unordered_map<std::string, Service* (*)(const ServiceCreationContext*)> creators;
    std::unordered_map<std::string, Service*> services;

    ServiceManager() {}
    ~ServiceManager() {}
};

#endif // SERVICEMANAGER_H