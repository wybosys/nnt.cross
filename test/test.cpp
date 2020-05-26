#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporterStdout.h>

#include <cross.h>
#include <fs.h>

TEST(fs)
{
    USE_CROSS;

    string dir = "test/abc/cde/efg";
    mkdirs(dir);
    rmtree("test");
}

int main()
{
    ::UnitTest::TestReporterStdout rpt;
    ::UnitTest::TestRunner runner(rpt);
    runner.RunTestsIf(::UnitTest::Test::GetTestList(), nullptr, ::UnitTest::True(), 0);
    return 0;
}