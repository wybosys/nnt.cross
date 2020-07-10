#include "ajni++.hpp"
#define __AJNI_PRIVATE__
#include "android-prv.hpp"

#define _AJNI_LOG_IDR "log@ajni++"
#define AJNI_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, _AJNI_LOG_IDR, __VA_ARGS__)
#define AJNI_LOGI(...) __android_log_print(ANDROID_LOG_INFO, _AJNI_LOG_IDR, __VA_ARGS__)
#define AJNI_LOGW(...) __android_log_print(ANDROID_LOG_WARN, _AJNI_LOG_IDR, __VA_ARGS__)
#define AJNI_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, _AJNI_LOG_IDR, __VA_ARGS__)
#define AJNI_LOGF(...) __android_log_print(ANDROID_LOG_FATAL, _AJNI_LOG_IDR, __VA_ARGS__)

AJNI_BEGIN

USE_CROSS;

Logger::Logger()
{
    prefix = "AJNI";
}

void Logger::log(::CROSS_NS::LogLevel lv, string const& msg)
{
    auto str = format(lv, msg);
    switch (lv)
    {
    case LogLevel::DEVELOP:
    {
        NNT_DEBUG_EXPRESS(
            AJNI_LOGD("%s", str.c_str());
        );
    }
        break;
    case LogLevel::SPECIAL:
    case LogLevel::CUSTOM:
    case LogLevel::INFO:
    {
        AJNI_LOGI("%s", str.c_str());
    }
        break;
    case LogLevel::NOTICE:
    {
        AJNI_LOGE("%s", str.c_str());
    }
        break;
    case LogLevel::WARNING:
    {
        AJNI_LOGW("%s", str.c_str());
    }
        break;
    case LogLevel::FATAL:
    case LogLevel::ALERT:
    case LogLevel::CRITICAL:
    case LogLevel::EMERGENCY:
    {
        AJNI_LOGF("%s", str.c_str());
    }
        break;
    }
}

AJNI_END

void AJNI_LOGGER_ERROR(::std::string const& msg)
{
    AJNI_NS::Logger::Error(msg);
}
