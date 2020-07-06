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

#include "jnienv.hpp"
#include "ast.hpp"
#include "jre.hpp"
#include "kotlin.hpp"
#include "android.hpp"

#endif
