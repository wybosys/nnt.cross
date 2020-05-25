#ifndef __NNTCROSS_STR_H_INCLUDED
#define __NNTCROSS_STR_H_INCLUDED

CROSS_BEGIN

extern vector<string> explode(string const& str, string const& delimeter, bool skipempty = false);
extern string implode(vector<string> const&, string const& delimeter);
extern bool beginwith(string const& str, string const& tgt);
extern bool endwith(string const& str, string const& tgt);

extern int toInt(string const&);
extern float toFloat(string const&);
extern double toDouble(string const&);

CROSS_END

#endif