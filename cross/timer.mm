#import "cross.hpp"
#import "timer.hpp"
#include "logger.hpp"
#include "threads.hpp"
#include <map>
#include <atomic>

CROSS_BEGIN

class NSTimerThread
: public Thread
{
public:
    
    NSTimerThread()
    : Thread("n2c.timer"),
    _delays(make_shared<delay_timers>())
    {
        repeat = FOREVER;
    }
    
    NNT_SINGLETON_DECL(NSTimerThread);
  
    ::std::atomic<Timer::timer_t> idr;
    ::std::map<Timer::timer_t, NSTimer*> timers;
    NSRunLoop *loop = nullptr;
    ::std::mutex mtx;
    
    void run(NSTimer *tmr)
    {
        if (loop) {
            [loop addTimer:tmr forMode:NSDefaultRunLoopMode];
        } else {
            _delays->emplace_back(tmr);
        }
    }
    
    virtual void main() override
    {
        if (!loop)
        {
            NNT_AUTOGUARD(mtx);
            loop = [NSRunLoop currentRunLoop];
            
            for (auto &e:*_delays) {
                [loop addTimer:e forMode:NSDefaultRunLoopMode];
            }
            _delays = nullptr;
        }
        
        [loop runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
    }
    
private:
    
    typedef ::std::vector<NSTimer*> delay_timers;
    shared_ptr<delay_timers> _delays; // 定时器线程启动成功前加入的timer
    
};

NNT_SINGLETON_IMPL(NSTimerThread);

void NSTimerThread::_shared_init()
{
    start();
}

static Timer::timer_t SetTimer(double time, Timer::tick_t&& tick, bool repeat)
{
    auto &thd = NSTimerThread::shared();
    
    auto tmrid = thd.idr++;
    auto cb = ::std::forward<Timer::tick_t>(tick); // 复制一份，oc的block不会像cxx的lambda抛出编译错误
    auto tmr = [NSTimer timerWithTimeInterval:time repeats:repeat block:^(NSTimer * _Nonnull timer) {
        if (!repeat)
        {
            NNT_AUTOGUARD(thd.mtx);
            thd.timers.erase(tmrid);
        }
        
        cb();
    }];
    
    NNT_AUTOGUARD(thd.mtx);
    thd.timers[tmrid] = tmr;
    thd.run(tmr);
    
    return tmrid;
}

Timer::timer_t Timer::SetTimeout(double time, tick_t&& tick)
{
    return SetTimer(time, ::std::forward<tick_t>(tick), false);
}

void Timer::CancelTimeout(timer_t tmrid)
{
    auto &thd = NSTimerThread::shared();

    NNT_AUTOGUARD(thd.mtx);
    
    auto fnd = thd.timers.find(tmrid);
    if (fnd == thd.timers.end()) {
        Logger::Warn("没有找到定时器，定时器已经结束或者传入的定时器id错误");
        return;
    }
    
    auto tmr = fnd->second;
    [tmr invalidate];
    thd.timers.erase(fnd);
}

Timer::timer_t Timer::SetInterval(double time, tick_t&& tick)
{
    return SetTimer(time, ::std::forward<tick_t>(tick), true);
}

void Timer::CancelInterval(timer_t tmrid)
{
    CancelTimeout(tmrid);
}

CROSS_END
