#ifndef __NNT_CROSS_TIME_H_INCLUDED
#define __NNT_CROSS_TIME_H_INCLUDED

CROSS_BEGIN

typedef unsigned long long timestamp_t;
typedef double seconds_t;

class NNT_API Time
{
public:

    static timestamp_t Current();
    static seconds_t Now();

};

CROSS_END

#endif