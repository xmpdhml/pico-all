#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <concepts>
#include <map>

#include "service.h"

class ServiceManager {
public:

    static ServiceManager& instance();

    template<typename T, typename... Args> 
    requires std::derived_from<T, Service>
    void initService(Args&&... args) {
        services.emplace(T::priority, T::create(std::forward<Args>(args)...));
    }

    ServiceManager& operator=(const ServiceManager&) = delete;
    ServiceManager(ServiceManager&&) = delete;
    ServiceManager& operator=(ServiceManager&&) = delete;

private:

    std::map<int, Service*> services;

    ServiceManager() {}
    ~ServiceManager() {}
};

#endif // SERVICEMANAGER_H