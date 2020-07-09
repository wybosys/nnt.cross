#import "test.h"
#import <sstream>

#import <cross/cross.hpp>
#import <cross/logger.hpp>
#import <cross/str.hpp>
#import <cross/threads.hpp>
#import <cross/timer.hpp>
#import <cross/json.hpp>
#import <cross/xml.hpp>
#import <cross/sys.hpp>
#import <cross/fs.hpp>
#import <cross/url.hpp>
#import <cross/digest.hpp>
#import <cross/zip.hpp>
#import <cross/connector_curl.hpp>
#import <cross/connector_objc.hpp>

#if TARGET_OS_MACOS
#define HTTPCONNECTOR ObjcHttpConnector
#define DOWNLOADCONNECTOR ObjcDownloadConnector
//#define HTTPCONNECTOR CurlHttpConnector
//#define DOWNLOADCONNECTOR CurlDownloadConnector
#else
#define HTTPCONNECTOR ObjcHttpConnector
#define DOWNLOADCONNECTOR ObjcDownloadConnector
#endif

USE_NNT;
USE_CROSS;

#define UNITTEST_CHECK_EQUAL(a, b) assert(a == b)

void test_sys()
{
    string str = " a b c ";
    UNITTEST_CHECK_EQUAL(replace(str, " ", "/"), "/a/b/c/");
    
    cout << "当前线程号 " << get_thread_id() << endl;
    cout << "uuid: " << uuid() << endl;
}

void test_test()
{
    ::std::stringstream ss;
    ss << "abc";
    UNITTEST_CHECK_EQUAL(ss.str().length(), 3);
    string val;
    ss >> val;
    UNITTEST_CHECK_EQUAL(val, "abc");
    UNITTEST_CHECK_EQUAL(ss.str().length(), 3);
    
    val = tostr(100012345.0);
    val = tostr(100012345.12345);
    val = tostr(100012345);
}

void test_fs()
{
    string dir = "xxx/abc/cde/efg";
    mkdirs(dir);
    rmtree("xxx");
    
    UNITTEST_CHECK_EQUAL(Time::Current(), (timestamp_t)Time::Now());
    
    Logger t;
    t.warn("hahaha");
    t.info(pwd());
    
    UNITTEST_CHECK_EQUAL(dirname(""), "");
    UNITTEST_CHECK_EQUAL(dirname(dirname(dir)), normalize("xxx/abc"));
    UNITTEST_CHECK_EQUAL(isabsolute(dir), false);
    UNITTEST_CHECK_EQUAL(isabsolute("/"), true);
    
#ifdef NNT_WINDOWS
    UNITTEST_CHECK_EQUAL(isabsolute("C://"), true);
#endif
    
    string str = "jfklsajfdlsajflajfdksjfkldsf";
    string nstr;
    file_put_contents("xx", str);
    file_get_contents("xx", nstr);
    UNITTEST_CHECK_EQUAL(str, nstr);
}

void test_url()
{
    string str = "http://www.baidu.com:80/abc/cde?abc=123&cde=123";
    Url u(str);
    UNITTEST_CHECK_EQUAL(u.toString(), str);
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

void test_prop()
{
    string str = "{\"b\":false,\"nil\":null,\"s\":\"string\"}";
    auto v = json_decode(str);
    auto p = toproperty(*v);
    v = tojsonobj(*p);
    string astr = json_encode(*v);
    UNITTEST_CHECK_EQUAL(str, astr);
    
    auto xo = toxmlobj(*p);
    astr = xml_encode(*xo);
    xo = xml_decode(astr);
    p = toproperty(*xo);
    v = tojsonobj(*p);
    astr = json_encode(*v);
    UNITTEST_CHECK_EQUAL(str, astr);
    
    str = "[false,null,\"string\"]";
    v = json_decode(str);
    p = toproperty(*v);
    v = tojsonobj(*p);
    astr = json_encode(*v);
    UNITTEST_CHECK_EQUAL(str, astr);
    
    xo = toxmlobj(*p);
    astr = xml_encode(*xo);
    xo = xml_decode(astr);
    p = toproperty(*xo);
    v = tojsonobj(*p);
    astr = json_encode(*v);
    UNITTEST_CHECK_EQUAL(str, astr);
}

void test_time()
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

void test_md5()
{
    UNITTEST_CHECK_EQUAL(md5str("hello"), "5d41402abc4b2a76b9719d911017c592");
}

void test_zip()
{
    string cur = absolute(dirname(__FILE__) + "/..");
    
    mkdir("test-file");
    mkdir("test-dir");
    
    bool suc = unzip(cur + "/test/test-file.zip", "test-file");
    UNITTEST_CHECK_EQUAL(suc, true);
    suc = unzip(cur + "/test/test-dir.zip", "test-dir");
    UNITTEST_CHECK_EQUAL(suc, true);
}

void test_rest()
{
    HTTPCONNECTOR cnt;
    cnt.url = "https://cn.bing.com/search";
    //cnt.method = HttpConnector::Method::GET;
    cnt.method = HttpConnector::Method::POST;
    cnt.setarg("q", "abc");
    UNITTEST_CHECK_EQUAL(cnt.send(), true);
    cout << cnt.body().str() << endl;
    for (auto& e : cnt.respheaders())
    {
        cout << e.first << ":" << e.second << endl;
    }
}

NNT_HEAP_OBJECT_EXPRESS(FixedTaskDispatcher, dis_download, {
    self.start();
});

void test_download()
{
    dis_download.start();
    
    // 阻塞模式
    DOWNLOADCONNECTOR cnt;
    cnt.url = "http://wybosys.com/github/datasets/icons/sample-0.zip";
    cnt.target = "sample-0.zip";
    if (cnt.send())
    {
        size_t sz = stat(cnt.target)->size;
        UNITTEST_CHECK_EQUAL(sz, 190567);
    }
    else
    {
        UNITTEST_CHECK_EQUAL(false, true);
    }
    
    // 非阻塞模式
    dis_download.add([=](ITask&)
                     {
        DOWNLOADCONNECTOR cnt;
        cnt.url = "http://wybosys.com/github/datasets/icons/sample-0.zip";
        cnt.target = "sample-0.zip";
        if (cnt.send())
        {
            size_t sz = stat(cnt.target)->size;
            UNITTEST_CHECK_EQUAL(sz, 190567);
        }
        else
        {
            UNITTEST_CHECK_EQUAL(false, true);
        }
    });
}

void test_task()
{
    // 测试单线程任务池
    // SingleTaskDispatcher dis;
    
    // 测试定长线程任务池
    // FixedTaskDispatcher dis;
    
    // 测试可变线程任务池
    QueuedTaskDispatcher dis;
    
    // dis.attach();
    
    ::std::atomic<int> count(0);
    
    for (int i = 0; i < 100; ++i)
    {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask&)
                                                 {
            cout << ++count << endl;
            Time::Sleep(1);
        }));
    }
    dis.start();
    for (int i = 0; i < 100; ++i)
    {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask&)
                                                 {
            cout << ++count << endl;
            Time::Sleep(1);
        }));
    }
    dis.wait();
}

FixedTaskDispatcher dis;

::std::atomic<int> async_count(9000);

void test_async_task()
{
    // 测试异步线程
    dis.start();
    
    for (int i = 0; i < 100; ++i)
    {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask&)
                                                 {
            if (IsMainThread())
            {
                cout << "主线程: " << ++async_count << endl;
            }
            else
            {
                cout << "子线程: " << ++async_count << endl;
            }
        }));
    }
    
    for (int i = 0; i < 100; ++i)
    {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask&)
                                                 {
            MainThread::shared().invoke([&]()
                                        {
                if (IsMainThread())
                {
                    cout << "主线程: "
                    << ++async_count << endl;
                }
                else
                {
                    cout << "子线程: "
                    << ++async_count << endl;
                }
            });
        }));
    }
}

void Test()
{
    test_sys();
    test_test();
    test_fs();
    test_url();
    test_prop();
    test_thread();
    test_time();
    test_md5();
    test_zip();
    test_rest();
    test_download();
    test_task();
    test_async_task();
    
    MainThread::shared().exec();
}
