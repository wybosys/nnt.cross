#include <ajni++/ajni++.hpp>
#include <cross/cross.hpp>
#include <cross/logger.hpp>
#include <cross/threads.hpp>
#include <cross/timer.hpp>

AJNI_IMP_LOADED({})
AJNI_IMP_UNLOADED({})

#define CROSS_FUNC(cls, name) AJNI_FUNCNAME(com_nnt_cross, cls, name)

USE_CROSS;

AJNI_API(void) CROSS_FUNC(MainActivity, test)(JNIEnv* env, jobject thiz)
{
    Logger::shared().prefix = "CROSS-AJNI";
}

AJNI_API(void) CROSS_FUNC(MainActivity, test_1thread)(JNIEnv* env, jobject thiz)
{
    Thread thd("测试");
    semaphore wait;
    thd.proc = [&](Thread&)
    {
        Logger::Info("测试线程");
        wait.notify();
    };
    thd.start();
    wait.wait();
}

AJNI_API(void) CROSS_FUNC(MainActivity, test_1time)(JNIEnv* env, jobject thiz)
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
