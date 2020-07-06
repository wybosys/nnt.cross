#ifndef __CROSS_TIMER_H_INCLUDED
#define __CROSS_TIMER_H_INCLUDED

#include <functional>

CROSS_BEGIN

NNT_CLASS_PREPARE(TimeCounter);

// 计时器
class TimeCounter
{
NNT_CLASS_DECL(TimeCounter);

public:

	TimeCounter();

	~TimeCounter();

	// 启动定时
	void start();

	// 过去的秒
	double seconds(bool reset = true);
};

NNT_CLASS_PREPARE(CoTimers);

// 最小粒度定时器
class CoTimers
{
NNT_CLASS_DECL(CoTimers);

public:

	enum
	{
		INFINITE = -1
	};

	// interval为基于秒定义的最小粒度间隔，当前实现的最小粒度为 0.001 既无法使用小于1毫秒的定时器
	explicit CoTimers(double interval);

	~CoTimers() noexcept;

	typedef ::std::function<void()> tick_t;
	typedef unsigned int timer_t;

	// 添加一个定时器, 秒间隔，重试次数, 回调
	timer_t add(double interval, int repeat, tick_t&& cb);

	// 取消定时器
	void cancel(timer_t);

	// 清空所有定时器
	void clear();

	// 开始计时
	void start();

	// 结束计时
	void stop();

NNT_SINGLETON_DECL(CoTimers);
};

// 定时器
class Timer
{
public:

	typedef CoTimers::tick_t tick_t;
	typedef CoTimers::timer_t timer_t;

	// 延迟调用
	static timer_t SetTimeout(double time, tick_t&&);

	// 取消延迟调用
	static void CancelTimeout(timer_t);

	// 按时间重复
	static timer_t SetInterval(double time, tick_t&&);

	// 取消按时间重复
	static void CancelInterval(timer_t);
};

CROSS_END

#endif