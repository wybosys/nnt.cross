#ifndef __NNTCROSS_URL_H_INCLUDED
#define __NNTCROSS_URL_H_INCLUDED

#include "property.hpp"

CROSS_BEGIN

class Url
{
public:
    Url();

    Url(string const &);

    typedef ::std::map<string, Property> args_type;

    string protocol, host;
    unsigned short port = 0; // 端口0代表没有设置
    ::std::vector<string> paths;
    args_type args;

    void clear();

    bool parse(string const &);

    string toString() const;

    inline operator string() const
    {
        return toString();
    }

    // 调用路径
    string path() const;

    // 调用url
    string url() const;
};

typedef string(fn_url_encoder)(Property const &);

extern NNT_API string build_querystring(Url::args_type const &, fn_url_encoder = nullptr);

CROSS_END

#ifdef NNT_OBJC

@interface NSString (cross)

- (NSString *)escape;
- (NSString *)unescape;

@end

#endif

#endif
