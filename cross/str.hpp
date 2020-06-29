#ifndef __NNTCROSS_STR_H_INCLUDED
#define __NNTCROSS_STR_H_INCLUDED

#include <vector>

CROSS_BEGIN

extern NNT_API strings explode(string const &str, string const &delimeter, bool skipempty = false);

extern NNT_API string implode(strings const &, string const &delimeter);

extern NNT_API bool beginwith(string const &str, string const &tgt);

extern NNT_API bool endwith(string const &str, string const &tgt);

extern NNT_API int toint(string const &);

extern NNT_API float tofloat(string const &);

extern NNT_API double todouble(string const &);

extern NNT_API string tostr(int);

extern NNT_API string tostr(unsigned int);

extern NNT_API string tostr(float);

extern NNT_API string tostr(double);

extern NNT_API string tostr(unsigned long long);

extern NNT_API string replace(string const &, string const &pat, string const &tgt);

CROSS_END

#endif
