#include "ajni++.hpp"
#define __AJNI_PRIVATE__
#include "jnienv.hpp"
#include "ast.hpp"
#include "jre.hpp"
#include "variant.hpp"
#include "android.hpp"
#include "java-prv.hpp"
#include "android-prv.hpp"
#include <mutex>

#include <cross/cross.hpp>
#include <cross/str.hpp>

AJNI_BEGIN

// 全局唯一的Env
JEnv Env;

// 随着当前线程和设置回收jni
static thread_local JEnvThreadAutoGuard tls_guard;

// 当前线程的env
#define tls_env tls_guard.env

class JEnvPrivate
{
public:

    // 清理
    void clear()
    {
        // 清理ajni中构造的局部数据
        ctx.clear();

        // 清理全局obj
        if (clz_classloader)
        {
            Env.DeleteGlobalRef(clz_classloader);
            Env.DeleteGlobalRef(obj_classloader);
            clz_classloader = nullptr;
            obj_classloader = nullptr;
            mid_loadclass = nullptr;
        }
    }

    // ajni增加的数据上下文
    JContext ctx;

    // 用于跨线程查找类id
    jclass clz_classloader = nullptr;
    jobject obj_classloader = nullptr;
    jmethodID mid_loadclass = nullptr;

    // 其他线程中不能使用FindClass查找类型，并且也无法使用GetMethod等对GlobalClass查找其成员，所以需要使用classloder查找出local的class对象
    jclass safeFindClass(JClassPath const& cp) const
    {
        jstring str = tls_env->NewStringUTF(cp.c_str());
        auto clz = (jclass)tls_env->CallObjectMethod(obj_classloader, mid_loadclass, str);
        tls_env->DeleteLocalRef(str);
        return clz;
    }

    // 全局锁
    ::std::mutex mtx_global;
};

JEnv::JEnv()
{
    NNT_CLASS_CONSTRUCT();

    // 使用AJNI的Logger实现
    Logger::set_shared<AJNI_NS::Logger>();
}

JEnv::~JEnv()
{
    NNT_CLASS_DESTORY();
}

JContext& JEnv::context()
{
    return d_ptr->ctx;
}

void JEnv::BindVM(JavaVM* vm, JNIEnv* env)
{
    if (gs_vm)
    {
        Logger::Critical("AJNI++环境已经初始化");
        return;
    }

    gs_vm = vm;
    gs_during_init = true;

    Logger::Info("启动AJNI++环境");

    if (!env)
    {
        jint jret = vm->GetEnv((void**)&tls_guard.env, JNI_VERSION_1_4);
        if (jret == JNI_EDETACHED)
        {
            gs_vm->AttachCurrentThread(&tls_guard.env, nullptr);
            tls_guard.detach = false;
        }
    }
    else
    {
        tls_guard.env = env;
    }

    tls_guard.ismain = true;
    gs_during_init = false;
}

void JEnv::UnbindVM()
{
    // 清理所有
    JEnvThreadAutoGuard::Clear();

    // 清理当前资源
    gs_vm = nullptr;
    gs_activity = nullptr;
    gs_context = nullptr;

    d_ptr->clear();

    Logger::Info("释放AJNI++环境");
}

void JEnv::Check()
{
    tls_guard.check();
}

void JEnv::BindContext(jobject jact, jobject jctx)
{
    // 清理
    d_ptr->clear();

    // 绑定当前，有可能为置空
    gs_activity = jact;
    gs_context = jctx;

    if (gs_context)
    {
        // 绑定新的context, 必须获取到，否则整个ajni++运行失败
        jclass clz_context = tls_env->FindClass("android/content/Context");
        jmethodID mid_getclassloader = tls_env->GetMethodID(clz_context, "getClassLoader",
            "()Ljava/lang/ClassLoader;");
        jobject obj_classloader = tls_env->CallObjectMethod(gs_context, mid_getclassloader);
        jclass clz_classloader = tls_env->FindClass("java/lang/ClassLoader");
        jmethodID mid_loadclass = tls_env->GetMethodID(clz_classloader, "loadClass",
            "(Ljava/lang/String;)Ljava/lang/Class;");
        d_ptr->clz_classloader = (jclass)tls_env->NewGlobalRef(clz_classloader);
        d_ptr->obj_classloader = tls_env->NewGlobalRef(obj_classloader);
        d_ptr->mid_loadclass = mid_loadclass;
    }
}

void JEnv::lock()
{
    d_ptr->mtx_global.lock();
}

void JEnv::unlock()
{
    d_ptr->mtx_global.unlock();
}

JEnv::class_type JEnv::FindClass(string const& str)
{
    jclass clz;
    if (d_ptr->clz_classloader)
    {
        // 如果从其他逻辑而不是JNI回调调用，则大概率env==null，需要加以保护
        Check();
        clz = d_ptr->safeFindClass(str);
    }
    else if (tls_guard.ismain)
    {
        clz = tls_env->FindClass(str.c_str());
    }
    else
    {
        Logger::Critical("不能在线程中使用FindClass命令，请确认是否调用了 ajnix.Activity.Bind 函数");
        return nullptr;
    }

    if (!clz)
    {
        ExceptionClear();
        return nullptr;
    }

    auto r = make_shared<JClass>();
    r->_clazz._reset(clz);
    tls_env->DeleteLocalRef(clz);

    return r;
}

bool JEnv::IsAssignableFrom(JClass const& l, JClass const& r)
{
    return tls_env->IsAssignableFrom((jclass)l._clazz._obj, (jclass)r._clazz._obj);
}

bool JEnv::IsInstanceOf(JObject const& obj, JClass const& clz)
{
    return tls_env->IsInstanceOf(obj._obj, (jclass)clz._clazz._obj);
}

bool JEnv::IsSameObject(JObject const& l, JObject const& r)
{
    return tls_env->IsSameObject(l._obj, r._obj);
}

jfieldID JEnv::GetStaticFieldID(JClass const& cls, const string& name, const string& typ)
{
    return tls_env->GetStaticFieldID((jclass)cls._clazz._obj, name.c_str(), typ.c_str());
}

jboolean JEnv::GetStaticBooleanField(JClass const& cls, jfieldID id)
{
    return tls_env->GetStaticBooleanField((jclass)cls._clazz._obj, id);
}

jbyte JEnv::GetStaticByteField(JClass const& cls, jfieldID id)
{
    return tls_env->GetStaticByteField((jclass)cls._clazz._obj, id);
}

jobject JEnv::GetStaticObjectField(JClass const& cls, jfieldID id)
{
    return tls_env->GetStaticObjectField((jclass)cls._clazz._obj, id);
}

JEnv::string_type JEnv::GetStaticStringField(JClass const& cls, jfieldID id)
{
    auto s = (jstring)tls_env->GetStaticObjectField((jclass)cls._clazz._obj, id);
    if (!s)
        return nullptr;

    auto r = make_shared<JString>();
    r->_reset(s);
    tls_env->DeleteLocalRef(s);

    return r;
}

JEnv::array_type JEnv::GetStaticArrayField(JClass const& cls, jfieldID id)
{
    auto o = (jobjectArray)tls_env->GetStaticObjectField((jclass)cls._clazz._obj, id);
    if (!o)
        return nullptr;
    size_t sz = tls_env->GetArrayLength(o);

    auto r = make_shared<JArray>();
    r->_reset(o, sz);
    tls_env->DeleteLocalRef(o);

    return r;
}

jchar JEnv::GetStaticCharField(JClass const& cls, jfieldID id)
{
    return tls_env->GetStaticCharField((jclass)cls._clazz._obj, id);
}

jshort JEnv::GetStaticShortField(JClass const& cls, jfieldID id)
{
    return tls_env->GetStaticShortField((jclass)cls._clazz._obj, id);
}

jint JEnv::GetStaticIntField(JClass const& cls, jfieldID id)
{
    return tls_env->GetStaticIntField((jclass)cls._clazz._obj, id);
}

jlong JEnv::GetStaticLongField(JClass const& cls, jfieldID id)
{
    return tls_env->GetStaticLongField((jclass)cls._clazz._obj, id);
}

jfloat JEnv::GetStaticFloatField(JClass const& cls, jfieldID id)
{
    return tls_env->GetStaticFloatField((jclass)cls._clazz._obj, id);
}

jdouble JEnv::GetStaticDoubleField(JClass const& cls, jfieldID id)
{
    return tls_env->GetStaticDoubleField((jclass)cls._clazz._obj, id);
}

void JEnv::SetStaticObjectField(JClass const& cls, jfieldID id, JVariant const& v)
{
    JValue t(v);
    jvalue jv = t;
    tls_env->SetStaticObjectField((jclass)cls._clazz._obj, id, jv.l);
}

void JEnv::SetStaticBooleanField(JClass const& cls, jfieldID id, jboolean v)
{
    tls_env->SetStaticBooleanField((jclass)cls._clazz._obj, id, v);
}

void JEnv::SetStaticByteField(JClass const& cls, jfieldID id, jbyte v)
{
    tls_env->SetStaticByteField((jclass)cls._clazz._obj, id, v);
}

void JEnv::SetStaticCharField(JClass const& cls, jfieldID id, jchar v)
{
    tls_env->SetStaticCharField((jclass)cls._clazz._obj, id, v);
}

void JEnv::SetStaticShortField(JClass const& cls, jfieldID id, jshort v)
{
    tls_env->SetStaticShortField((jclass)cls._clazz._obj, id, v);
}

void JEnv::SetStaticIntField(JClass const& cls, jfieldID id, jint v)
{
    tls_env->SetStaticIntField((jclass)cls._clazz._obj, id, v);
}

void JEnv::SetStaticLongField(JClass const& cls, jfieldID id, jlong v)
{
    tls_env->SetStaticLongField((jclass)cls._clazz._obj, id, v);
}

void JEnv::SetStaticFloatField(JClass const& cls, jfieldID id, jfloat v)
{
    tls_env->SetStaticFloatField((jclass)cls._clazz._obj, id, v);
}

void JEnv::SetStaticDoubleField(JClass const& cls, jfieldID id, jdouble v)
{
    tls_env->SetStaticDoubleField((jclass)cls._clazz._obj, id, v);
}

jfieldID JEnv::GetFieldID(JClass const& cls, string const& name, string const& sig)
{
    return tls_env->GetFieldID((jclass)cls._clazz._obj, name.c_str(), sig.c_str());
}

JEnv::object_type JEnv::GetObjectField(JObject const& obj, jfieldID fid)
{
    auto jo = tls_env->GetObjectField(obj._obj, fid);
    if (!jo)
        return nullptr;

    auto r = make_shared<JObject>();
    r->_reset(jo);
    tls_env->DeleteLocalRef(jo);

    return r;
}

JEnv::string_type JEnv::GetStringField(JObject const& obj, jfieldID fid)
{
    auto jo = (jstring)tls_env->GetObjectField(obj._obj, fid);
    if (!jo)
        return nullptr;

    auto r = make_shared<JString>();
    r->_reset(jo);
    tls_env->DeleteLocalRef(jo);

    return r;
}

JEnv::array_type JEnv::GetArrayField(JObject const& obj, jfieldID fid)
{
    auto jo = (jobjectArray)tls_env->GetObjectField(obj._obj, fid);
    if (!jo)
        return nullptr;
    size_t sz = tls_env->GetArrayLength(jo);

    auto r = make_shared<JArray>();
    r->_reset(jo, sz);
    tls_env->DeleteLocalRef(jo);

    return r;
}

jboolean JEnv::GetBooleanField(JObject const& obj, jfieldID fid)
{
    return tls_env->GetBooleanField(obj._obj, fid);
}

jbyte JEnv::GetByteField(JObject const& obj, jfieldID fid)
{
    return tls_env->GetByteField(obj._obj, fid);
}

jchar JEnv::GetCharField(JObject const& obj, jfieldID fid)
{
    return tls_env->GetCharField(obj._obj, fid);
}

jshort JEnv::GetShortField(JObject const& obj, jfieldID fid)
{
    return tls_env->GetShortField(obj._obj, fid);
}

jint JEnv::GetIntField(JObject const& obj, jfieldID fid)
{
    return tls_env->GetIntField(obj._obj, fid);
}

jlong JEnv::GetLongField(JObject const& obj, jfieldID fid)
{
    return tls_env->GetLongField(obj._obj, fid);
}

jfloat JEnv::GetFloatField(JObject const& obj, jfieldID fid)
{
    return tls_env->GetFloatField(obj._obj, fid);
}

jdouble JEnv::GetDoubleField(JObject const& obj, jfieldID fid)
{
    return tls_env->GetDoubleField(obj._obj, fid);
}

void JEnv::SetObjectField(JObject const& obj, jfieldID id, object_type const& v)
{
    if (v)
    {
        tls_env->SetObjectField(obj._obj, id, v->_obj);
    }
    else
    {
        tls_env->SetObjectField(obj._obj, id, nullptr);
    }
}

void JEnv::SetStringField(JObject const& obj, jfieldID id, string const& v)
{
    auto s = tls_env->NewStringUTF(v.c_str());
    tls_env->SetObjectField(obj._obj, id, s);
    tls_env->DeleteLocalRef(s);
}

void JEnv::SetBooleanField(JObject const& obj, jfieldID id, jboolean v)
{
    tls_env->SetBooleanField(obj._obj, id, v);
}

void JEnv::SetByteField(JObject const& obj, jfieldID id, jbyte v)
{
    tls_env->SetByteField(obj._obj, id, v);
}

void JEnv::SetCharField(JObject const& obj, jfieldID id, jchar v)
{
    tls_env->SetCharField(obj._obj, id, v);
}

void JEnv::SetShortField(JObject const& obj, jfieldID id, jshort v)
{
    tls_env->SetShortField(obj._obj, id, v);
}

void JEnv::SetIntField(JObject const& obj, jfieldID id, jint v)
{
    tls_env->SetIntField(obj._obj, id, v);
}

void JEnv::SetLongField(JObject const& obj, jfieldID id, jlong v)
{
    tls_env->SetLongField(obj._obj, id, v);
}

void JEnv::SetFloatField(JObject const& obj, jfieldID id, jfloat v)
{
    tls_env->SetFloatField(obj._obj, id, v);
}

void JEnv::SetDoubleField(JObject const& obj, jfieldID id, jdouble v)
{
    tls_env->SetDoubleField(obj._obj, id, v);
}

jmethodID JEnv::GetMethodID(JClass const& cls, string const& name, string const& sig)
{
    return tls_env->GetMethodID((jclass)cls._clazz._obj, name.c_str(), sig.c_str());
}

jmethodID JEnv::GetStaticMethodID(JClass const& cls, string const& name, string const& sig)
{
    return tls_env->GetStaticMethodID((jclass)cls._clazz._obj, name.c_str(), sig.c_str());
}

jobject JEnv::NewObject(JClass const& cls, jmethodID id, JValues const& vals)
{
    return tls_env->NewObjectA((jclass)cls._clazz._obj, id, vals._args());
}

jclass JEnv::GetObjectClass(jobject obj)
{
    return tls_env->GetObjectClass(obj);
}

jboolean JEnv::CallStaticBooleanMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    return tls_env->CallStaticBooleanMethodA((jclass)cls._clazz._obj, id, vals._args());
}

jbyte JEnv::CallStaticByteMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    return tls_env->CallStaticByteMethodA((jclass)cls._clazz._obj, id, vals._args());
}

jchar JEnv::CallStaticCharMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    return tls_env->CallStaticCharMethodA((jclass)cls._clazz._obj, id, vals._args());
}

jshort JEnv::CallStaticShortMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    return tls_env->CallStaticShortMethodA((jclass)cls._clazz._obj, id, vals._args());
}

jint JEnv::CallStaticIntMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    return tls_env->CallStaticIntMethodA((jclass)cls._clazz._obj, id, vals._args());
}

jlong JEnv::CallStaticLongMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    return tls_env->CallStaticLongMethodA((jclass)cls._clazz._obj, id, vals._args());
}

jfloat JEnv::CallStaticFloatMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    return tls_env->CallStaticFloatMethodA((jclass)cls._clazz._obj, id, vals._args());
}

jdouble JEnv::CallStaticDoubleMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    return tls_env->CallStaticDoubleMethodA((jclass)cls._clazz._obj, id, vals._args());
}

JEnv::object_type JEnv::CallStaticObjectMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    auto o = tls_env->CallStaticObjectMethodA((jclass)cls._clazz._obj, id, vals._args());
    if (!o)
        return nullptr;

    auto r = make_shared<JObject>();
    r->_reset(o);
    tls_env->DeleteLocalRef(o);

    return r;
}

JEnv::string_type JEnv::CallStaticStringMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    auto o = (jstring)tls_env->CallStaticObjectMethodA((jclass)cls._clazz._obj, id, vals._args());
    if (!o)
        return nullptr;

    auto r = make_shared<JString>();
    r->_reset(o);
    tls_env->DeleteLocalRef(o);

    return r;
}

JEnv::array_type JEnv::CallStaticArrayMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    auto o =
        (jobjectArray)tls_env->CallStaticObjectMethodA((jclass)cls._clazz._obj, id, vals._args());
    if (!o)
        return nullptr;
    size_t sz = tls_env->GetArrayLength(o);

    auto r = make_shared<JArray>();
    r->_reset(o, sz);
    tls_env->DeleteLocalRef(o);

    return r;
}

void JEnv::CallStaticVoidMethod(JClass const& cls, jmethodID id, JValues const& vals)
{
    tls_env->CallStaticVoidMethodA((jclass)cls._clazz._obj, id, vals._args());
}

jboolean JEnv::CallBooleanMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    return tls_env->CallBooleanMethodA(obj._obj, id, vals._args());
}

jbyte JEnv::CallByteMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    return tls_env->CallByteMethodA(obj._obj, id, vals._args());
}

jchar JEnv::CallCharMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    return tls_env->CallCharMethodA(obj._obj, id, vals._args());
}

jshort JEnv::CallShortMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    return tls_env->CallShortMethodA(obj._obj, id, vals._args());
}

jint JEnv::CallIntMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    return tls_env->CallIntMethodA(obj._obj, id, vals._args());
}

jlong JEnv::CallLongMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    return tls_env->CallLongMethodA(obj._obj, id, vals._args());
}

jfloat JEnv::CallFloatMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    return tls_env->CallFloatMethodA(obj._obj, id, vals._args());
}

jdouble JEnv::CallDoubleMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    return tls_env->CallDoubleMethodA(obj._obj, id, vals._args());
}

JEnv::object_type JEnv::CallObjectMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    auto o = tls_env->CallObjectMethodA(obj._obj, id, vals._args());
    if (!o)
        return nullptr;

    auto r = make_shared<JObject>();
    r->_reset(o);
    tls_env->DeleteLocalRef(o);

    return r;
}

JEnv::string_type JEnv::CallStringMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    auto o = (jstring)tls_env->CallObjectMethodA(obj._obj, id, vals._args());
    if (!o)
        return nullptr;

    auto r = make_shared<JString>();
    r->_reset(o);
    tls_env->DeleteLocalRef(o);

    return r;
}

JEnv::array_type JEnv::CallArrayMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    auto o = (jobjectArray)tls_env->CallObjectMethodA(obj._obj, id, vals._args());
    if (!o)
        return nullptr;
    size_t sz = tls_env->GetArrayLength(o);

    auto r = make_shared<JArray>();
    r->_reset(o, sz);
    tls_env->DeleteLocalRef(o);

    return r;
}

void JEnv::CallVoidMethod(JObject const& obj, jmethodID id, JValues const& vals)
{
    tls_env->CallVoidMethodA(obj._obj, id, vals._args());
}

size_t JEnv::GetArrayLength(jarray arr)
{
    return tls_env->GetArrayLength(arr);
}

jbyte const* JEnv::GetBytes(JArray const& arr)
{
    return tls_env->GetByteArrayElements((jbyteArray)arr._arr._obj, JNI_FALSE);
}

jchar const* JEnv::GetChars(JArray const& arr)
{
    return tls_env->GetCharArrayElements((jcharArray)arr._arr._obj, JNI_FALSE);
}

void JEnv::ProcessBytes(JArray const& arr, ::std::function<void(jbyte const*)> proc)
{
    jboolean cp;
    auto r = tls_env->GetByteArrayElements((jbyteArray)arr._arr._obj, &cp);
    proc(r);
    if (cp)
        tls_env->ReleaseByteArrayElements((jbyteArray)arr._arr._obj, r, 0);
}

void JEnv::ProcessChars(JArray const& arr, ::std::function<void(jchar const*)> proc)
{
    jboolean cp;
    auto r = tls_env->GetCharArrayElements((jcharArray)arr._arr._obj, &cp);
    proc(r);
    if (cp)
        tls_env->ReleaseCharArrayElements((jcharArray)arr._arr._obj, r, 0);
}

jobject JEnv::NewLocalRef(jobject obj)
{
    return tls_env->NewLocalRef(obj);
}

void JEnv::DeleteLocalRef(jobject obj)
{
    tls_env->DeleteLocalRef(obj);
}

jobject JEnv::NewGlobalRef(jobject obj)
{
    return tls_env->NewGlobalRef(obj);
}

void JEnv::DeleteGlobalRef(jobject obj)
{
    // global在业务层通常会持久拿在手里，所以当jvm全局清理时，vm会变空，此时不应再释放
    if (!gs_vm)
        return;
    return tls_env->DeleteGlobalRef(obj);
}

jsize JEnv::GetStringUTFLength(jstring jstr)
{
    return tls_env->GetStringUTFLength(jstr);
}

string JEnv::GetStringUTFChars(jstring jstr)
{
    jboolean cp = false;
    char const* cs = tls_env->GetStringUTFChars(jstr, &cp);
    if (cs)
    {
        string r = cs;
        if (cp)
            tls_env->ReleaseStringUTFChars(jstr, cs);
        return r;
    }
    return "";
}

jstring JEnv::NewStringUTF(string const& str)
{
    return tls_env->NewStringUTF(str.c_str());
}

void JEnv::ExceptionClear()
{
    tls_env->ExceptionClear();
}

namespace TypeSignature
{
const JTypeSignature CLASS = "Ljava/lang/Class;";

const JTypeSignature STRING = "Ljava/lang/String;";

const JTypeSignature OBJECT = "Ljava/lang/Object;";

const JTypeSignature BOOLEAN = "Z";

const JTypeSignature BYTE = "B";

const JTypeSignature CHAR = "C";

const JTypeSignature SHORT = "S";

const JTypeSignature INT = "I";

const JTypeSignature LONG = "J";

const JTypeSignature FLOAT = "F";

const JTypeSignature DOUBLE = "D";

const JTypeSignature VOID = "V";

const JTypeSignature BYTEARRAY = "[B";
} // namespace TypeSignature

ExceptionGuard::~ExceptionGuard()
{
    // 如果运行在子线程中，则为自动清理，否则JNI会每次都抛出 using JNIEnv* from thread Thread 的异常
    if (!tls_guard.ismain)
    {
        // 不进行Clear操作，也会报这个问题，采用业务层遇到错误时自己清理
        return;
    }

    if (Check() && _print)
    {
        Logger::Error("捕获JNI异常 " + tls_guard.errmsg);
    }
}

bool ExceptionGuard::Check()
{
    if (JNI_TRUE != tls_env->ExceptionCheck())
    {
        tls_guard.errmsg.clear();
        return false;
    }

    jthrowable err = tls_env->ExceptionOccurred();
    tls_env->ExceptionClear();

    JEntry<jre::Throwable> obj(JVariant((jobject)err));
    tls_guard.errmsg = *obj->toString(obj);

    return true;
}

string ExceptionGuard::GetLastErrorMessage()
{
    return tls_guard.errmsg;
}

JClassName::JClassName(JClassPath const& cp)
{
    *this = ::CROSS_NS::replace((string const&)cp, "/", ".");
}

JTypeSignature::JTypeSignature(JClassPath const& cp)
{
    *this = cp;
}

JTypeSignature& JTypeSignature::operator=(JClassPath const& cp)
{
    if (cp.empty())
    {
        *this = "V";
        goto LABEL_RETURN;
    }
    if (cp[0] == 'L')
    {
        *this = (string const&)cp;
        goto LABEL_RETURN;
    }
    if (cp.find('/') != string::npos)
    {
        *this = "L" + (string const&)cp + ";";
        goto LABEL_RETURN;
    }
    *this = (string const&)cp;

LABEL_RETURN:
    return *this;
}

AJNI_END
