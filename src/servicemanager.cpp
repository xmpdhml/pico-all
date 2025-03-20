#include "servicemanager.h"

ServiceManager& ServiceManager::instance() {
    static ServiceManager instance;
    return instance;
}

