#include "system.h"

System& internal_create()
{
    static System instance;
    return instance;
}

System& system = internal_create();

System::System()
    : io(*new UartDMAStdio()),
      keys(*new KeyScan())
{
    // Constructor implementation
}

