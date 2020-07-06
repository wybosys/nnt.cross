#ifndef __AJNIXX_H_INCLUDED
#define __AJNIXX_H_INCLUDED

#include "macro.hpp"

#include <jni.h>
#include <android/log.h>

#ifndef AJNI_NS
#define AJNI_NS ajni
#endif

#define AJNI_BEGIN    \
    namespace AJNI_NS \
    {
#define AJNI_END }
#define USE_AJNI using namespace AJNI_NS;

#define AJNI_BEGIN_NS(ns)   \
    AJNI_BEGIN namespace ns \
    {
#define AJNI_END_NS \
    }               \
    AJNI_END

#define _AJNI_LOG_IDR "log@ajni++"
#define AJNI_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, _AJNI_LOG_IDR, __VA_ARGS__)
#define AJNI_LOGI(...) __android_log_print(ANDROID_LOG_INFO, _AJNI_LOG_IDR, __VA_ARGS__)
#define AJNI_LOGW(...) __android_log_print(ANDROID_LOG_WARN, _AJNI_LOG_IDR, __VA_ARGS__)
#define AJNI_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, _AJNI_LOG_IDR, __VA_ARGS__)
#define AJNI_LOGF(...) __android_log_print(ANDROID_LOG_FATAL, _AJNI_LOG_IDR, __VA_ARGS__)

#include "jnienv.hpp"
#include "ast.hpp"
#include "jre.hpp"
#include "kotlin.hpp"
#include "android.hpp"

#endif
