#ifndef __CROSS_OBJC_H_INCLUDED
#define __CROSS_OBJC_H_INCLUDED

#include "memory.hpp"
#include "property.hpp"
#include <map>

CROSS_BEGIN

#ifdef NNT_OBJC

extern NSString *toOc(string const&);

extern string fromOc(NSString *);

extern NSDictionary *toOc(::std::map<string, shared_ptr<Property> > const&);

extern void fromOc(NSDictionary*, ::std::map<string, shared_ptr<Property> > &);

#endif

CROSS_END

#endif
