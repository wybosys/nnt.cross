#import "test.h"

#import <cross/cross.hpp>
#import <cross/str.hpp>
#import <cross/threads.hpp>

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

void Test()
{
    test_sys();
    test_thread();
}
