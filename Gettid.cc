#include "Gettid.h"

pid_t base::gettid()
{
    return syscall(SYS_gettid);
}