﻿#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporterStdout.h>

#include <jsoncpp/json/json.h>
#include <tinyxml2/tinyxml2.h>

#include <sstream>
#include <cross.hpp>
#include <fs.hpp>
#include <str.hpp>
#include <datetime.hpp>
#include <logger.hpp>
#include <json.hpp>
#include <xml.hpp>
#include <url.hpp>
#include <threads.hpp>
#include <connector_curl.hpp>
#include <connector_lws.hpp>
#include <connector_boost.hpp>
#include <digest.hpp>
#include <zip.hpp>
#include <sys.hpp>
#include <timer.hpp>
#include <stringbuilder.hpp>

// #define WEBSOCKET_CONNECTOR LibWebSocketConnector
#define WEBSOCKET_CONNECTOR BoostWebSocketConnector

USE_NNT;
USE_CROSS;

TEST (sys)
{
	string str = " a b c ";
	UNITTEST_CHECK_EQUAL(replace(str, " ", "/"), "/a/b/c/");

	stringbuilder sb;
	sb.space(" ");
	cout <<
		 sb.add("当前线程号:").add(get_thread_id()).ln()
			 .add("uuid:").add(uuid()).ln();

    UNITTEST_CHECK_EQUAL("ABC", toupper("abc"));
}

TEST (test)
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

TEST (fs)
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

    mv("xx", "yy");
    UNITTEST_CHECK(exists("yy"));
    rmfile("yy");
}

TEST (url)
{
	string str = "http://www.baidu.com:80/abc/cde?abc=123&cde=123";
	Url u(str);
	UNITTEST_CHECK_EQUAL(u.toString(), str);
}

TEST (thread)
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

TEST (time)
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

NNT_HEAP_OBJECT_EXPRESS(FixedTaskDispatcher, dis_ws, {
	self.start();
});

TEST (ws)
{
	return;

	WEBSOCKET_CONNECTOR cnt;
	cnt.url = "wss://echo.websocket.org:443";
	cnt.connect();
	cnt.write("hello");
	auto body = cnt.wait();
	auto receive = body->str();
	UNITTEST_CHECK_EQUAL(receive, "hello");
	cnt.write("test");
	body = cnt.wait();
	receive = body->str();
	UNITTEST_CHECK_EQUAL(receive, "test");
	dis_ws.add([](ITask&)
	{
		WEBSOCKET_CONNECTOR cnt;
		cnt.url = "wss://echo.websocket.org:443";
		cnt.connect();
		string input;
		while (input[0] != 'q')
		{
			cin >> input;
			if (!cnt.write(input))
			{
				break;
			}
			auto body = cnt.wait();
			cout << "输入了 " << body->str() << endl;
		}
		cout << "退出WebSocket Echo测试" << endl;
	});
}

TEST (md5)
{
	UNITTEST_CHECK_EQUAL(md5str("hello"), "5d41402abc4b2a76b9719d911017c592");
}

TEST (zip)
{
	mkdir("test-file");
	mkdir("test-dir");

	bool suc = unzip("../test/test-file.zip", "test-file");
	UNITTEST_CHECK_EQUAL(suc, true);
	suc = unzip("../test/test-dir.zip", "test-dir");
	UNITTEST_CHECK_EQUAL(suc, true);
}

TEST (rest)
{
	CurlHttpConnector cnt;
	cnt.url = "https://cn.bing.com/search";
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

TEST (download)
{
	dis_download.start();

	// 阻塞模式
	CurlDownloadConnector cnt;
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
		CurlDownloadConnector cnt;
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

TEST (prop)
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

TEST (task)
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

TEST (async_task)
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

int main()
{
	using namespace ::UnitTest;

	cd(path_home());

	TestReporterStdout rpt;
	TestRunner runner(rpt);

	runner.RunTestsIf(Test::GetTestList(), nullptr, True(), 0);

	MainThread::shared().exec();
	return 0;
}
