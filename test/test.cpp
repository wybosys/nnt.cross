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
    SingleTaskDispatcher dis;
    atomic<int> count;

    // auto x = make_shared<Task>();

    for (int i = 0; i < 100; ++i) {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask*) {
            cout << ++count << endl;
            }));
    }
    dis.start();
    for (int i = 0; i < 100; ++i) {
        dis.add(make_dynamic_shared<Task, ITask>([&](ITask*) {
            cout << ++count << endl;
            }));
    }
}

int main() {
    ::UnitTest::TestReporterStdout rpt;
    ::UnitTest::TestRunner runner(rpt);
    runner.RunTestsIf(::UnitTest::Test::GetTestList(), nullptr, ::UnitTest::True(), 0);

    MainThreadExec();
    return 0;
}
