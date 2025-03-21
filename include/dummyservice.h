#ifndef DUMMYSERVICE_H
#define DUMMYSERVICE_H

#include "service.h"

class DummyService : public Service {
public:
    static inline const int priority = 9999;

    static DummyService* init(int, char*[]);

    ~DummyService();
private:

    static DummyService* instance;
    DummyService(int, char*[]);

};

#endif // DUMMYSERVICE_H
