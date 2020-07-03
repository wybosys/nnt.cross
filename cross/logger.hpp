#ifndef __NNT_CROSS_LOGGER_H_INCLUDED
#define __NNT_CROSS_LOGGER_H_INCLUDED

#include "macro.hpp"

CROSS_BEGIN

enum struct LogLevel
{
    SPECIAL = 9,
    CUSTOM = 8,
    DEVELOP = 7,
    INFO = 6,
    NOTICE = 5,
    WARNING = 4,
    FATAL = 3,
    ALERT = 2,
    CRITICAL = 1,
    EMERGENCY = 0
};

class ILogger: public ::NNT_NS::IObject
{
public:
    virtual void debug(string const &) = 0;
    virtual void info(string const &) = 0;
    virtual void notice(string const &) = 0;
    virtual void warn(string const &) = 0;
    virtual void error(string const &) = 0;
    virtual void alert(string const &) = 0;
    virtual void critical(string const &) = 0;
    virtual void emergency(string const &) = 0;

    virtual void log(LogLevel, string const &) = 0;
};

class NNT_API Logger: public ILogger
{
NNT_SINGLETON_DECL(Logger);

public:

    Logger();

    string prefix;

    virtual void debug(string const &);
    virtual void info(string const &);
    virtual void notice(string const &);
    virtual void warn(string const &);
    virtual void error(string const &);
    virtual void alert(string const &);
    virtual void critical(string const &);
    virtual void emergency(string const &);

    virtual void log(LogLevel, string const &);
    virtual string format(LogLevel, string const &);

    // 静态的方便函数
#define _LOGGER_GO(func, imp) \
    static void func(string const &msg) { Logger::shared().imp(msg); }

    _LOGGER_GO(Debug, debug);
    _LOGGER_GO(Info, info);
    _LOGGER_GO(Notice, notice);
    _LOGGER_GO(Warn, warn);
    _LOGGER_GO(Error, error);
    _LOGGER_GO(Alert, alert);
    _LOGGER_GO(Critical, critical);
    _LOGGER_GO(Emergency, emergency);
};

CROSS_END

#endif