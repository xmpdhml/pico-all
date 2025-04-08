#include "system.h"

System& internal_create()
{
    static System instance;
    return instance;
}

System& sys = internal_create();

System::System()
    : io(io_instance), keys(keys_instance)
{
    // Constructor implementation
}

