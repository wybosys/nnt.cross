#include <ajni++/ajni++.hpp>
#include <cross/cross.hpp>
#include <cross/logger.hpp>
#include <cross/threads.hpp>

AJNI_IMP_LOADED({})
AJNI_IMP_UNLOADED({})

#define CROSS_FUNC(cls, name) AJNI_FUNCNAME(com_nnt_cross, cls, name)

USE_CROSS;

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
