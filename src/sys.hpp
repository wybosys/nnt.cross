#ifndef __CROSS_SYS_H_INCLUDED
#define __CROSS_SYS_H_INCLUDED

CROSS_BEGIN

typedef unsigned long long pid_t;

extern pid_t get_thread_id();

CROSS_END

#endif