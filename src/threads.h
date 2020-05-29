#ifndef __NNTCROSS_THREADS_H_INCLUDED
#define __NNTCROSS_THREADS_H_INCLUDED

#include <functional>

CROSS_BEGIN

// 业务的主线程每帧回调
extern NNT_API void MainThreadTick();

// 用于做主线程大循环
extern NNT_API void MainThreadExec();

class Task
{
    NNT_NOCOPY(Task);

public:
    typedef function<void()> func_type;

    Task(func_type fn);
    virtual ~Task();

    virtual void main() {}
    
    func_type proc;
};

// 接口线程调度器
class ITaskDispatcher : public IObject
{
public:

    typedef shared_ptr<Task> task_type;

    virtual void add(task_type const&) = 0;
};

NNT_CLASS_PREPARE(SingleTaskDispatcher);

// 单线程任务调度
class SingleTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(SingleTaskDispatcher);

public:

    SingleTaskDispatcher();
    virtual ~SingleTaskDispatcher();

};

NNT_CLASS_PREPARE(FixedTaskDispatcher);

// 定长线程任务调度
class FixedTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(FixedTaskDispatcher);

public:

    FixedTaskDispatcher();
    virtual ~FixedTaskDispatcher();

};

NNT_CLASS_PREPARE(QueuedTaskDispatcher);

// 队列线程任务调度
class QueuedTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(QueuedTaskDispatcher);

public:

    QueuedTaskDispatcher();
    virtual ~QueuedTaskDispatcher();

};

CROSS_END

#endif