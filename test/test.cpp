#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporterStdout.h>

#include <cross.h>
#include <fs.h>
#include <datetime.h>
#include <logger.h>
#include <json.h>
#include <xml.h>
#include <url.h>
#include <threads.h>
#include <connector_curl.h>

USE_CROSS;

TEST (fs) {
    string dir = "xxx/abc/cde/efg";
    mkdirs(dir);
    rmtree("xxx");

    UNITTEST_CHECK_EQUAL(Time::Current(), (timestamp_t)Time::Now());

    Logger t;
    t.warn("hahaha");

    UNITTEST_CHECK_EQUAL(dirname(""), "");
    UNITTEST_CHECK_EQUAL(dirname(dirname(dir)), normalize("xxx/abc"));
    UNITTEST_CHECK_EQUAL(isabsolute(dir), false);
    UNITTEST_CHECK_EQUAL(isabsolute("/"), true);

#ifdef NNT_WINDOWS
    UNITTEST_CHECK_EQUAL(isabsolute("C://"), true);
#endif
}

TEST(url)
{
    string str = "http://www.baidu.com/abc/cde?abc=123&cde=123";
    Url u(str);
    UNITTEST_CHECK_EQUAL(u.toString(), str);
}

TEST(rest)
{
    CurlHttpConnector cnt;
    cnt.url = "https://cn.bing.com/search";
    cnt.setarg("q", "abc");
    UNITTEST_CHECK_EQUAL(cnt.send(), true);
    // cout << cnt.body().str() << endl;
    for (auto &e : cnt.respheaders()) {
        cout << e.first << ":" << e.second << endl;
    }
}

TEST(prop)
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

TEST(task)
{
    // 测试单线程任务池
    // SingleTaskDispatcher dis;

    // 测试定长线程任务池
    // FixedTaskDispatcher dis;

    // 测试可变线程任务池
    QueuedTaskDispatcher dis;

    // dis.attach();

    atomic<int> count = 0;

    for (int i = 0; i < 100; ++i) {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask*) {
            cout << ++count << endl;
            Time::Sleep(1);
            }));
    }
    dis.start();
    for (int i = 0; i < 100; ++i) {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask*) {
            cout << ++count << endl;
            Time::Sleep(1);
            }));
    }
    dis.wait();
}

FixedTaskDispatcher dis;
atomic<int> async_count = 9000;

TEST(async_task)
{
    // 测试异步线程
    dis.start();

    for (int i = 0; i < 100; ++i) {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask*) {
            if (IsMainThread()) {
                cout << "主线程: " << ++async_count << endl;
            }
            else {
                cout << "子线程: " << ++async_count << endl;
            }
            }));
    }

    for (int i = 0; i < 100; ++i) {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask*) {
            MainThread::shared().invoke([&]() {
                if (IsMainThread()) {
                    cout << "主线程: " << ++async_count << endl;
                }
                else {
                    cout << "子线程: " << ++async_count << endl;
                }
                });
            }));
    }
}

int main() {
    ::UnitTest::TestReporterStdout rpt;
    ::UnitTest::TestRunner runner(rpt);
    runner.RunTestsIf(::UnitTest::Test::GetTestList(), nullptr, ::UnitTest::True(), 0);

    MainThread::shared().exec();
    return 0;
}
