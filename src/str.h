#ifndef __NNTCROSS_STR_H_INCLUDED
#define __NNTCROSS_STR_H_INCLUDED

CROSS_BEGIN

extern NNT_API vector<string> explode(string const& str, string const& delimeter, bool skipempty = false);
extern NNT_API string implode(vector<string> const&, string const& delimeter);
extern NNT_API bool beginwith(string const& str, string const& tgt);
extern NNT_API bool endwith(string const& str, string const& tgt);

extern NNT_API int toInt(string const&);
extern NNT_API float toFloat(string const&);
extern NNT_API double toDouble(string const&);

CROSS_END

#endif