#ifndef SERVICE_H
#define SERVICE_H

#include <unordered_map>
#include <string>

class Service {
protected:
    Service() = default;
public:
    virtual ~Service() = default;
    template<typename T, typename ServiceCreationContext>
    static T* create(const ServiceCreationContext &context) {
        return nullptr;
    }
    
private:
    std::unordered_map<std::string, Service *> services;
};

#endif
