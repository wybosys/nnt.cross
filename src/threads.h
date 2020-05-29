#ifndef __NNTCROSS_THREADS_H_INCLUDED
#define __NNTCROSS_THREADS_H_INCLUDED

#include <functional>

CROSS_BEGIN

// ҵ������߳�ÿ֡�ص�
extern NNT_API void MainThreadTick();

// ���������̴߳�ѭ��
extern NNT_API void MainThreadExec();

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
    virtual bool add(task_type const&) = 0;

    // ��ʼ����
    virtual void start() = 0;

    // ��������
    virtual void stop() = 0;

    // �Ƿ�������
    virtual bool isrunning() const = 0;

    // �������
    virtual void clear() = 0;

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

    virtual bool add(task_type const&);
    virtual void start();
    virtual void stop();
    virtual bool isrunning() const;
    virtual void clear();

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

    virtual bool add(task_type const&);
    virtual void start();
    virtual void stop();
    virtual bool isrunning() const;
    virtual void clear();
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

    virtual bool add(task_type const&);
    virtual void start();
    virtual void stop();
    virtual bool isrunning() const;
    virtual void clear();
};

CROSS_END

#endif