#include "cross.hpp"
#include "datetime.hpp"
#include <ctime>
#include <thread>
#include <chrono>

#ifdef NNT_WINDOWS

#include <Windows.h>
#include <WinUser.h>

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

static void usleep(long usec)
{
    HANDLE timer;
    LARGE_INTEGER interval;
    interval.QuadPart = -(10 * usec);

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &interval, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}

#endif

#ifdef NNT_UNIXLIKE

timestamp_t Time::Current()
{
	return time(nullptr);
}

seconds_t Time::Now()
{
	auto ms =
		::std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
			.count();
	return double(ms * 1e-3);
}

#endif

millseconds_t Time::CurrentMs()
{
	return (millseconds_t)::std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
		.count();
}

void Time::Sleep(seconds_t sec)
{
	if (sec <= 0)
		return;
	::std::this_thread::sleep_for(::std::chrono::milliseconds((long)(sec * 1e3)));
}

void Time::SleepMs(millseconds_t msec)
{
    if (msec <= 0)
        return;
    ::std::this_thread::sleep_for(::std::chrono::milliseconds(msec));
}

CROSS_END
