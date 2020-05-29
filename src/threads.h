#ifndef __NNTCROSS_THREADS_H_INCLUDED
#define __NNTCROSS_THREADS_H_INCLUDED

#include <functional>

CROSS_BEGIN

// ҵ������߳�ÿ֡�ص�
extern NNT_API void MainThreadTick();

// ���������̴߳�ѭ��
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

// �ӿ��̵߳�����
class ITaskDispatcher : public IObject
{
public:

    typedef shared_ptr<Task> task_type;

    virtual void add(task_type const&) = 0;
};

NNT_CLASS_PREPARE(SingleTaskDispatcher);

// ���߳��������
class SingleTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(SingleTaskDispatcher);

public:

    SingleTaskDispatcher();
    virtual ~SingleTaskDispatcher();

};

NNT_CLASS_PREPARE(FixedTaskDispatcher);

// �����߳��������
class FixedTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(FixedTaskDispatcher);

public:

    FixedTaskDispatcher();
    virtual ~FixedTaskDispatcher();

};

NNT_CLASS_PREPARE(QueuedTaskDispatcher);

// �����߳��������
class QueuedTaskDispatcher : public ITaskDispatcher
{
    NNT_CLASS_DECL(QueuedTaskDispatcher);

public:

    QueuedTaskDispatcher();
    virtual ~QueuedTaskDispatcher();

};

CROSS_END

#endif