#ifndef __NNT_CROSS_DATETIME_H_INCLUDED
#define __NNT_CROSS_DATETIME_H_INCLUDED

CROSS_BEGIN

typedef unsigned long long timestamp_t;
typedef double seconds_t;

class NNT_API Time
{
public:

    static timestamp_t Current();
    static seconds_t Now();
    static void Sleep(seconds_t);

};

CROSS_END

#endif