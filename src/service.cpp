#include "service.h"

#include <iostream>

Service::Service() {
    std::cout << "Service created" << std::endl;
}

Service::~Service() {
    std::cout << "Service destroyed" << std::endl;
}
