#ifndef __NNT_CROSS_DATETIME_H_INCLUDED
#define __NNT_CROSS_DATETIME_H_INCLUDED

CROSS_BEGIN

typedef unsigned long long timestamp_t;
typedef double seconds_t;
typedef long millseconds_t;

class NNT_API Time
{
public:

	// 当前时间戳(整数秒)
	static timestamp_t Current();

	// 当前时间戳(小数秒)
	static seconds_t Now();

	// 毫秒时间戳
	static millseconds_t CurrentMs();

	// 等待多少秒
	static void Sleep(seconds_t);

	// 等待多少毫秒
	static void SleepMs(millseconds_t);

};

CROSS_END

#endif