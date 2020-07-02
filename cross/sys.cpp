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
    return ::GetCurrentProcessId();
}

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static string tls_thread_name;

string get_thread_name()
{
    return tls_thread_name;
}

void set_thread_name(string const& name)
{
    tls_thread_name = name;
    auto tid = ::GetCurrentThreadId();

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = tls_thread_name.c_str();
    info.dwThreadID = tid;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
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