#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporterStdout.h>

#include <cross.h>
#include <fs.h>
#include <time.h>
#include <logger.h>

TEST (fs) {
    USE_CROSS;

    string dir = "xxx/abc/cde/efg";
    mkdirs(dir);
    rmtree("xxx");

    UNITTEST_CHECK_EQUAL(Time::Current(), (timestamp_t)Time::Now());

    Logger t;
    t.warn("hahaha");
}

int main() {
    ::UnitTest::TestReporterStdout rpt;
    ::UnitTest::TestRunner runner(rpt);
    runner.RunTestsIf(::UnitTest::Test::GetTestList(), nullptr, ::UnitTest::True(), 0);
    return 0;
}
