#import "cross.hpp"
#import "fs.hpp"
#import "objc.hpp"
#import "logger.hpp"

CROSS_BEGIN

bool mv(string const& from, string const& to)
{
    NSError *err;
    [NSFileManager.defaultManager moveItemAtPath:toOc(from)
                                          toPath:toOc(to)
                                           error:&err];
    if (err) {
        Logger::Error(fromOc(err.description));
        return false;
    }
    return true;
}

CROSS_END
