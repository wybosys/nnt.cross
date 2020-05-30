#ifndef __NNTCROSS_DIGEST_H_INCLUDED
#define __NNTCROSS_DIGEST_H_INCLUDED

CROSS_BEGIN

struct md5_result {
    unsigned char hex[16];
};

extern NNT_API md5_result md5(string const&);
extern NNT_API string md5_hex2str(md5_result const&);
extern NNT_API string md5str(string const&);

CROSS_END

#endif