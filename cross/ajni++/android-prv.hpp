#ifndef __AJNI_ANDROID_PRIVATE_H_INCLUDED
#define __AJNI_ANDROID_PRIVATE_H_INCLUDED

#ifndef __AJNI_PRIVATE__
#error "禁止在外部引用该文件"
#endif

#include <cross/cross.hpp>
#include <cross/logger.hpp>

AJNI_BEGIN

// AJNI日志接口
class Logger : public ::CROSS_NS::Logger
{
public:

    Logger();

    virtual void log(::CROSS_NS::LogLevel, string const&) override;

};

AJNI_END

extern void AJNI_LOGGER_ERROR(::std::string const&);

#endif
