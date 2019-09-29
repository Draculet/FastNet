#ifndef __BASE_GETTID_H__
#define __BASE_GETTID_H__

#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

namespace base
{
	pid_t gettid();
}

#endif