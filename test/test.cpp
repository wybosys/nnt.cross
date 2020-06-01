#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporterStdout.h>

#include <sstream>
#include <cross.hpp>
#include <fs.hpp>
#include <datetime.hpp>
#include <logger.hpp>
#include <json.hpp>
#include <xml.hpp>
#include <url.hpp>
#include <threads.hpp>
#include <connector_curl.hpp>
#include <connector_lws.hpp>
#include <digest.hpp>
#include <zip.hpp>

USE_CROSS;

TEST (test) {
    ::std::stringstream ss;
    ss << "abc";
    UNITTEST_CHECK_EQUAL(ss.str().length(), 3);
    string val;
    ss >> val;
    UNITTEST_CHECK_EQUAL(val, "abc");
    UNITTEST_CHECK_EQUAL(ss.str().length(), 3);
}

TEST (fs) {
    string dir = "xxx/abc/cde/efg";
    mkdirs(dir);
    rmtree("xxx");

    UNITTEST_CHECK_EQUAL(Time::Current(), (timestamp_t) Time::Now());

    Logger t;
    t.warn("hahaha");

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

TEST (url) {
    string str = "http://www.baidu.com:80/abc/cde?abc=123&cde=123";
    Url u(str);
    UNITTEST_CHECK_EQUAL(u.toString(), str);
}

TEST (ws) {
    LibWebSocketConnector cnt;
    cnt.url = "ws://192.168.102.200:60304";
    //cnt.connect();
}

TEST (md5) {
    UNITTEST_CHECK_EQUAL(md5str("hello"), "5d41402abc4b2a76b9719d911017c592");
}

TEST (zip) {
    mkdir("test-file");
    mkdir("test-dir");

    bool suc = unzip("../../test/test-file.zip", "test-file");
    UNITTEST_CHECK_EQUAL(suc, true);
    suc = unzip("../../test/test-dir.zip", "test-dir");
    UNITTEST_CHECK_EQUAL(suc, true);
}

TEST (rest) {
    CurlHttpConnector cnt;
    cnt.url = "https://cn.bing.com/search";
    cnt.method = HttpConnector::METHOD_POST;
    cnt.setarg("q", "abc");
    UNITTEST_CHECK_EQUAL(cnt.send(), true);
    cout << cnt.body().str() << endl;
    for (auto &e : cnt.respheaders()) {
        cout << e.first << ":" << e.second << endl;
    }
}

TEST (prop) {
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

TEST (task) {
    // 测试单线程任务池
    // SingleTaskDispatcher dis;

    // 测试定长线程任务池
    // FixedTaskDispatcher dis;

    // 测试可变线程任务池
    QueuedTaskDispatcher dis;

    // dis.attach();

    ::std::atomic<int> count(0);

    for (int i = 0; i < 100; ++i) {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask *) {
            cout << ++count << endl;
            Time::Sleep(1);
        }));
    }
    dis.start();
    for (int i = 0; i < 100; ++i) {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask *) {
            cout << ++count << endl;
            Time::Sleep(1);
        }));
    }
    dis.wait();
}

FixedTaskDispatcher dis;
::std::atomic<int> async_count(9000);

TEST (async_task) {
    // 测试异步线程
    dis.start();

    for (int i = 0; i < 100; ++i) {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask *) {
            if (IsMainThread()) {
                cout << "主线程: " << ++async_count << endl;
            } else {
                cout << "子线程: " << ++async_count << endl;
            }
        }));
    }

    for (int i = 0; i < 100; ++i) {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask *) {
            MainThread::shared().invoke([&]() {
                if (IsMainThread()) {
                    cout << "主线程: " << ++async_count << endl;
                } else {
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
