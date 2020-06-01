#ifndef __NNTCROSS_ZIP_H_INCLUDED
#define __NNTCROSS_ZIP_H_INCLUDED

CROSS_BEGIN

// 加压到目录中，如果已经存在，则默认为覆盖，如果无法覆盖，则返回错误
extern bool unzip(string const& ar, string const& dir);

CROSS_END

#endif