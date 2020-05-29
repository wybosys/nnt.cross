#ifndef __NNTCROSS_THREADS_H_INCLUDED
#define __NNTCROSS_THREADS_H_INCLUDED

#include <functional>

CROSS_BEGIN

// 业务的主线程每帧回调
extern NNT_API void MainThreadTick();

// 用于做主线程大循环
extern NNT_API void MainThreadExec();

class ITask : public IObject
{
    NNT_NOCOPY(ITask);

public:

    typedef function<void(ITask*)> func_type;
    typedef shared_ptr<ITask> task_type;

    ITask(func_type fn = nullptr);

    // 继承方式实现运行
    virtual void main() {}
    
    // 或者设定运行函数
    func_type proc;

    // 是否在运行
    bool isrunning() const;

    // 隶属的调度器
    class ITaskDispatcher *dispatcher() const;

    // 复制一个任务
    virtual task_type copy() const = 0;

private:
    void _main();
    bool _running;
    class ITaskDispatcher *_dispatcher = nullptr;
    friend class ITaskDispatcher;
};

template <typename TImpl>
class TaskImpl : public ITask
{
public:
    TaskImpl(func_type fn = nullptr) : ITask(fn) {}
    
    virtual task_type copy() const {
        auto r = make_dynamic_shared<TImpl, ITask>();
        r->proc = proc;
        return r;
    }
};

class Task : public TaskImpl<Task> {
public:
    Task(func_type fn = nullptr) :TaskImpl<Task>(fn) {}
};

// 接口线程调度器
class ITaskDispatcher : public IObject
{
public:

    typedef ITask::task_type task_type;
    
    // 添加一个任务
    virtual bool add(task_type const&) = 0;

    // 开始调度
    virtual void start() = 0;

    // 结束调度
    virtual void stop() = 0;

    // 是否在运行
    virtual bool isrunning() const = 0;

    // 清空任务
    virtual void clear() = 0;

protected:

    // 执行一次任务
    void _run(task_type& tsk);
};

NNT_CLASS_PREPARE(SingleTaskDispatcher);

// 单线程任务调度
class SingleTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(SingleTaskDispatcher);

public:

    SingleTaskDispatcher();
    virtual ~SingleTaskDispatcher();

    virtual bool add(task_type const&);
    virtual void start();
    virtual void stop();
    virtual bool isrunning() const;
    virtual void clear();

    // 连接到当前运行线程
    bool attach();
};

NNT_CLASS_PREPARE(FixedTaskDispatcher);

// 定长线程任务调度
class FixedTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(FixedTaskDispatcher);

public:

    // \@count 使用多少个线程执行任务
    FixedTaskDispatcher(size_t count);
    virtual ~FixedTaskDispatcher();

    virtual bool add(task_type const&);
    virtual void start();
    virtual void stop();
    virtual bool isrunning() const;
    virtual void clear();
};

NNT_CLASS_PREPARE(QueuedTaskDispatcher);

// 队列线程任务调度
class QueuedTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(QueuedTaskDispatcher);

public:

    /*
    \@min 最小多少个线程
    \@max 超过max线程后，其他任务排队等待执行
    */
    QueuedTaskDispatcher(size_t min, size_t max);
    virtual ~QueuedTaskDispatcher();

    virtual bool add(task_type const&);
    virtual void start();
    virtual void stop();
    virtual bool isrunning() const;
    virtual void clear();
};

CROSS_END

#endif