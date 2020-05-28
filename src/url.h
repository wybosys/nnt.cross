#ifndef __NNTCROSS_URL_H_INCLUDED
#define __NNTCROSS_URL_H_INCLUDED

#include "property.h"

CROSS_BEGIN

class Url
{
public:

    Url();
    Url(string const&);

    typedef map<string, Property> args_type;

    string protocol, host;
    vector<string> paths;
    args_type args;

    void clear();
    bool parse(string const&);

    string toString() const;

    inline operator string () const {
        return toString();
    }
};

typedef string(fn_url_encoder)(Property const&);

extern NNT_API string build_querystring(Url::args_type const&, fn_url_encoder = nullptr);

CROSS_END

#endif