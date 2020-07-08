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
                          NSLog(@"%s", str.c_str());
                          );
    }
        break;
    case LogLevel::SPECIAL:
    case LogLevel::CUSTOM:
    case LogLevel::INFO:
    case LogLevel::NOTICE:
    {
        NSLog(@"%s", str.c_str());
    }
        break;
    case LogLevel::WARNING:
    case LogLevel::FATAL:
    case LogLevel::ALERT:
    case LogLevel::CRITICAL:
    case LogLevel::EMERGENCY:
    {
        NSLog(@"%s", str.c_str());
    }
        break;
    }
}

CROSS_END
