#include "cross.h"
#include "threads.h"
#include "datetime.h"
#include <thread>
#include <mutex>
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
    bool running = false, waitstop = false;
    shared_ptr<thread> thd;

    condition_variable cond;
    mutex mtx_tasks, mtx_cond;

    set<ITask::task_type> tasks;

    void start()
    {
        if (running)
            return;
        running = true;

        if (newthd) {
            thd = make_shared<thread>(ThdProc, this);
            thd->detach();
        }
        else {
            ThdProc(this);
        }
    }

    static void ThdProc(SingleTaskDispatcherPrivate *self)
    {
        self->mtx_tasks.lock();
        auto snap = self->tasks;
        self->mtx_tasks.unlock();

        for (auto e : snap) {
            self->owner->_run(e);
        }

        // 移除所有执行完成的任务
        self->mtx_tasks.lock();
        for (auto e : snap) {
            self->tasks.erase(e);
        }
        self->mtx_tasks.unlock();

        // 启动等待，并开始下一次迭代
        unique_lock<mutex> lck(self->mtx_cond);
        self->cond.wait(lck);

        if (!self->waitstop)
            ThdProc(self);
    }

    void stop()
    {
        if (!running)
            return;

        waitstop = true;
        cond.notify_all();

        if (thd) {
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

bool SingleTaskDispatcher::add(task_type const& tsk)
{
    if (tsk->dispatcher())
        return false;
    d_ptr->tasks.emplace(tsk);
    d_ptr->cond.notify_all();
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

bool FixedTaskDispatcher::add(task_type const&)
{
    return true;
}

void FixedTaskDispatcher::start()
{

}

void FixedTaskDispatcher::stop()
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

bool QueuedTaskDispatcher::add(task_type const&)
{
    return true;
}

void QueuedTaskDispatcher::start()
{

}

void QueuedTaskDispatcher::stop()
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