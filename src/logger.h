#ifndef __NNT_CROSS_LOGGER_H_INCLUDED
#define __NNT_CROSS_LOGGER_H_INCLUDED

CROSS_BEGIN

enum struct LogLevel
{
    SPECIAL = 9,
    CUSTOM = 8,
    DEBUG = 7,
    INFO = 6,
    NOTICE = 5,
    WARNING = 4,
    FATAL = 3,
    ALERT = 2,
    CRITICAL = 1,
    EMERGENCY = 0
};

class NNT_API Logger
{
public:

    virtual ~Logger() = default;

    string prefix;

    virtual void debug(string const&);
    virtual void info(string const&);
    virtual void notice(string const&);
    virtual void warn(string const&);
    virtual void error(string const&);
    virtual void alert(string const&);
    virtual void critical(string const&);
    virtual void emergency(string const&);

    virtual void log(LogLevel, string const&);
};

CROSS_END

#endif