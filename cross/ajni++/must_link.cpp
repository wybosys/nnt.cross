#include <ajni++/ajni++.hpp>

USE_AJNI;

extern void AJNI_LOGGER_ERROR(::std::string const&);

AJNI_API(void) AJNI_COMPANION_FUNC(Activity, jni_1bind)(JNIEnv* env, jobject thiz, jobject act,
    jobject ctx)
{
    Env.BindContext(act, ctx);
}

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1grab)(JNIEnv* env, jobject thiz, jlong fnidx)
{
    Env.context().function_grab(fnidx);
    return nullptr;
}

AJNI_API(jboolean) AJNI_FUNC(Callback, jni_1drop)(JNIEnv* env, jobject thiz, jlong fnidx)
{
    return Env.context().function_drop(fnidx);
}

// Env.Check(); 不需要增加保护，线程初始化时会自动调用 (JEnvThreadAutoGuard)
#define _AJNI_CALLBACK_IMPL_BEGIN \
    auto &ctx = Env.context(); \
    auto fn = ctx.find_function(fnidx); \
    if (!fn) { \
        AJNI_LOGGER_ERROR("没有找到回调函数"); \
        return 0; \
    }

#define _AJNI_CALLBACK_IMPL_END return 0;
// ctx.function_drop(fnidx); value释放时，会自动调用drop

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1invoke)(JNIEnv* env, jobject thiz, jlong fnidx)
{
    _AJNI_CALLBACK_IMPL_BEGIN
    (*fn)();
    _AJNI_CALLBACK_IMPL_END
}

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1invoke1)(JNIEnv* env, jobject thiz, jlong fnidx,
    jobject v0)
{
    _AJNI_CALLBACK_IMPL_BEGIN
    (*fn)(*JObject::Extract(v0));
    _AJNI_CALLBACK_IMPL_END
}

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1invoke2)(JNIEnv* env, jobject thiz, jlong fnidx,
    jobject v0,
    jobject v1)
{
    _AJNI_CALLBACK_IMPL_BEGIN
    (*fn)(*JObject::Extract(v0), *JObject::Extract(v1));
    _AJNI_CALLBACK_IMPL_END
}

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1invoke3)(JNIEnv* env, jobject thiz, jlong fnidx,
    jobject v0,
    jobject v1, jobject v2)
{
    _AJNI_CALLBACK_IMPL_BEGIN
    (*fn)(*JObject::Extract(v0), *JObject::Extract(v1), *JObject::Extract(v2));
    _AJNI_CALLBACK_IMPL_END
}

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1invoke4)(JNIEnv* env, jobject thiz, jlong fnidx,
    jobject v0,
    jobject v1, jobject v2, jobject v3)
{
    _AJNI_CALLBACK_IMPL_BEGIN
    (*fn)(*JObject::Extract(v0), *JObject::Extract(v1), *JObject::Extract(v2),
        *JObject::Extract(v3));
    _AJNI_CALLBACK_IMPL_END
}

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1invoke5)(JNIEnv* env, jobject thiz, jlong fnidx,
    jobject v0,
    jobject v1, jobject v2, jobject v3,
    jobject v4)
{
    _AJNI_CALLBACK_IMPL_BEGIN
    (*fn)(*JObject::Extract(v0), *JObject::Extract(v1), *JObject::Extract(v2),
        *JObject::Extract(v3), *JObject::Extract(v4));
    _AJNI_CALLBACK_IMPL_END
}

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1invoke6)(JNIEnv* env, jobject thiz, jlong fnidx,
    jobject v0,
    jobject v1, jobject v2, jobject v3, jobject v4,
    jobject v5)
{
    _AJNI_CALLBACK_IMPL_BEGIN
    (*fn)(*JObject::Extract(v0), *JObject::Extract(v1), *JObject::Extract(v2),
        *JObject::Extract(v3), *JObject::Extract(v4), *JObject::Extract(v5));
    _AJNI_CALLBACK_IMPL_END
}

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1invoke7)(JNIEnv* env, jobject thiz, jlong fnidx,
    jobject v0,
    jobject v1, jobject v2, jobject v3, jobject v4,
    jobject v5, jobject v6)
{
    _AJNI_CALLBACK_IMPL_BEGIN
    (*fn)(*JObject::Extract(v0), *JObject::Extract(v1), *JObject::Extract(v2),
        *JObject::Extract(v3), *JObject::Extract(v4), *JObject::Extract(v5),
        *JObject::Extract(v6));
    _AJNI_CALLBACK_IMPL_END
}

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1invoke8)(JNIEnv* env, jobject thiz, jlong fnidx,
    jobject v0,
    jobject v1, jobject v2, jobject v3, jobject v4,
    jobject v5, jobject v6, jobject v7)
{
    _AJNI_CALLBACK_IMPL_BEGIN
    (*fn)(*JObject::Extract(v0), *JObject::Extract(v1), *JObject::Extract(v2),
        *JObject::Extract(v3), *JObject::Extract(v4), *JObject::Extract(v5),
        *JObject::Extract(v6), *JObject::Extract(v7));
    _AJNI_CALLBACK_IMPL_END
}

AJNI_API(jobject) AJNI_FUNC(Callback, jni_1invoke9)(JNIEnv* env, jobject thiz, jlong fnidx,
    jobject v0,
    jobject v1, jobject v2, jobject v3, jobject v4,
    jobject v5, jobject v6, jobject v7,
    jobject v8)
{
    _AJNI_CALLBACK_IMPL_BEGIN
    (*fn)(*JObject::Extract(v0), *JObject::Extract(v1), *JObject::Extract(v2),
        *JObject::Extract(v3), *JObject::Extract(v4), *JObject::Extract(v5),
        *JObject::Extract(v6), *JObject::Extract(v7), *JObject::Extract(v8));
    _AJNI_CALLBACK_IMPL_END
}
