#include "cross.hpp"
#include "timer.hpp"
#include <chrono>
#include <thread>
#include <map>
#include <mutex>
#include <atomic>
#include <list>
#include <cstring>

#include "datetime.hpp"
#include "logger.hpp"
#include "threads.hpp"
#include "sys.hpp"

#ifdef NNT_WINDOWS
#include <Windows.h>
#include <WinUser.h>
#endif

#if defined(NNT_UNIXLIKE) && !defined(NNT_DARWIN)
  #include <unistd.h>
  #include <sys/epoll.h>
  #include <sys/timerfd.h>
#endif

// 同时运行的最大定时器数量
#define MAX_TIMERS 256

// 使用cotimers实现Time
//#define TIME_IMPL_COTIMERS

// 使用系统api实现Time
#define TIME_IMPL_SYSTEM

CROSS_BEGIN

USE_STL

// ----------------------------------------------------- TimeCounter

class TimeCounterPrivate
{
public:
	chrono::high_resolution_clock::time_point t0;
};

TimeCounter::TimeCounter()
{
	NNT_CLASS_CONSTRUCT();
}

TimeCounter::~TimeCounter()
{
	NNT_CLASS_DESTROY();
}

void TimeCounter::start()
{
	d_ptr->t0 = chrono::high_resolution_clock::now();
}

double TimeCounter::seconds(bool reset)
{
	auto t1 = chrono::high_resolution_clock::now();
	auto dur = t1 - d_ptr->t0;

	if (reset)
	{
		d_ptr->t0 = t1;
	}

	return chrono::duration_cast<chrono::microseconds>(dur).count() * 0.000001;
}

// -------------------------------------------- CoTimers

struct CoTimerItem
{
	// 定时器id
	CoTimers::timer_t id = 0;

	// 当前分割计时的总时长
	millseconds_t full = 0;

	// 还剩下多少时长
	millseconds_t left = 0;

	// 重复次数
	int repeat = 0;

	// 业务回调
	CoTimers::tick_t proc;
};

class CoTimersPrivate
{
public:

	explicit CoTimersPrivate(CoTimers* o)
		: d_owner(o),
		  thd("n2c.cotimers")
	{
		thd.repeat = Thread::FOREVER;
		thd.proc = [=](Thread&)
		{
			auto d = d_owner->d_ptr; // 避免被释放
			// Logger::Debug("定时器Tick");

			// 等待
			Time::SleepMs(minterval - cost);

			// 计算时间消耗
			millseconds_t b = Time::CurrentMs();

			// tick所有分片
			d->tick();

			millseconds_t e = Time::CurrentMs();
			cost = e - b;
			if (cost >= 30)
			{
				Logger::Warn("定时器调用耗时过长");
			}
		};
	}

	CoTimers* d_owner;

	typedef map<CoTimers::timer_t, shared_ptr<CoTimerItem>> timers_type;

	void tick()
	{
		bool needclean = false;

		// 做一个快照避免业务层在timer-tick中增加新timer导致冲突
		timers_type snaps;
		{
			NNT_AUTOGUARD(mtx);
			snaps = timers;
		}

		// 遍历快照
		for (auto& e : snaps)
		{
			auto& item = *e.second;
			item.left -= minterval;
			if (item.left <= 0)
			{
				if (item.repeat != -1)
				{
					if (--item.repeat == 0)
						needclean = true;
				}

				// 调用业务层回调
				item.proc();
				item.left = item.full;
			}
		}

		// 移除失效的
		if (needclean)
		{
			NNT_AUTOGUARD(mtx);

			for (auto i = timers.begin(); i != timers.end();)
			{
				if (i->second->repeat <= 0)
				{
					i = timers.erase(i);
				}
				else
				{
					++i;
				}
			}
		}
	}

	millseconds_t minterval = 0; // 间隔粒度为毫秒
	millseconds_t cost = 0; // 执行每一组业务定时回调耗费的时间
	CoTimers::timer_t counter = 0; // 当前计数器
	timers_type timers; // 所有分片定时器
	mutex mtx; // 锁
	Thread thd; // 总计时器线程
};

CoTimers::CoTimers(double interval)
{
	NNT_CLASS_CONSTRUCT(this);
	d_ptr->minterval = (millseconds_t)(interval * 1e3);
}

CoTimers::~CoTimers() noexcept
{
	// 等待退出
	stop();

	NNT_CLASS_DESTROY();
}

CoTimers::timer_t CoTimers::add(double interval, int repeat, tick_t&& cb)
{
	NNT_AUTOGUARD(d_ptr->mtx);

	auto t = make_shared<CoTimerItem>();
	t->id = ++d_ptr->counter;
	t->full = (millseconds_t)(interval * 1e3);
	t->left = t->full;
	t->proc = cb;
	t->repeat = repeat;
	d_ptr->timers.insert(make_pair(t->id, t));

	return t->id;
}

void CoTimers::cancel(timer_t id)
{
	NNT_AUTOGUARD(d_ptr->mtx)
	d_ptr->timers.erase(id);
}

void CoTimers::clear()
{
	NNT_AUTOGUARD(d_ptr->mtx)
	d_ptr->timers.clear();
}

// 最小粒度定时器
NNT_SINGLETON_IMPL(CoTimers, 0.1);

void CoTimers::_shared_init()
{
	// shared对象自动开始计时
	start();
}

void CoTimers::start()
{
	d_ptr->thd.start();
	d_ptr->thd.wait();
}

void CoTimers::stop()
{
	d_ptr->thd.quit();
}

// -------------------------------------------------- Time

#ifdef TIME_IMPL_COTIMERS

Timer::timer_t Timer::SetTimeout(double time, tick_t&& cb)
{
	if (time <= 0) {
		Logger::Warn("定时器的时间必须 > 0");
		return 0;
	}

	return CoTimers::shared().add(time, 1, ::std::forward<tick_t>(cb));
}

void Timer::CancelTimeout(timer_t id)
{
	CoTimers::shared().cancel(id);
}

Timer::timer_t Timer::SetInterval(double time, tick_t&& cb)
{
	if (time <= 0) {
		Logger::Warn("定时器的时间必须 > 0");
		return 0;
	}

	return CoTimers::shared().add(time, -1, ::std::forward<tick_t>(cb));
}

void Timer::CancelInterval(timer_t id)
{
	CoTimers::shared().cancel(id);
}

#endif

#ifdef TIME_IMPL_SYSTEM

#ifdef NNT_WINDOWS

class TimerItem
{
public:
	Timer::tick_t tick;
	UINT id;
	HANDLE hdl;
};

// windows定时器id和业务回调的映射
static ::std::mutex gsmtx_timers;
static ::std::atomic<UINT> gsid_timers;
static ::std::map<UINT, ::std::shared_ptr<TimerItem> > gs_timers;

static void CALLBACK FnTimeout(PVOID parameter, BOOLEAN TimerOrWaitFired)
{
	set_thread_name("sys.timer");

	UINT id = ((TimerItem*)parameter)->id;
	::std::shared_ptr<TimerItem> tmr;

	// 查找定时器对应的业务回调
	{
		NNT_AUTOGUARD(gsmtx_timers);
		auto fnd = gs_timers.find(id);
		if (fnd == gs_timers.end()) {
			Logger::Critical("遇到一个不存在定时器");
			return;
		}
		tmr = fnd->second;
		gs_timers.erase(fnd);
	}

	// 执行回调
	tmr->tick();
}

Timer::timer_t Timer::SetTimeout(double time, tick_t&& cb)
{
	if (time <= 0) {
		Logger::Warn("定时器的时间必须 > 0");
		return 0;
	}

	NNT_AUTOGUARD(gsmtx_timers);

	auto tmr = make_shared<TimerItem>();
	tmr->id = ++gsid_timers;
	tmr->tick = cb;
	gs_timers[tmr->id] = tmr;

	auto s = ::CreateTimerQueueTimer(&tmr->hdl, NULL, FnTimeout, tmr.get(), (DWORD)(time * 1e3), 0, WT_EXECUTEINTIMERTHREAD);
	if (!s) {
		Logger::Critical("启动定时器失败");
		gs_timers.erase(tmr->id);
		return 0;
	}

	return tmr->id;
}

void Timer::CancelTimeout(timer_t tmr)
{
	auto id = (UINT)tmr;
	HANDLE hdl;

	// 清除回调映射
	{
		NNT_AUTOGUARD(gsmtx_timers);
		auto fnd = gs_timers.find(id);
		if (fnd == gs_timers.end()) {
			Logger::Debug("定时器已经执行完成或者不存在");
			return;
		}
		hdl = fnd->second->hdl;
		gs_timers.erase(fnd);
	}

	::DeleteTimerQueueTimer(NULL, hdl, NULL);
}

static void CALLBACK FnInterval(PVOID parameter, BOOLEAN TimerOrWaitFired)
{
	set_thread_name("sys.timer");

	UINT id = ((TimerItem*)parameter)->id;
	::std::shared_ptr<TimerItem> tmr;

	// 查找定时器对应的业务回调
	{
		NNT_AUTOGUARD(gsmtx_timers);
		auto fnd = gs_timers.find(id);
		if (fnd == gs_timers.end()) {
			Logger::Critical("遇到一个不存在定时器");
			return;
		}
		tmr = fnd->second;
	}

	// 执行回调
	tmr->tick();
}

Timer::timer_t Timer::SetInterval(double time, tick_t&& cb)
{
	if (time <= 0) {
		Logger::Warn("定时器的时间必须 > 0");
		return 0;
	}

	NNT_AUTOGUARD(gsmtx_timers);

	auto tmr = make_shared<TimerItem>();
	tmr->id = ++gsid_timers;
	tmr->tick = cb;
	gs_timers[tmr->id] = tmr;

	auto s = ::CreateTimerQueueTimer(&tmr->hdl, NULL, FnInterval, tmr.get(), (DWORD)(time * 1e3), (DWORD)(time * 1e3), WT_EXECUTEINTIMERTHREAD);
	if (!s) {
		Logger::Critical("启动定时器失败");
		gs_timers.erase(tmr->id);
		return 0;
	}

	return tmr->id;
}

void Timer::CancelInterval(timer_t tmr)
{
	auto id = (UINT)tmr;
	HANDLE hdl;

	// 清除回调映射
	{
		NNT_AUTOGUARD(gsmtx_timers);
		auto fnd = gs_timers.find(id);
		if (fnd == gs_timers.end()) {
			Logger::Debug("定时器已经执行完成或者不存在");
			return;
		}
		hdl = fnd->second->hdl;
		gs_timers.erase(fnd);
	}

	::DeleteTimerQueueTimer(NULL, hdl, NULL);
}

#endif

#if defined(NNT_UNIXLIKE) && !defined(NNT_DARWIN)

class TimerThread
	: public Thread
{
public:

NNT_SINGLETON_DECL(TimerThread);

	class TimerItem
	{
	public:
		uint32_t id;
		int hdl;
		bool interval = false;
		Timer::tick_t tick;
	};

	TimerThread()
		: Thread("n2c.timer")
	{
		repeat = FOREVER;
		fd_epoll = epoll_create(MAX_TIMERS);
		tmpevents.resize(MAX_TIMERS);
	}

	virtual void main() override
	{
		auto fired = epoll_wait(fd_epoll, &tmpevents.at(0), MAX_TIMERS, 1);
		if (fired <= 0)
			return;

		// 收集所有待激活timer
		::std::vector<shared_ptr<TimerItem> > ticks;
		{
			NNT_AUTOGUARD(mtx);
			for (auto i = 0; i < fired; ++i)
			{
				auto id = tmpevents[i].data.u32;
				auto& tmr = timers[id];

				// 读取状态
				uint64_t val;
				read(tmr->hdl, &val, sizeof(val));

				// 激活
				ticks.emplace_back(tmr);

				// 如果是interval类型，则继续
				if (tmr->interval)
				{
					++i;
					continue;
				}

				// 移除已经结束的timer
				epoll_ctl(fd_epoll, EPOLL_CTL_DEL, tmr->hdl, nullptr);

				// 删除系统资源
				close(tmr->hdl);

				// 还回去id
				i = timers.erase(i);
			}
		}

		// 激活所有可以激活的
		for (auto& e:ticks)
		{
			e->tick();
		}
	}

	int timeout(uint msec, Timer::tick_t&& tick)
	{
		NNT_AUTOGUARD(mtx);
		if (timers.size() == MAX_TIMERS)
		{
			Logger::Critical("当前持有的定时器超过系统最大定时器数量");
			return 0;
		}

		auto tmr = make_shared<TimerItem>();

		tmr->tick = tick;
		tmr->id = ids_timers++;
		timers[tmr->id] = tmr;

		itimerspec its = { 0 };
		its.it_value.tv_sec = msec / 1000;
		its.it_value.tv_nsec = (msec % 1000) * 1e6;
		tmr->hdl = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
		timerfd_settime(tmr->hdl, 0, &its, nullptr);

		epoll_event evt = { 0 };
		evt.events = EPOLLIN;
		evt.data.fd = tmr->hdl;
		evt.data.u32 = tmr->id;
		epoll_ctl(fd_epoll, EPOLL_CTL_ADD, tmr->hdl, &evt);

		return tmr->id;
	}

	int interval(uint msec, Timer::tick_t&& tick)
	{
		NNT_AUTOGUARD(mtx);
		if (timers.size() == MAX_TIMERS)
		{
			Logger::Critical("当前持有的定时器超过系统最大定时器数量");
			return 0;
		}

		auto tmr = make_shared<TimerItem>();

		tmr->interval = true;
		tmr->tick = tick;
		tmr->id = ids_timers++;
		timers[tmr->id] = tmr;

		itimerspec its = { 0 };
		its.it_value.tv_sec = its.it_interval.tv_sec = msec / 1000;
		its.it_value.tv_nsec = its.it_interval.tv_nsec = (msec % 1000) * 1e6;
		tmr->hdl = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
		timerfd_settime(tmr->hdl, 0, &its, nullptr);

		epoll_event evt = { 0 };
		evt.events = EPOLLIN;
		evt.data.fd = tmr->hdl;
		evt.data.u32 = tmr->id;
		epoll_ctl(fd_epoll, EPOLL_CTL_ADD, tmr->hdl, &evt);

		return tmr->id;
	}

	void cancel(int id)
	{
		NNT_AUTOGUARD(mtx);
		auto fnd = timers.find(id);
		if (fnd == timers.end())
		{
			Logger::Warn("没有找到定时器");
			return;
		}

		// 清除事件
		auto tmr = fnd->second;
		epoll_ctl(fd_epoll, EPOLL_CTL_DEL, tmr->hdl, nullptr);

		// 删除系统资源
		close(tmr->hdl);

		// 删除
		timers.erase(fnd);
	}

	// epoll池
	int fd_epoll;

	// 操作锁
	::std::mutex mtx;

	// 定时器id池
	::std::atomic<uint32_t> ids_timers;

	// timers映射
	::std::map<uint32_t, shared_ptr<TimerItem> > timers;

	// event数组
	::std::vector<epoll_event> tmpevents;
};

NNT_SINGLETON_IMPL(TimerThread);

void TimerThread::_shared_init()
{
	start();
}

Timer::timer_t Timer::SetTimeout(double time, tick_t&& cb)
{
	return TimerThread::shared().timeout((uint)(time * 1e3), ::std::forward<tick_t>(cb));
}

void Timer::CancelTimeout(timer_t tmr)
{
	TimerThread::shared().cancel((uint)tmr);
}

Timer::timer_t Timer::SetInterval(double time, tick_t&& cb)
{
	return TimerThread::shared().interval((uint)(time * 1e3), ::std::forward<tick_t>(cb));
}

void Timer::CancelInterval(timer_t tmr)
{
	TimerThread::shared().cancel((uint)tmr);
}

#endif

#endif

CROSS_END
