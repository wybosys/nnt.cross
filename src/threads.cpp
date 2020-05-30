#include "cross.hpp"
#include "threads.hpp"
#include "datetime.hpp"
#include <list>
#include <set>
#include <csignal>

#ifdef NNT_WINDOWS
#include <Windows.h>
#endif

CROSS_BEGIN

NNT_SINGLETON_IMPL(MainThread);

class MainThreadPrivate
{
public:

    MainThreadPrivate() {
#ifdef NNT_WINDOWS
        ::SetConsoleCtrlHandler(FnQuit, TRUE);
#endif
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

    mutex mtx_funcs;
    vector<MainThread::func_type> funcs;

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
    NNT_CLASS_DESTORY();
}

void MainThread::exec()
{
    while (!private_class_type::waitquit) {
        tick();
        Time::Sleep(1);
    }
}

void MainThread::tick()
{
    MainThreadPrivate::ismainthread = true;

    NNT_AUTOGUARD(d_ptr->mtx_funcs);
    for (auto &e : d_ptr->funcs) {
        e();
    }
    d_ptr->funcs.clear();
}

void MainThread::invoke(func_type const& fn)
{
    if (MainThreadPrivate::ismainthread) {
        // ����Ѿ������̣߳���ֱ������
        fn();
    }
    else {
        NNT_AUTOGUARD(d_ptr->mtx_funcs);
        d_ptr->funcs.emplace_back(fn);
    }
}

bool IsMainThread()
{
    return MainThreadPrivate::ismainthread;
}

ITask::ITask(func_type fn)
    :proc(move(fn))
{
    // pass
}

void ITask::_main()
{
    _running = true;
    main();
    if (proc)
        proc(this);
    _running = false;
}

bool ITask::isrunning() const {
    return _running;
}

ITaskDispatcher *ITask::dispatcher() const {
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

    SingleTaskDispatcher *owner;
    bool newthd = true; // �Ƿ���Ҫ��һ�����̳߳���
    bool running = false, waitstop = false, waitwait = false;
    shared_ptr<thread> thd;
    semaphore smp_tasks;
    mutex mtx_tasks;
    set<ITask::task_type> tasks;

    void start()
    {
        if (running)
            return;
        running = true;

        waitwait = false;
        waitstop = false;

        if (newthd) {
            thd = make_shared<thread>(ThdProc, this);
        }
        else {
            for (auto e : tasks) {
                owner->_run(e);
            }
            tasks.clear();
        }
    }

    static void ThdProc(SingleTaskDispatcherPrivate *self)
    {
        // cout << "��ʼִ������" << endl;
         
        self->mtx_tasks.lock();
        auto snap = self->tasks;
        self->tasks.clear();
        // cout << "��������:" << snap.size() << endl;
        self->mtx_tasks.unlock();

        for (auto e : snap) {
            self->owner->_run(e);
        }

        // �Ƴ�����ִ����ɵ�����
        self->mtx_tasks.lock();
        const size_t left = self->tasks.size();
        snap.clear();
        self->mtx_tasks.unlock();
            
        if (self->waitwait && left == 0) {
            // ���������Ѿ����н���
            return;
        }

        // �����ȴ�������ʼ��һ�ε���
        // cout << "�ȴ��������" << endl;
        self->smp_tasks.wait();

        if (!self->waitstop)
            ThdProc(self);
    }

    void stop()
    {
        if (!running)
            return;

        waitstop = true;
        smp_tasks.notify();

        if (thd && thd->joinable()) {
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
    NNT_CLASS_DESTORY();
}

bool SingleTaskDispatcher::add(task_type&& tsk)
{
    if (tsk->dispatcher())
        return false;
    if (d_ptr->newthd) {
        d_ptr->tasks.emplace(tsk);
        // cout << "���һ������" << endl;
        d_ptr->smp_tasks.notify();
    }
    else {
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
    if (d_ptr->thd && d_ptr->thd->joinable()) {
        d_ptr->thd->join();
        d_ptr->thd = nullptr;
    }
}

void SingleTaskDispatcher::cancel()
{
    for (auto &e : d_ptr->tasks) {
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

    FixedTaskDispatcher *owner;
    mutex mtx_tasks;
    set<ITask::task_type> tasks;
    size_t maxcount;
    bool running = false, waitstop = false, waitwait = false;

    vector<shared_ptr<thread>> thds;
    semaphore smp_tasks;

    void start()
    {
        if (running)
            return;
        running = true;

        waitwait = false;
        waitstop = false;

        for (size_t i = 0; i < maxcount; ++i) {
            auto t = make_shared<thread>(ThdProc, this);
            thds.emplace_back(t);
        }
    }

    void stop()
    {
        if (!running)
            return;

        waitstop = true;
        smp_tasks.notify();

        for (auto &e : thds) {
            if (e->joinable()) {
                e->join();
                e = nullptr;
            }
        }
        thds.clear();

        running = false;
    }

    static void ThdProc(FixedTaskDispatcherPrivate *self)
    {
        // cout << "��ʼִ������" << endl;

        self->mtx_tasks.lock();
        ITask::task_type tsk;
        if (!self->tasks.empty()) {
            tsk = *self->tasks.begin();
            self->tasks.erase(self->tasks.begin());
        }
        self->mtx_tasks.unlock();

        self->owner->_run(tsk);

        // �Ƴ�����ִ����ɵ�����
        self->mtx_tasks.lock();
        const size_t left = self->tasks.size();
        self->mtx_tasks.unlock();

        if (self->waitwait && left == 0) {
            // ���������Ѿ����н���
            return;
        }

        // �����ȴ�������ʼ��һ�ε���
        // cout << "�ȴ��������" << endl;
        self->smp_tasks.wait();

        if (!self->waitstop)
            ThdProc(self);
    }
};

FixedTaskDispatcher::FixedTaskDispatcher()
{
    NNT_CLASS_CONSTRUCT();
    d_ptr->owner = this;
    d_ptr->maxcount = thread::hardware_concurrency();
}

FixedTaskDispatcher::FixedTaskDispatcher(size_t count)
{
    NNT_CLASS_CONSTRUCT();
    d_ptr->owner = this;
    d_ptr->maxcount = count < 1 ? thread::hardware_concurrency() : count;
}

FixedTaskDispatcher::~FixedTaskDispatcher()
{
    d_ptr->stop();
    NNT_CLASS_DESTORY();
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
    for (auto &e : d_ptr->thds) {
        if (e->joinable()) {
            e->join();
            e = nullptr;
        }
    }
    d_ptr->thds.clear();
}

void FixedTaskDispatcher::cancel()
{
    for (auto &e : d_ptr->tasks) {
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

    QueuedTaskDispatcher *owner;
    mutex mtx_tasks;
    set<ITask::task_type> tasks;
    bool running = false, waitstop = false, waitwait = false;
    atomic<size_t> thd_runnings;

    vector<shared_ptr<thread>> thds;
    semaphore smp_tasks;

    void start()
    {
        if (running)
            return;
        running = true;

        waitwait = false;
        waitstop = false;

        for (size_t i = 0; i < mincount; ++i) {
            auto t = make_shared<thread>(ThdProc, this);
            thds.emplace_back(t);
        }
    }

    void stop()
    {
        if (!running)
            return;

        waitstop = true;
        smp_tasks.notify();

        for (auto &e : thds) {
            if (e->joinable()) {
                e->join();
                e = nullptr;
            }
        }
        thds.clear();

        running = false;
    }

    static void ThdProc(QueuedTaskDispatcherPrivate *self)
    {
        // cout << "��ʼִ������" << endl;

        ++self->thd_runnings;

        self->mtx_tasks.lock();
        ITask::task_type tsk;
        if (!self->tasks.empty()) {
            tsk = *self->tasks.begin();
            self->tasks.erase(self->tasks.begin());
        }
        self->mtx_tasks.unlock();

        self->owner->_run(tsk);

        // �Ƴ�����ִ����ɵ�����
        self->mtx_tasks.lock();
        const size_t left = self->tasks.size();
        self->mtx_tasks.unlock();

        --self->thd_runnings;

        if (self->waitwait && left == 0) {
            // ���������Ѿ����н���
            return;
        }

        // �����ȴ�������ʼ��һ�ε���
        // cout << "�ȴ��������" << endl;
        self->smp_tasks.wait();

        if (!self->waitstop)
            ThdProc(self);
    }

    void create_worker()
    {
        auto t = make_shared<thread>(ThdProc, this);
        thds.emplace_back(t);
    }

    size_t mincount, maxcount;
};

QueuedTaskDispatcher::QueuedTaskDispatcher()
{
    NNT_CLASS_CONSTRUCT();
    d_ptr->owner = this;
    d_ptr->mincount = thread::hardware_concurrency();
    d_ptr->maxcount = d_ptr->mincount << 1;
}

QueuedTaskDispatcher::QueuedTaskDispatcher(size_t min, size_t max)
{
    NNT_CLASS_CONSTRUCT();
    d_ptr->owner = this;
    d_ptr->mincount = min < 1 ? thread::hardware_concurrency() : min;
    d_ptr->maxcount = min < max ? max : min;
}

QueuedTaskDispatcher::~QueuedTaskDispatcher()
{
    d_ptr->stop();
    NNT_CLASS_DESTORY();
}

bool QueuedTaskDispatcher::add(task_type&& tsk)
{
    if (tsk->dispatcher())
        return false;

    NNT_AUTOGUARD(d_ptr->mtx_tasks);
    d_ptr->tasks.emplace(tsk);

    if (d_ptr->thd_runnings == d_ptr->thds.size()) {
        if (d_ptr->thds.size() < d_ptr->maxcount) {
            // ����һ���¹����߳�
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
    for (auto &e : d_ptr->thds) {
        if (e->joinable()) {
            e->join();
            e = nullptr;
        }
    }
    d_ptr->thds.clear();
}

void QueuedTaskDispatcher::cancel()
{
    for (auto &e : d_ptr->tasks) {
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

CROSS_END