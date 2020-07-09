#ifndef __CROSS_SYS_H_INCLUDED
#define __CROSS_SYS_H_INCLUDED

CROSS_BEGIN

typedef unsigned long long pid_t, tid_t;

// 获得当前进程id
extern pid_t get_process_id();

// 获得当前线程id
extern tid_t get_thread_id();

// 获得线程名称
extern string get_thread_name();

// 设置线程名称
extern void set_thread_name(string const&);

// 生成UUID字符串，不带-的形式
extern NNT_API string uuid();

CROSS_END

#endif
