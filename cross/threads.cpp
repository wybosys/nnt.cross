#include "cross.hpp"
#include "threads.hpp"
#include "datetime.hpp"
#include <list>
#include <set>
#include <csignal>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "sys.hpp"
#include "logger.hpp"

#ifdef NNT_WINDOWS
#include <Windows.h>
#endif

CROSS_BEGIN

// ---------------------------------- MainThread

NNT_SINGLETON_IMPL(MainThread);

void MainThread::_shared_init()
{
	// pass
}

class MainThreadPrivate
{
public:

	MainThreadPrivate()
	{
#ifdef NNT_WINDOWS
		::SetConsoleCtrlHandler(FnQuit, TRUE);
#endif

		// 实例化的线程定义为主线程（一般如此）
		ismainthread = true;
	}

	static bool waitquit;

#ifdef NNT_WINDOWS
	static BOOL WINAPI FnQuit(DWORD type)
	{
		waitquit = true;
		return TRUE;
	}
#else

	static void FnQuit()
	{
		waitquit = true;
	}

#endif

	::std::mutex mtx_funcs;
	::std::vector<MainThread::func_type> funcs;

	static thread_local bool ismainthread;
};

bool MainThreadPrivate::waitquit = false;

bool thread_local MainThreadPrivate::ismainthread = false;

MainThread::MainThread()
{
	NNT_CLASS_CONSTRUCT();
}

MainThread::~MainThread()
{
	NNT_CLASS_DESTROY();

	// 清理
	PreferredInvokeImpl = nullptr;
}

::std::function<void(MainThread::func_type const&)> MainThread::PreferredInvokeImpl = nullptr;

void MainThread::exec()
{
	while (!private_class_type::waitquit)
	{
		tick();
		Time::Sleep(1);
	}
}

void MainThread::tick()
{
	NNT_AUTOGUARD(d_ptr->mtx_funcs);
	for (auto& e : d_ptr->funcs)
	{
		e();
	}
	d_ptr->funcs.clear();
}

void MainThread::Invoke(func_type const& fn)
{
	shared().invoke(fn);
}

void MainThread::invoke(func_type const& fn)
{
	if (PreferredInvokeImpl)
	{
		// 使用业务层的实现
		PreferredInvokeImpl(fn);
		return;
	}

	if (MainThreadPrivate::ismainthread)
	{
		// 如果已经在主线程，则直接运行
		fn();
	}
	else
	{
		NNT_AUTOGUARD(d_ptr->mtx_funcs);
		d_ptr->funcs.emplace_back(fn);
	}
}

bool IsMainThread()
{
	return MainThreadPrivate::ismainthread;
}

// ------------------------------------------ Semaphore

class semaphorePrivate
{
public:
	::std::mutex mtx;
	::std::condition_variable cond;
	size_t count = 0, waits = 0;
};

semaphore::semaphore()
{
	NNT_CLASS_CONSTRUCT();
}

semaphore::~semaphore()
{
	NNT_CLASS_DESTROY();
}

void semaphore::notify()
{
	::std::lock_guard<decltype(d_ptr->mtx)> lck(d_ptr->mtx);
	++d_ptr->count;
	d_ptr->cond.notify_one();
}

void semaphore::notify_all()
{
	::std::lock_guard<decltype(d_ptr->mtx)> lck(d_ptr->mtx);
	d_ptr->count = d_ptr->waits;
	d_ptr->cond.notify_all();
}

void semaphore::wait()
{
	::std::unique_lock<decltype(d_ptr->mtx)> lck(d_ptr->mtx);
	while (!d_ptr->count)
	{
		d_ptr->waits += 1;
		d_ptr->cond.wait(lck);
		d_ptr->waits -= 1;
	}
	--d_ptr->count;
}

bool semaphore::try_wait()
{
	::std::lock_guard<decltype(d_ptr->mtx)> lck(d_ptr->mtx);
	if (d_ptr->count)
	{
		--d_ptr->count;
		return true;
	}
	return false;
}

bool semaphore::waiting() const
{
	return d_ptr->waits != 0;
}

// ---------------------------------------- Thread

class ThreadPrivate
{
public:
	tid_t tid = -1;
	bool waitquit = false;
	string name;
	::std::shared_ptr<::std::thread> thd;
	::std::mutex mtx_start, mtx_running;
	semaphore sema_start;
};

Thread::Thread(string const& name)
{
	NNT_CLASS_CONSTRUCT();
	d_ptr->name = name;
}

Thread::~Thread()
{
	// 等待安全退出
	quit();

	NNT_CLASS_DESTROY();
}

void Thread::wait() const
{
	d_ptr->sema_start.wait();
}

tid_t Thread::tid() const
{
	return d_ptr->tid;
}

void Thread::start()
{
	auto d = this->d_ptr;
	NNT_AUTOGUARD(d->mtx_start);

	if (d->thd)
	{
		Logger::Warn("线程 " + d->name + " 已经启动");
		return;
	}

	d->waitquit = false;

	d->thd = ::std::make_shared<::std::thread>([=]()
	{
		auto const tname = d->name;
		Logger::Debug("线程 " + tname + " 启动");
		d->sema_start.notify();

		if (!tname.empty())
			set_thread_name(tname);
		d->tid = get_thread_id();

		size_t count = this->repeat;
		while (d && !d->waitquit)
		{
			NNT_AUTOGUARD(d->mtx_running);

			// 执行线程任务
			main();
			if (proc)
				proc(*this);

			if (count != FOREVER)
			{
				// -1代表无限循环，》1代表有限次
				if (--count <= 0)
					break;
			}
		}

		Logger::Debug("线程 " + tname + " 退出");
	});
}

void Thread::quit()
{
	NNT_AUTOGUARD(d_ptr->mtx_start);

	if (!d_ptr->thd)
		return;

	d_ptr->waitquit = true;

	// 如果是同一个线程，则不能进行join，会导致deadlock
	if (get_thread_id() != d_ptr->tid)
	{
		// 工作线程内join会导致死锁
		d_ptr->thd->join();
	}
	else
	{
		// 解除联系，避免 thd=null 时释放系统县城资源导致崩溃
		d_ptr->thd->detach();
	}

	d_ptr->thd = nullptr;
}

// ---------------------------------------- Tasks

ITask::ITask(func_type fn)
	: proc(move(fn))
{
	// pass
}

void ITask::_main()
{
	_running = true;
	main();
	if (proc)
		proc(*this);
	_running = false;
}

bool ITask::isrunning() const
{
	return _running;
}

ITaskDispatcher* ITask::dispatcher() const
{
	return _dispatcher;
}

void ITaskDispatcher::_run(task_type& tsk)
{
	if (tsk)
		tsk->_main();
}

class SingleTaskDispatcherPrivate
{
public:

	SingleTaskDispatcher* owner;
	bool newthd = true; // 是否需要开一个新线程承载
	bool running = false, waitstop = false, waitwait = false;
	shared_ptr<::std::thread> thd;
	semaphore smp_tasks;
	::std::mutex mtx_tasks;
	::std::set<ITask::task_type> tasks;

	void start()
	{
		if (running)
			return;
		running = true;

		waitwait = false;
		waitstop = false;

		if (newthd)
		{
			thd = make_shared<::std::thread>(ThdProc, this);
		}
		else
		{
			for (auto e : tasks)
			{
				owner->_run(e);
			}
			tasks.clear();
		}
	}

	static void ThdProc(SingleTaskDispatcherPrivate* self)
	{
		// cout << "开始执行任务" << endl;

		self->mtx_tasks.lock();
		auto snap = self->tasks;
		self->tasks.clear();
		// cout << "任务数量:" << snap.size() << endl;
		self->mtx_tasks.unlock();

		for (auto e : snap)
		{
			self->owner->_run(e);
		}

		// 移除所有执行完成的任务
		self->mtx_tasks.lock();
		const size_t left = self->tasks.size();
		snap.clear();
		self->mtx_tasks.unlock();

		if (self->waitwait && left == 0)
		{
			// 所有任务已经运行结束
			return;
		}

		// 启动等待，并开始下一次迭代
		// cout << "等待添加任务" << endl;
		self->smp_tasks.wait();

		if (!self->waitstop)
			ThdProc(self);
	}

	void stop()
	{
		if (!running)
			return;

		waitstop = true;
		smp_tasks.notify_all();

		if (thd && thd->joinable())
		{
			thd->join();
			thd = nullptr;
		}

		running = false;
	}
};

SingleTaskDispatcher::SingleTaskDispatcher()
{
	NNT_CLASS_CONSTRUCT();
	d_ptr->owner = this;
}

SingleTaskDispatcher::~SingleTaskDispatcher()
{
	d_ptr->stop();
	NNT_CLASS_DESTROY();
}

bool SingleTaskDispatcher::add(task_type&& tsk)
{
	if (tsk->dispatcher())
		return false;
	if (d_ptr->newthd)
	{
		d_ptr->tasks.emplace(tsk);
		// cout << "添加一个任务" << endl;
		d_ptr->smp_tasks.notify();
	}
	else
	{
		_run(tsk);
	}
	return true;
}

void SingleTaskDispatcher::start()
{
	d_ptr->start();
}

void SingleTaskDispatcher::stop()
{
	d_ptr->stop();
}

void SingleTaskDispatcher::wait()
{
	d_ptr->waitwait = true;
	if (d_ptr->thd && d_ptr->thd->joinable())
	{
		d_ptr->thd->join();
		d_ptr->thd = nullptr;
	}
}

void SingleTaskDispatcher::cancel()
{
	for (auto& e : d_ptr->tasks)
	{
		e->cancel();
	}
	clear();
}

bool SingleTaskDispatcher::isrunning() const
{
	return d_ptr->running;
}

void SingleTaskDispatcher::clear()
{
	NNT_AUTOGUARD(d_ptr->mtx_tasks);
	d_ptr->tasks.clear();
}

bool SingleTaskDispatcher::attach()
{
	if (d_ptr->thd)
		return false;
	d_ptr->newthd = false;
	return true;
}

class FixedTaskDispatcherPrivate
{
public:

	FixedTaskDispatcher* owner;
	::std::mutex mtx_tasks;
	::std::set<ITask::task_type> tasks;
	size_t maxcount;
	bool running = false, waitstop = false, waitwait = false;

	::std::vector<shared_ptr<::std::thread>> thds;
	semaphore smp_tasks;

	void start()
	{
		if (running)
			return;
		running = true;

		waitwait = false;
		waitstop = false;

		for (size_t i = 0; i < maxcount; ++i)
		{
			auto t = make_shared<::std::thread>(ThdProc, this);
			thds.emplace_back(t);
		}
	}

	void stop()
	{
		if (!running)
			return;

		waitstop = true;
		smp_tasks.notify_all();

		for (auto& e : thds)
		{
			if (e->joinable())
			{
				e->join();
				e = nullptr;
			}
		}
		thds.clear();

		running = false;
	}

	static void ThdProc(FixedTaskDispatcherPrivate* self)
	{
		// cout << "开始执行任务" << endl;

		self->mtx_tasks.lock();
		ITask::task_type tsk;
		if (!self->tasks.empty())
		{
			tsk = *self->tasks.begin();
			self->tasks.erase(self->tasks.begin());
		}
		self->mtx_tasks.unlock();

		self->owner->_run(tsk);

		// 移除所有执行完成的任务
		self->mtx_tasks.lock();
		const size_t left = self->tasks.size();
		self->mtx_tasks.unlock();

		if (self->waitwait && left == 0)
		{
			// 所有任务已经运行结束
			return;
		}

		// 启动等待，并开始下一次迭代
		// cout << "等待添加任务" << endl;
		self->smp_tasks.wait();

		if (!self->waitstop)
			ThdProc(self);
	}
};

FixedTaskDispatcher::FixedTaskDispatcher()
{
	NNT_CLASS_CONSTRUCT();
	d_ptr->owner = this;
	d_ptr->maxcount = ::std::thread::hardware_concurrency();
}

FixedTaskDispatcher::FixedTaskDispatcher(size_t count)
{
	NNT_CLASS_CONSTRUCT();
	d_ptr->owner = this;
	d_ptr->maxcount = count < 1 ? ::std::thread::hardware_concurrency() : count;
}

FixedTaskDispatcher::~FixedTaskDispatcher()
{
	d_ptr->stop();
	NNT_CLASS_DESTROY();
}

bool FixedTaskDispatcher::add(task_type&& tsk)
{
	if (tsk->dispatcher())
		return false;

	NNT_AUTOGUARD(d_ptr->mtx_tasks);
	d_ptr->tasks.emplace(tsk);
	d_ptr->smp_tasks.notify();
	return true;
}

void FixedTaskDispatcher::start()
{
	d_ptr->start();
}

void FixedTaskDispatcher::stop()
{
	d_ptr->stop();
}

void FixedTaskDispatcher::wait()
{
	d_ptr->waitwait = true;
	for (auto& e : d_ptr->thds)
	{
		if (e->joinable())
		{
			e->join();
			e = nullptr;
		}
	}
	d_ptr->thds.clear();
}

void FixedTaskDispatcher::cancel()
{
	for (auto& e : d_ptr->tasks)
	{
		e->cancel();
	}
	clear();
}

bool FixedTaskDispatcher::isrunning() const
{
	return d_ptr->running;
}

void FixedTaskDispatcher::clear()
{
	NNT_AUTOGUARD(d_ptr->mtx_tasks);
	d_ptr->tasks.clear();
}

class QueuedTaskDispatcherPrivate
{
public:

	QueuedTaskDispatcher* owner;
	::std::mutex mtx_tasks;
	::std::set<ITask::task_type> tasks;
	bool running = false, waitstop = false, waitwait = false;
	::std::atomic<size_t> thd_runnings;

	::std::vector<shared_ptr<::std::thread>> thds;
	semaphore smp_tasks;

	void start()
	{
		if (running)
			return;
		running = true;

		waitwait = false;
		waitstop = false;

		for (size_t i = 0; i < mincount; ++i)
		{
			auto t = make_shared<::std::thread>(ThdProc, this);
			thds.emplace_back(t);
		}
	}

	void stop()
	{
		if (!running)
			return;

		waitstop = true;
		smp_tasks.notify_all();

		for (auto& e : thds)
		{
			if (e->joinable())
			{
				e->join();
				e = nullptr;
			}
		}
		thds.clear();

		running = false;
	}

	static void ThdProc(QueuedTaskDispatcherPrivate* self)
	{
		// cout << "开始执行任务" << endl;

		++self->thd_runnings;

		self->mtx_tasks.lock();
		ITask::task_type tsk;
		if (!self->tasks.empty())
		{
			tsk = *self->tasks.begin();
			self->tasks.erase(self->tasks.begin());
		}
		self->mtx_tasks.unlock();

		self->owner->_run(tsk);

		// 移除所有执行完成的任务
		self->mtx_tasks.lock();
		const size_t left = self->tasks.size();
		self->mtx_tasks.unlock();

		--self->thd_runnings;

		if (self->waitwait && left == 0)
		{
			// 所有任务已经运行结束
			return;
		}

		// 启动等待，并开始下一次迭代
		// cout << "等待添加任务" << endl;
		self->smp_tasks.wait();

		if (!self->waitstop)
			ThdProc(self);
	}

	void create_worker()
	{
		auto t = make_shared<::std::thread>(ThdProc, this);
		thds.emplace_back(t);
	}

	size_t mincount, maxcount;
};

QueuedTaskDispatcher::QueuedTaskDispatcher()
{
	NNT_CLASS_CONSTRUCT();
	d_ptr->owner = this;
	d_ptr->mincount = ::std::thread::hardware_concurrency();
	d_ptr->maxcount = d_ptr->mincount << 1;
}

QueuedTaskDispatcher::QueuedTaskDispatcher(size_t min, size_t max)
{
	NNT_CLASS_CONSTRUCT();
	d_ptr->owner = this;
	d_ptr->mincount = min < 1 ? ::std::thread::hardware_concurrency() : min;
	d_ptr->maxcount = min < max ? max : min;
}

QueuedTaskDispatcher::~QueuedTaskDispatcher()
{
	d_ptr->stop();
	NNT_CLASS_DESTROY();
}

bool QueuedTaskDispatcher::add(task_type&& tsk)
{
	if (tsk->dispatcher())
		return false;

	NNT_AUTOGUARD(d_ptr->mtx_tasks);
	d_ptr->tasks.emplace(tsk);

	if (d_ptr->thd_runnings == d_ptr->thds.size())
	{
		if (d_ptr->thds.size() < d_ptr->maxcount)
		{
			// 创建一个新工作线程
			d_ptr->create_worker();
		}
	}

	d_ptr->smp_tasks.notify();
	return true;
}

void QueuedTaskDispatcher::start()
{
	d_ptr->start();
}

void QueuedTaskDispatcher::stop()
{
	d_ptr->stop();
}

void QueuedTaskDispatcher::wait()
{
	d_ptr->waitwait = true;
	for (auto& e : d_ptr->thds)
	{
		if (e->joinable())
		{
			e->join();
			e = nullptr;
		}
	}
	d_ptr->thds.clear();
}

void QueuedTaskDispatcher::cancel()
{
	for (auto& e : d_ptr->tasks)
	{
		e->cancel();
	}
	clear();
}

bool QueuedTaskDispatcher::isrunning() const
{
	return d_ptr->running;
}

void QueuedTaskDispatcher::clear()
{
	NNT_AUTOGUARD(d_ptr->mtx_tasks);
	d_ptr->tasks.clear();
}

class _ThreadResourceProviderPrivate
{
public:

	void* _obj = nullptr;
	semaphore _wait_start, _wait_stop;
	shared_ptr<::std::thread> _thd;

	static void _ThdWorker(_ThreadResourceProvider* self)
	{
		if (!self->d_ptr->_obj)
		{
			self->d_ptr->_obj = self->_init();
		}

		// 放开start
		self->d_ptr->_wait_start.notify();

		// 卡住线程等待结束
		self->d_ptr->_wait_stop.wait();

		// 清理资源
		if (self->d_ptr->_obj)
		{
			self->_delete(self->d_ptr->_obj);
			self->d_ptr->_obj = nullptr;
		}
	}
};

_ThreadResourceProvider::_ThreadResourceProvider()
{
	NNT_CLASS_CONSTRUCT();
}

_ThreadResourceProvider::~_ThreadResourceProvider()
{
	stop();
	NNT_CLASS_DESTROY();
}

void _ThreadResourceProvider::start()
{
	if (d_ptr->_thd)
		return;
	d_ptr->_thd = make_shared<::std::thread>(private_class_type::_ThdWorker, this);
	d_ptr->_wait_start.wait();
}

void _ThreadResourceProvider::stop()
{
	if (!d_ptr->_thd)
		return;

	// 自动退出线程
	d_ptr->_wait_stop.notify();
	d_ptr->_thd->join();
	d_ptr->_thd = nullptr;
}

void* _ThreadResourceProvider::_obj() const
{
	return d_ptr->_obj;
}

CROSS_END
