#import "cross.hpp"
#import "url.hpp"

@implementation NSString (cross)

- (NSString *)escape
{
    return [self stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLQueryAllowedCharacterSet]];
}

- (NSString *)unescape
{
    return self.stringByRemovingPercentEncoding;
}

@end
