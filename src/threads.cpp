#include "cross.h"
#include "threads.h"
//#include "time.h"

CROSS_BEGIN

void MainThreadTick() {

}

void MainThreadExec() {
    
}

Task::Task(func_type fn)
    :proc(move(fn))
{
}


Task::~Task() 
{
}

class SingleTaskDispatcherPrivate 
{
public:

};

SingleTaskDispatcher::SingleTaskDispatcher()
{
    NNT_CLASS_CONSTRUCT();
}

SingleTaskDispatcher::~SingleTaskDispatcher()
{
    NNT_CLASS_DESTORY();
}

class FixedTaskDispatcherPrivate
{
public:

};

FixedTaskDispatcher::FixedTaskDispatcher()
{
    NNT_CLASS_CONSTRUCT();
}

FixedTaskDispatcher::~FixedTaskDispatcher()
{
    NNT_CLASS_DESTORY();
}

class QueuedTaskDispatcherPrivate
{
public:

};

QueuedTaskDispatcher::QueuedTaskDispatcher()
{
    NNT_CLASS_CONSTRUCT();
}

QueuedTaskDispatcher::~QueuedTaskDispatcher()
{
    NNT_CLASS_DESTORY();
}

CROSS_END