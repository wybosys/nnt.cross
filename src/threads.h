#ifndef __NNTCROSS_THREADS_H_INCLUDED
#define __NNTCROSS_THREADS_H_INCLUDED

#include <functional>
#include <thread>
#include <mutex>

CROSS_BEGIN

// ҵ������߳�ÿ֡�ص�
extern NNT_API void MainThreadTick();

// ���������̴߳�ѭ��
extern NNT_API void MainThreadExec();

// �ź���
class semaphore
{
public:

    void notify() {
        lock_guard<decltype(_mtx)> lck(_mtx);
        ++_count;
        _cond.notify_one();
    }

    void wait() {
        unique_lock<decltype(_mtx)> lck(_mtx);
        while (!_count)
            _cond.wait(lck);
        --_count;
    }

    bool try_wait() {
        lock_guard<decltype(_mtx)> lck(_mtx);
        if (_count) {
            --_count;
            return true;
        }
        return false;
    }

private:
    mutex _mtx;
    condition_variable _cond;
    size_t _count = 0;
};

// ����ӿ�
class ITask : public IObject
{
    NNT_NOCOPY(ITask);

public:

    typedef function<void(ITask*)> func_type;
    typedef shared_ptr<ITask> task_type;

    ITask(func_type fn = nullptr);

    // �̳з�ʽʵ������
    virtual void main() {}
    
    // �����趨���к���
    func_type proc;

    // �Ƿ�������
    bool isrunning() const;

    // �����ĵ�����
    class ITaskDispatcher *dispatcher() const;

    // ����һ������
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

// �ӿ��̵߳�����
class ITaskDispatcher : public IObject
{
public:

    typedef ITask::task_type task_type;
    
    // ���һ������
    virtual bool add(task_type&&) = 0;

    // ��ʼ����
    virtual void start() = 0;

    // ��������
    virtual void stop() = 0;

    // �Ƿ�������
    virtual bool isrunning() const = 0;

    // �������
    virtual void clear() = 0;

    // �ȴ����������������
    virtual void wait() = 0;

    // ȡ����������
    virtual void cancel() = 0;

protected:

    // ִ��һ������
    void _run(task_type& tsk);
};

NNT_CLASS_PREPARE(SingleTaskDispatcher);

// ���߳��������
class SingleTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(SingleTaskDispatcher);

public:

    SingleTaskDispatcher();
    virtual ~SingleTaskDispatcher();

    virtual bool add(task_type&&);
    virtual void start();
    virtual void stop();
    virtual bool isrunning() const;
    virtual void clear();
    virtual void wait();
    virtual void cancel();

    // ���ӵ���ǰ�����߳�
    bool attach();
};

NNT_CLASS_PREPARE(FixedTaskDispatcher);

// �����߳��������
class FixedTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(FixedTaskDispatcher);

public:

    // \@count ʹ�ö��ٸ��߳�ִ������
    FixedTaskDispatcher(size_t count);
    virtual ~FixedTaskDispatcher();

    virtual bool add(task_type&&);
    virtual void start();
    virtual void stop();
    virtual bool isrunning() const;
    virtual void clear();
    virtual void wait();
    virtual void cancel();
};

NNT_CLASS_PREPARE(QueuedTaskDispatcher);

// �����߳��������
class QueuedTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(QueuedTaskDispatcher);

public:

    /*
    \@min ��С���ٸ��߳�
    \@max ����max�̺߳����������Ŷӵȴ�ִ��
    */
    QueuedTaskDispatcher(size_t min, size_t max);
    virtual ~QueuedTaskDispatcher();

    virtual bool add(task_type&&);
    virtual void start();
    virtual void stop();
    virtual bool isrunning() const;
    virtual void clear();
    virtual void wait();
    virtual void cancel();
};

CROSS_END

#endif