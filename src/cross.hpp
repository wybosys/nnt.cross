#ifndef __NNTCROSS_H_INCLUDED
#define __NNTCROSS_H_INCLUDED

#include "macro.hpp"

#ifndef CROSS_NS
#define CROSS_NS cross
#endif

#define CROSS_BEGIN namespace CROSS_NS {
#define CROSS_END }
#define USE_CROSS using namespace CROSS_NS;

CROSS_BEGIN

using ::std::string;
using ::NNT_NS::strings;
using ::std::shared_ptr;
using ::std::make_shared;
using ::std::cout;
using ::std::endl;
using ::std::cerr;

template <typename V>
struct Range
{
    V from, size;
};

template <typename V>
struct Progress : Range<V>
{
    V value;
};

template <typename MemT, typename V>
struct Memory : Range <V>
{
    Memory() {}
    Memory(MemT v) : buffer(v) {}
    MemT buffer;
};

CROSS_END

#endif