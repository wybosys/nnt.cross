#include "cross.h"
#include "datetime.h"
#include <time.h>
#include <thread>
#include <chrono>  

#ifdef NNT_WINDOWS

#include <Windows.h>

#endif

CROSS_BEGIN

#ifdef NNT_WINDOWS

timestamp_t Time::Current()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    tm t;
    t.tm_sec = st.wSecond;
    t.tm_min = st.wMinute;
    t.tm_hour = st.wHour;
    t.tm_mday = st.wDay;
    t.tm_mon = st.wMonth - 1;
    t.tm_year = st.wYear - 1900;
    t.tm_isdst = -1;

    return mktime(&t);
}

seconds_t Time::Now()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    tm t;
    t.tm_sec = st.wSecond;
    t.tm_min = st.wMinute;
    t.tm_hour = st.wHour;
    t.tm_mday = st.wDay;
    t.tm_mon = st.wMonth - 1;
    t.tm_year = st.wYear - 1900;
    t.tm_isdst = -1;

    seconds_t r = static_cast<seconds_t>(mktime(&t));
    r += st.wMilliseconds * 1e-3;
    return r;
}

#endif

void Time::Sleep(seconds_t sec)
{
    this_thread::sleep_for(chrono::milliseconds((long)(sec * 1e3)));
}

CROSS_END