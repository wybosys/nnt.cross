#import "cross.hpp"
#import "logger.hpp"

CROSS_BEGIN

void Logger::log(LogLevel lv, string const& msg)
{
    auto str = format(lv, msg);
    switch (lv)
    {
        case LogLevel::DEVELOP:
        {
            NNT_DEBUG_EXPRESS(
                              NSLog(@"[DEBUG] %s", str.c_str());
                              );
        }
            break;
        case LogLevel::SPECIAL:
        case LogLevel::CUSTOM:
        case LogLevel::INFO:
        case LogLevel::NOTICE:
        {
            NSLog(@"[INFO] %s", str.c_str());
        }
            break;
        case LogLevel::WARNING:
        {
            NSLog(@"[WARN] %s", str.c_str());
        }
            break;
        case LogLevel::FATAL:
        case LogLevel::ALERT:
        case LogLevel::CRITICAL:
        case LogLevel::EMERGENCY:
        {
            NSLog(@"[ERROR] %s", str.c_str());
        }
            break;
    }
}

CROSS_END
