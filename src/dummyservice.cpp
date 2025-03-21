#include "dummyservice.h"

DummyService* DummyService::instance = nullptr;

DummyService* DummyService::init(int argc, char* argv[]) {
    if (instance == nullptr) {
        instance = new DummyService(argc, argv);
    }
    return instance;
}

DummyService::DummyService(int argc, char* argv[]) {
    // dummy
}

DummyService::~DummyService() {
    // dummy
}
