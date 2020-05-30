#ifndef __NNTCROSS_STR_H_INCLUDED
#define __NNTCROSS_STR_H_INCLUDED

CROSS_BEGIN

extern NNT_API vector<string> explode(string const& str, string const& delimeter, bool skipempty = false);
extern NNT_API string implode(vector<string> const&, string const& delimeter);
extern NNT_API bool beginwith(string const& str, string const& tgt);
extern NNT_API bool endwith(string const& str, string const& tgt);

extern NNT_API int toint(string const&);
extern NNT_API float tofloat(string const&);
extern NNT_API double todouble(string const&);

extern NNT_API string tostr(int);
extern NNT_API string tostr(float);
extern NNT_API string tostr(double);

CROSS_END

#endif