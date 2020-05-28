#ifndef __NNTCROSS_THREADS_H_INCLUDED
#define __NNTCROSS_THREADS_H_INCLUDED

CROSS_BEGIN

// 业务的主线程每帧回调
extern NNT_API void MainThreadTick();

// 用于做主线程大循环
extern NNT_API void MainThreadExec();

CROSS_END

#endif