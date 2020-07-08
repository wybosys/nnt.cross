#include "cross.hpp"
#include "digest.hpp"

#if !defined(NNT_DARWIN)
#include <md5/md5.h>
#endif

CROSS_BEGIN

#if !defined(NNT_DARWIN)

md5_result md5(string const &str)
{
    md5_result r = {0};
    md5_state_t ctx;
    md5_init(&ctx);
    md5_append(&ctx, (unsigned char *)str.c_str(), str.length());
    md5_finish(&ctx, r.hex);
    return r;
}

#endif

string md5_hex2str(md5_result const &r)
{
    char result[33] = {0};
    for (size_t i = 0; i < 16; i++)
    {
#ifdef NNT_WINDOWS
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

        // 不可能越界，所以关闭编译器的警告
        sprintf(result + 2 * i, "%02x", r.hex[i]);

#ifdef NNT_WINDOWS
#pragma warning(pop)
#endif
    }
    return result;
}

string md5str(string const &str)
{
    return md5_hex2str(md5(str));
}

CROSS_END
