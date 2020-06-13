#include "cross.hpp"
#include "sys.hpp"

#ifdef NNT_WINDOWS

#include <Windows.h>

#endif

#ifdef NNT_UNIXLIKE

#include <sys/types.h>
#include <unistd.h>

#endif

CROSS_BEGIN

#ifdef NNT_WINDOWS

pid_t get_thread_id()
{
    return ::GetCurrentThreadId();
}

#endif

#ifdef NNT_UNIXLIKE

pid_t get_thread_id()
{
    return ::getpid();
}

#endif

CROSS_END