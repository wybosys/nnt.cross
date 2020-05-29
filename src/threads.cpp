#include "cross.h"
#include "threads.h"
#include "datetime.h"
#include <list>
#include <set>

CROSS_BEGIN

void MainThreadTick() {

}

void MainThreadExec() {
    
}

ITask::ITask(func_type fn)
    :proc(move(fn))
{

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
    tsk->_main();
}

class SingleTaskDispatcherPrivate 
{
public:

    SingleTaskDispatcher *owner;
    bool newthd = true; // 是否需要开一个新线程承载
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
        // cout << "开始执行任务" << endl;
         
        self->mtx_tasks.lock();
        auto snap = self->tasks;
        // cout << "任务数量:" << snap.size() << endl;
        self->mtx_tasks.unlock();

        for (auto e : snap) {
            self->owner->_run(e);
        }

        // 移除所有执行完成的任务
        self->mtx_tasks.lock();
        for (auto e : snap) {
            self->tasks.erase(e);
        }
        const size_t left = self->tasks.size();
        snap.clear();
        self->mtx_tasks.unlock();
            
        if (self->waitwait && left == 0) {
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
        // cout << "添加一个任务" << endl;
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

};

FixedTaskDispatcher::FixedTaskDispatcher(size_t count)
{
    NNT_CLASS_CONSTRUCT();
}

FixedTaskDispatcher::~FixedTaskDispatcher()
{
    NNT_CLASS_DESTORY();
}

bool FixedTaskDispatcher::add(task_type&&)
{
    return true;
}

void FixedTaskDispatcher::start()
{

}

void FixedTaskDispatcher::stop()
{

}

void FixedTaskDispatcher::wait()
{

}

void FixedTaskDispatcher::cancel()
{

}

bool FixedTaskDispatcher::isrunning() const
{
    return false;
}

void FixedTaskDispatcher::clear()
{

}

class QueuedTaskDispatcherPrivate
{
public:

};

QueuedTaskDispatcher::QueuedTaskDispatcher(size_t min, size_t max)
{
    NNT_CLASS_CONSTRUCT();
}

QueuedTaskDispatcher::~QueuedTaskDispatcher()
{
    NNT_CLASS_DESTORY();
}

bool QueuedTaskDispatcher::add(task_type&&)
{
    return true;
}

void QueuedTaskDispatcher::start()
{

}

void QueuedTaskDispatcher::stop()
{

}

void QueuedTaskDispatcher::wait() 
{

}

void QueuedTaskDispatcher::cancel()
{

}

bool QueuedTaskDispatcher::isrunning() const
{
    return false;
}

void QueuedTaskDispatcher::clear()
{

}

CROSS_END