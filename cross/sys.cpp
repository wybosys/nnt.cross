#include "cross.hpp"
#include "sys.hpp"
#include <sstream>
#include <thread>
#include "logger.hpp"

#ifdef NNT_WINDOWS
#include <Windows.h>
#include <objbase.h>
#endif

#ifdef NNT_UNIXLIKE
#include <sys/types.h>
#include <unistd.h>
#ifndef NNT_ANDROID
#include <uuid/uuid.h>
#endif
#endif

#ifdef NNT_ANDROID
#include <sys/prctl.h>
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
    if (tls_thread_name == name)
        return;

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
        // pass
    }
}

string uuid()
{
    GUID t;
    ::CoCreateGuid(&t);

    char result[33] = { 0 };
#ifdef __GNUC__
    snprintf(
#else // MSVC
    _snprintf_s(
#endif
        result,
        33,
        "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
        t.Data1, t.Data2, t.Data3,
        t.Data4[0], t.Data4[1],
        t.Data4[2], t.Data4[3],
        t.Data4[4], t.Data4[5],
        t.Data4[6], t.Data4[7]);
    return result;
}

#endif

#if defined(NNT_UNIXLIKE) && !defined(NNT_DARWIN)

pid_t get_process_id()
{
    return ::getpid();
}

static string tls_thread_name;

string get_thread_name()
{
    if (tls_thread_name.empty())
    {
        auto tid = ::std::this_thread::get_id();
        auto na = (pthread_t*)(&tid);
        char buf[256];
#ifdef NNT_ANDROID
        if (0 == prctl(PR_GET_NAME, buf, 256))
#else
            if (0 == pthread_getname_np(*na, buf, 256))
#endif
        {
            tls_thread_name = buf;
        }
        else
        {
            Logger::Warn("获得线程名称失败");
        }
    }
    return tls_thread_name;
}

// 设置线程名称
void set_thread_name(string const& name)
{
    if (tls_thread_name == name)
        return;

    if (name.length() > 16)
    {
        Logger::Warn("设置线程名称长度超过限制(16) " + name);
        return;
    }

    auto tid = ::std::this_thread::get_id();
    auto na = (pthread_t*)(&tid);
    tls_thread_name = name;
    auto ret = pthread_setname_np(*na, tls_thread_name.c_str());
    if (ret)
        Logger::Warn("设置线程名称失败");
}

#endif

#if defined(NNT_UNIXLIKE) && !defined(NNT_ANDROID)

string uuid()
{
    uuid_t t;
    uuid_generate(t);

    char result[33] = { 0 };
    for (size_t i = 0; i < 16; i++)
    {
#ifdef NNT_WINDOWS
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

        // 不可能越界，所以关闭编译器的警告
        sprintf(result + 2 * i, "%02x", t[i]);

#ifdef NNT_WINDOWS
#pragma warning(pop)
#endif
    }

    return result;
}

#endif

tid_t get_thread_id()
{
    ::std::ostringstream oss;
    oss << ::std::this_thread::get_id();
    return ::std::stoull(oss.str());
}

CROSS_END
