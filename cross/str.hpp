#ifndef __NNTCROSS_STR_H_INCLUDED
#define __NNTCROSS_STR_H_INCLUDED

#include <vector>

CROSS_BEGIN

// 分割字符串
extern NNT_API strings explode(string const &str, string const &delimeter, bool skipempty = false);

// 合并分割后的字符串
extern NNT_API string implode(strings const &, string const &delimeter);

// 字符串是否以目标字串为开头
extern NNT_API bool beginwith(string const &str, string const &tgt);

// 字符串是否以目标字串为结尾
extern NNT_API bool endwith(string const &str, string const &tgt);

// 转换为int
extern NNT_API int toint(string const &);

// 转换为float
extern NNT_API float tofloat(string const &);

// 转换为double
extern NNT_API double todouble(string const &);

// 转换为string
extern NNT_API string tostr(int);

extern NNT_API string tostr(unsigned int);

extern NNT_API string tostr(float, bool trim = true);

extern NNT_API string tostr(double, bool trim = true);

extern NNT_API string tostr(unsigned long long);

// 字符替换
extern NNT_API string replace(string const &, string const &pat, string const &tgt);

// 如果字符串为小数类型，则删除小数点后的无效0
extern NNT_API string trim_decimal_string(string const&);

CROSS_END

#endif
