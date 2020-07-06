#include "ajni++.hpp"
#define __AJNI_PRIVATE__
#include "jnienv.hpp"
#include "ast.hpp"
#include "java-prv.hpp"
#include "android-prv.hpp"
#include "jre.hpp"

#include <cross/cross.hpp>
#include <cross/str.hpp>
#include <cross/sys.hpp>

AJNI_BEGIN

bool gs_during_init = false; // 标记当前正位于初始化流程中，避免 JEnvThreadAutoGuard 自动绑定
JavaVM* gs_vm = nullptr; // jni绑定主jvm对象
jobject gs_activity = nullptr; // 业务层主activity对象
jobject gs_context = nullptr; // 业务层android上下文

// 所有线程的AutoGuard资源
static ::std::mutex gsmtx_tlses;

static ::std::set<JEnvThreadAutoGuard*> gs_tlses;

JEnvThreadAutoGuard::JEnvThreadAutoGuard()
{
    gs_tlses.insert(this);

    // 自动绑定Env
    if (!gs_vm || gs_during_init)
    {
        // 整个环境还没有初始化，并且会之后由BindVM操作初始化，此处直接返回
        return;
    }

    bind();
}

JEnvThreadAutoGuard::~JEnvThreadAutoGuard()
{
    NNT_AUTOGUARD(gsmtx_tlses);
    gs_tlses.erase(this);

    // 线程结束时，自动释放
    free();
}

void JEnvThreadAutoGuard::bind()
{
    auto tids = ::CROSS_NS::tostr(tid);

    // 绑定Env环境
    if (Env.GetCurrentJniEnv)
    {
        env = Env.GetCurrentJniEnv();
        if (env)
        {
            Logger::Info("线程" + tids + ": 获得业务定义的线程级JNIEnv");
            return;
        }
        else
        {
            Logger::Critical("线程" + tids + ": 获得业务定义的线程级JNIEnv 失败");
            return;
        }
    }

    // 使用内置的创建函数创建
    jint ret = gs_vm->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (ret == JNI_EDETACHED)
    {
        gs_vm->AttachCurrentThread(&env, nullptr);
        detach = true;
    }

    Logger::Info("线程" + tids + ": 获得线程级JNIEnv");
}

void JEnvThreadAutoGuard::Clear()
{
    NNT_AUTOGUARD(gsmtx_tlses);
    gs_tlses.clear();
}

void JEnvThreadAutoGuard::free()
{
    if (env && detach)
    {
        gs_vm->DetachCurrentThread();
        detach = false;
    }

    env = nullptr;
    errmsg.clear();

    auto tids = ::CROSS_NS::tostr(tid);
    Logger::Info("线程" + tids + ": 释放线程级JNIEnv资源");
}

void JEnvThreadAutoGuard::check()
{
    if (env)
        return;

    string tids = ::CROSS_NS::tostr(tid);

    if (Env.GetCurrentJniEnv)
    {
        env = Env.GetCurrentJniEnv();
        if (env)
        {
            Logger::Info("线程" + tids + ": 获得业务定义的线程级JNIEnv");
            return;
        }
    }

    // 使用内置的创建函数创建
    jint ret = gs_vm->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (ret == JNI_EDETACHED)
    {
        gs_vm->AttachCurrentThread(&env, nullptr);
        detach = true;
    }

    Logger::Info("线程" + tids + ": 获得线程级JNIEnv");
}

shared_ptr<JVariant> JObject::Extract(jobject _obj)
{
    if (_obj == nullptr)
    {
        return ::std::make_shared<JVariant>(); // 不能返回null，客户端收到的是引用类型，通过vt判断
    }

    auto obj = make_shared<JObject>();
    obj->_reset(_obj);
    Env.DeleteLocalRef(_obj);

    // 从具体的java类型中获取数据
    auto& ctx = Env.context();

    // 判断是否是number数字对象
    auto STD_NUMBER = ctx.register_class<jre::Number>();
    if (Env.IsInstanceOf(*obj, *STD_NUMBER))
    {
        auto STD_DOUBLE = ctx.register_class<jre::Double>();
        if (Env.IsInstanceOf(*obj, *STD_DOUBLE))
        {
            JEntry<jre::Double> ref(obj);
            return ref->doubleValue(ref);
        }

        auto STD_FLOAT = ctx.register_class<jre::Float>();
        if (Env.IsInstanceOf(*obj, *STD_FLOAT))
        {
            JEntry<jre::Float> ref(obj);
            return ref->floatValue(ref);
        }

        JEntry<jre::Number> ref(obj);
        return ref->longValue(ref);
    }

    // 判断是否是boolean对象
    auto STD_BOOLEAN = ctx.register_class<jre::Boolean>();
    if (Env.IsInstanceOf(*obj, *STD_BOOLEAN))
    {
        JEntry<jre::Boolean> ref(obj);
        return ref->booleanValue(ref);
    }

    // 判断是否是string对象
    auto STD_STRING = ctx.register_class<jre::String>();
    if (Env.IsInstanceOf(*obj, *STD_STRING))
    {
        JEntry<jre::String> ref(obj);
        return ref->getBytes(ref);
    }

    // 自定义类型
    return _V(_obj);
}

shared_ptr<JObject> JObject::Putin(shared_ptr<JVariant> const& var)
{
    if (!var || var->isnil())
        return nullptr;

    auto& ctx = Env.context();

    switch (var->vt)
    {
    case JVariant::VT::OBJECT:
        return var->toObject();
    case JVariant::VT::BOOLEAN:
    {
        auto STD_BOOLEAN = ctx.register_class<jre::Boolean>();
        return STD_BOOLEAN->construct(var->toBool())->toObject();
    }
    case JVariant::VT::INTEGER:
    {
        auto STD_INTEGER = ctx.register_class<jre::Integer>();
        return STD_INTEGER->construct(var->toInteger())->toObject();
    }
    case JVariant::VT::NUMBER:
    {
        auto STD_DOUBLE = ctx.register_class<jre::Double>();
        return STD_DOUBLE->construct(var->toNumber())->toObject();
    }
    case JVariant::VT::STRING:
    {
        auto STD_STRING = ctx.register_class<jre::String>();
        return STD_STRING->construct(var->toString())->toObject();
    }
        break;
    default:
        break;
    }
    Logger::Error("无法将类型 " + ::CROSS_NS::tostr((int)var->vt) + " 转换成为JObject");
    return nullptr;
}

# define JOBJECT_IMPL_EXTRACT_ARR \
size_t sz = Env.GetArrayLength(arr); \
auto o = make_shared<JArray>(); \
o->_reset(arr, sz); \
return make_shared<JVariant>(o);

shared_ptr<JVariant> JObject::Extract(jobjectArray arr)
{
    JOBJECT_IMPL_EXTRACT_ARR;
}

shared_ptr<JVariant> JObject::Extract(jbooleanArray arr)
{
    JOBJECT_IMPL_EXTRACT_ARR;
}

shared_ptr<JVariant> JObject::Extract(jbyteArray arr)
{
    JOBJECT_IMPL_EXTRACT_ARR;
}

shared_ptr<JVariant> JObject::Extract(jcharArray arr)
{
    JOBJECT_IMPL_EXTRACT_ARR;
}

shared_ptr<JVariant> JObject::Extract(jshortArray arr)
{
    JOBJECT_IMPL_EXTRACT_ARR;
}

shared_ptr<JVariant> JObject::Extract(jintArray arr)
{
    JOBJECT_IMPL_EXTRACT_ARR;
}

shared_ptr<JVariant> JObject::Extract(jlongArray arr)
{
    JOBJECT_IMPL_EXTRACT_ARR;
}

shared_ptr<JVariant> JObject::Extract(jfloatArray arr)
{
    JOBJECT_IMPL_EXTRACT_ARR;
}

shared_ptr<JVariant> JObject::Extract(jdoubleArray arr)
{
    JOBJECT_IMPL_EXTRACT_ARR;
}

namespace TypeSignature
{

// 简化switch写法的工具函数
TS GetTypeForSwitch(JTypeSignature const& ts)
{
    static ::std::map<string, TS> gs_types = {
        { CLASS, TS::CLASS },
        { STRING, TS::STRING },
        { OBJECT, TS::OBJECT },
        { BOOLEAN, TS::BOOLEAN },
        { BYTE, TS::BYTE },
        { CHAR, TS::CHAR },
        { SHORT, TS::SHORT },
        { INT, TS::INT },
        { LONG, TS::LONG },
        { FLOAT, TS::FLOAT },
        { DOUBLE, TS::DOUBLE },
        { VOID, TS::VOID },
        { BYTEARRAY, TS::BYTEARRAY }
    };
    auto fnd = gs_types.find(ts);
    return fnd == gs_types.end() ? TS::UNKNOWN : fnd->second;
}
}

AJNI_END
