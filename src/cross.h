#ifndef __NNTCROSS_H_INCLUDED
#define __NNTCROSS_H_INCLUDED

#include "macro.h"

#ifndef CROSS_NS
#define CROSS_NS cross
#endif

#define CROSS_BEGIN NNT_BEGIN_NS(CROSS_NS)
#define CROSS_END NNT_END_NS()
#define USE_CROSS USE_NNT_NS(CROSS_NS)

CROSS_BEGIN

template <typename V>
struct Range
{
    V from, to;
};

template <typename V>
struct Progress : Range<V>
{
    V value;
};

CROSS_END

#endif