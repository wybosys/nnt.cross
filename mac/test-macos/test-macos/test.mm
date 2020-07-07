#import "test.h"

#import <cross/cross.hpp>
#import <cross/logger.hpp>
#import <cross/str.hpp>
#import <cross/threads.hpp>
#import <cross/timer.hpp>

USE_CROSS;

void test_sys()
{
    
}

void test_thread()
{
    Thread thd("测试");
    semaphore wait;
    thd.proc = [&](Thread&)
    {
        cout << "测试线程" << endl;
        wait.notify();
    };
    thd.start();
    wait.wait();
}

void test_timer()
{
    auto tmrs = new CoTimers(0.001);
    tmrs->add(0.2, 5, [=]()
              {
        static size_t count = 0;
        Logger::Info("定时器回调");
        if (++count == 5)
        {
            delete tmrs;
        }
    });
    tmrs->start();
    
    Timer::SetTimeout(1, []()
                      {
        Logger::Info("1s定时器");
    });
    
    auto tmr = make_shared<Timer::timer_t>();
    *tmr = Timer::SetInterval(2, [=]()
                              {
        Logger::Info("2s循环定时器");
        static size_t count = 5;
        if (--count == 0)
        {
            Logger::Info("取消2s循环定时器");
            Timer::CancelInterval(*tmr);
        }
    });
}

void Test()
{
    test_sys();
    test_thread();
    test_timer();
    
    MainThread::shared().exec();
}
