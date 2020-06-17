#ifndef __NNTCROSS_THREADS_H_INCLUDED
#define __NNTCROSS_THREADS_H_INCLUDED

#include <functional>
#include <mutex>

CROSS_BEGIN

NNT_CLASS_PREPARE(MainThread);

// 业务主线程
class NNT_API MainThread {
NNT_CLASS_DECL(MainThread);

    MainThread();

    ~MainThread();

public:

NNT_SINGLETON_DECL(MainThread);

    typedef ::std::function<void()> func_type;

    // 便利性函数
    static void Invoke(func_type const&);

    // 大循环中执行
    void invoke(func_type const &);

    // 业务层实现的大循环接口，如果!=null则 invoke函数直接使用业务层定义的大循环实现
    static ::std::function<void(func_type const&)> PreferredInvokeImpl;

    // 大循环
    void exec();

    // 或者在已有大循环中进行回调
    void tick();
};

// 当前是否在主线程
extern bool IsMainThread();

NNT_CLASS_PREPARE(semaphore);

// 信号量
class NNT_API semaphore {
NNT_CLASS_DECL(semaphore);

public:

    semaphore();

    ~semaphore();

    // 唤醒一个
    void notify();

    // 唤醒所有
    void notify_all();

    // 等待
    void wait();

    // 尝试等待
    bool try_wait();
};

// 任务接口
class NNT_API ITask : public ::NNT_NS::IObject {
NNT_NOCOPY(ITask);

public:

    typedef ::std::function<void(ITask &)> func_type;
    typedef shared_ptr <ITask> task_type;

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

    // 取消任务
    virtual void cancel() {}

private:
    void _main();

    bool _running;

    class ITaskDispatcher *_dispatcher = nullptr;

    friend class ITaskDispatcher;
};

template<typename TImpl>
class TaskImpl : public ITask {
public:
    TaskImpl(func_type fn = nullptr) : ITask(fn) {}

    virtual task_type copy() const {
        auto r = ::NNT_NS::make_dynamic_shared<TImpl, ITask>();
        r->proc = proc;
        return r;
    }
};

class NNT_API Task : public TaskImpl<Task> {
public:
    Task(func_type fn = nullptr) : TaskImpl<Task>(fn) {}
};

// 接口线程调度器
class NNT_API ITaskDispatcher : public ::NNT_NS::IObject {
public:

    typedef ITask::task_type task_type;

    // 添加一个任务
    virtual bool add(task_type &&) = 0;

    virtual bool add(ITask::func_type fn) {
        return add(make_shared<Task>(fn));
    }

    // 开始调度
    virtual void start() = 0;

    // 结束调度
    virtual void stop() = 0;

    // 是否在运行
    virtual bool isrunning() const = 0;

    // 清空任务
    virtual void clear() = 0;

    // 等待所有任务运行完成
    virtual void wait() = 0;

    // 取消所有任务
    virtual void cancel() = 0;

protected:

    // 执行一次任务
    void _run(task_type &tsk);
};

NNT_CLASS_PREPARE(SingleTaskDispatcher);

// 单线程任务调度
class NNT_API SingleTaskDispatcher : public ITaskDispatcher {
NNT_CLASS_DECL(SingleTaskDispatcher);

public:

    SingleTaskDispatcher();

    virtual ~SingleTaskDispatcher();

    virtual bool add(task_type &&);

    virtual bool add(ITask::func_type fn) {
        return ITaskDispatcher::add(fn);
    }

    virtual void start();

    virtual void stop();

    virtual bool isrunning() const;

    virtual void clear();

    virtual void wait();

    virtual void cancel();

    // 连接到当前运行线程
    bool attach();
};

NNT_CLASS_PREPARE(FixedTaskDispatcher);

// 定长线程任务调度
class NNT_API FixedTaskDispatcher : public ITaskDispatcher {
NNT_CLASS_DECL(FixedTaskDispatcher);

public:

    // \@count 使用多少个线程执行任务， 默认为cpu核心线程数
    FixedTaskDispatcher();

    FixedTaskDispatcher(size_t count);

    virtual ~FixedTaskDispatcher();

    virtual bool add(task_type &&);

    virtual bool add(ITask::func_type fn) {
        return ITaskDispatcher::add(fn);
    }

    virtual void start();

    virtual void stop();

    virtual bool isrunning() const;

    virtual void clear();

    virtual void wait();

    virtual void cancel();
};

NNT_CLASS_PREPARE(QueuedTaskDispatcher);

// 队列线程任务调度
class NNT_API QueuedTaskDispatcher : public ITaskDispatcher {
NNT_CLASS_DECL(QueuedTaskDispatcher);

public:

    /*
    \@min 最小多少个线程，默认cpu核心线程数
    \@max 超过max线程后，其他任务排队等待执行，默认为2倍cpu核心线程数
    */
    QueuedTaskDispatcher();

    QueuedTaskDispatcher(size_t min, size_t max);

    virtual ~QueuedTaskDispatcher();

    virtual bool add(task_type &&);

    virtual bool add(ITask::func_type fn) {
        return ITaskDispatcher::add(fn);
    }

    virtual void start();

    virtual void stop();

    virtual bool isrunning() const;

    virtual void clear();

    virtual void wait();

    virtual void cancel();
};

// 临界工作资源提供器
class NNT_API _ThreadResourceProvider
{
public:

    virtual ~_ThreadResourceProvider();

    // 开始提供
    void start();

    // 停止
    void stop();

protected:

    ::std::function<void*()> _init;
    ::std::function<void(void*)> _delete;
    void *_obj = nullptr;

private:
    
    semaphore _wait_start, _wait_stop;
    shared_ptr<::std::thread> _thd;

    static void _ThdWorker(_ThreadResourceProvider*);
};

template <typename T>
class ThreadResourceProvider : public _ThreadResourceProvider
{
public:

    ThreadResourceProvider(bool autostart = true)
    {
        _init = []()->void* {
            return new T();
        };

        _delete = [](void* p) {
            delete (T*)p;
        };

        if (autostart)
            start();
    }

    inline T& resource() {
        return *(T*)_obj;
    }

    inline T const& resource() const {
        return *(T*)_obj;
    }
    
    inline T* operator -> () {
        return (T*)_obj;
    }

    inline T const* operator -> () const {
        return (T*)_obj;
    }
};

CROSS_END

#endif
