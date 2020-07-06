#include "cross.hpp"
#include "timer.hpp"
#include <chrono>
#include <thread>
#include <map>
#include <mutex>
#include <atomic>

#include "datetime.hpp"
#include "logger.hpp"
#include "threads.hpp"
#include "sys.hpp"

#ifdef NNT_WINDOWS
#include <Windows.h>
#include <WinUser.h>
#endif

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
	NNT_CLASS_DESTORY();
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

	NNT_CLASS_DESTORY();
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

//#define TIME_IMPL_COTIMERS
#define TIME_IMPL_SYSTEM

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

#endif

CROSS_END