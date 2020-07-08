#import "cross.hpp"
#import "digest.hpp"
#import <CommonCrypto/CommonDigest.h>

CROSS_BEGIN

md5_result md5(string const &str)
{
    md5_result r = {0};
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    
    CC_MD5(str.c_str(), (CC_LONG)str.length(), r.hex);
    
#pragma clang diagnostic pop
    return r;
}

CROSS_END
