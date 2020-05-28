#ifndef __NNTCROSS_URL_H_INCLUDED
#define __NNTCROSS_URL_H_INCLUDED

#include <map>

CROSS_BEGIN

class Url
{
public:

    typedef map<string, string> args_type;

    string domain;
    string protocol;
    args_type args;
};

extern NNT_API string url_encode(string const&);
extern NNT_API string url_decode(string const&);

CROSS_END

#endif