#include "cross.hpp"
#include "sys.hpp"
#include <sstream>
#include <thread>

#ifdef NNT_WINDOWS

#include <Windows.h>

#endif

#ifdef NNT_UNIXLIKE

#include <sys/types.h>
#include <unistd.h>

#endif

CROSS_BEGIN

#ifdef NNT_WINDOWS

pid_t get_process_id()
{
    return ::GetCurrentThreadId();
}

#endif

#ifdef NNT_UNIXLIKE

pid_t get_process_id()
{
    return ::getpid();
}

#endif

tid_t get_thread_id()
{
    ::std::ostringstream oss;
    oss << ::std::this_thread::get_id();
    return ::std::stoull(oss.str());
}

CROSS_END