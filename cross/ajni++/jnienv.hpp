#ifndef __AJNI_JNIENV_H_INCLUDED
#define __AJNI_JNIENV_H_INCLUDED

// #define AJNI_CHECKEXCEPTION ::AJNI_NS::ExceptionGuard _NNT_COMBINE(__exception_guard_, __LINE__)

#define AJNI_API(ret) extern "C" JNIEXPORT ret JNICALL
#define AJNI_FUNCNAME(mod, cls, name) Java_##mod##_##cls##_##name
#define AJNI_COMPANION_FUNCNAME(mod, cls, name) Java_##mod##_##cls##_00024Companion_##name
#define AJNI_FUNC(cls, name) AJNI_FUNCNAME(com_nnt_ajnixx, cls, name)
#define AJNI_COMPANION_FUNC(cls, name) AJNI_COMPANION_FUNCNAME(com_nnt_ajnixx, cls, name)

// 如果没有实现，需要再so里实现JNI的初始化函数
#define AJNI_IMP_LOADED(exp) \
AJNI_API(jint) JNI_OnLoad(JavaVM *vm, void *reserved) \
{ \
::AJNI_NS::Env.BindVM(vm); \
exp \
return JNI_VERSION_1_4; \
}

#define AJNI_IMP_UNLOADED(exp) \
AJNI_API(void) JNI_OnUnload(JavaVM* vm, void* reserved) \
{ \
        ::AJNI_NS::Env.UnbindVM(); \
        exp \
}

AJNI_BEGIN

using ::std::make_shared;
using ::std::shared_ptr;
using ::std::string;
using ::std::endl;

// 定义全局空对象
extern const jobject jnull;

#define _STRING_LIKE_IMPL(left, right) \
    using left::right; \
    using left::operator =; \
    using left::operator +=;

template<typename T>
class StringLike: public string
{
public:
    _STRING_LIKE_IMPL(string, string)

    StringLike(string const &r)
        : string(r)
    {}
};

// 定义java类的路径，例如 com/google/gson/Gson
class JClassPath: public StringLike<JClassPath>
{
public:
    _STRING_LIKE_IMPL(StringLike<JClassPath>, StringLike)
};

// 定义java类的名称，例如 com.google.gson.Gson
class JClassName: public StringLike<JClassName>
{
public:
    _STRING_LIKE_IMPL(StringLike<JClassName>, StringLike)

    JClassName(JClassPath const &);
};

// 定义签名类型
class JTypeSignature: public StringLike<JTypeSignature>
{
public:
    _STRING_LIKE_IMPL(StringLike<JTypeSignature>, StringLike)

    JTypeSignature(JClassPath const &);

    JTypeSignature &operator=(JClassPath const &);
};

class JObject;
class JClass;
class JString;
class JVariant;
class JValues;
class JArray;
class JContext;

NNT_CLASS_PREPARE(JEnv);

class JEnv
{
NNT_CLASS_DECL(JEnv);

public:

    JEnv();
    ~JEnv();

    // 设置vm对象，ajni++不使用JNI_OnLoad的形式获取vm避免干扰其他库的工作
    // \@env 已经存在的env，不传则即时获取一个
    void BindVM(JavaVM *, JNIEnv *env = nullptr);

    // 取消注册
    void UnbindVM();

    // 检查线程安全行
    void Check();

    // 绑定当前上下文，用于在JNI环境中获得资源访问
    void BindContext(jobject act, jobject ctx);

    // 外部业务层提供的创建线程中JNIEnv的实现
    typedef ::std::function<JNIEnv *()> jnienv_retrieve_impl;
    jnienv_retrieve_impl GetCurrentJniEnv;

    // 获得上下文，之后类均从该对象获得
    JContext &context();

    // 全局锁
    void lock();
    void unlock();

    typedef shared_ptr<JObject> object_type;
    typedef shared_ptr<JString> string_type;
    typedef shared_ptr<JArray> array_type;
    typedef shared_ptr<JClass> class_type;

    class_type FindClass(string const &);
    bool IsAssignableFrom(JClass const &, JClass const &);
    bool IsInstanceOf(JObject const &, JClass const &);
    bool IsSameObject(JObject const &, JObject const &);

    jfieldID GetStaticFieldID(JClass const &, string const &name, string const &typ);
    jobject GetStaticObjectField(JClass const &, jfieldID);
    string_type GetStaticStringField(JClass const &, jfieldID);
    array_type GetStaticArrayField(JClass const &, jfieldID);
    jboolean GetStaticBooleanField(JClass const &, jfieldID);
    jbyte GetStaticByteField(JClass const &, jfieldID);
    jchar GetStaticCharField(JClass const &, jfieldID);
    jshort GetStaticShortField(JClass const &, jfieldID);
    jint GetStaticIntField(JClass const &, jfieldID);
    jlong GetStaticLongField(JClass const &, jfieldID);
    jfloat GetStaticFloatField(JClass const &, jfieldID);
    jdouble GetStaticDoubleField(JClass const &, jfieldID);

    void SetStaticObjectField(JClass const &, jfieldID, JVariant const &);
    void SetStaticBooleanField(JClass const &, jfieldID, jboolean);
    void SetStaticByteField(JClass const &, jfieldID, jbyte);
    void SetStaticCharField(JClass const &, jfieldID, jchar);
    void SetStaticShortField(JClass const &, jfieldID, jshort);
    void SetStaticIntField(JClass const &, jfieldID, jint);
    void SetStaticLongField(JClass const &, jfieldID, jlong);
    void SetStaticFloatField(JClass const &, jfieldID, jfloat);
    void SetStaticDoubleField(JClass const &, jfieldID, jdouble);

    jfieldID GetFieldID(JClass const &, string const &name, string const &sig);
    object_type GetObjectField(JObject const &, jfieldID);
    string_type GetStringField(JObject const &, jfieldID);
    array_type GetArrayField(JObject const &, jfieldID);
    jboolean GetBooleanField(JObject const &, jfieldID);
    jbyte GetByteField(JObject const &, jfieldID);
    jchar GetCharField(JObject const &, jfieldID);
    jshort GetShortField(JObject const &, jfieldID);
    jint GetIntField(JObject const &, jfieldID);
    jlong GetLongField(JObject const &, jfieldID);
    jfloat GetFloatField(JObject const &, jfieldID);
    jdouble GetDoubleField(JObject const &, jfieldID);

    void SetObjectField(JObject const &, jfieldID, object_type const &);
    void SetStringField(JObject const &, jfieldID, string const &);
    void SetBooleanField(JObject const &, jfieldID, jboolean);
    void SetByteField(JObject const &, jfieldID, jbyte);
    void SetCharField(JObject const &, jfieldID, jchar);
    void SetShortField(JObject const &, jfieldID, jshort);
    void SetIntField(JObject const &, jfieldID, jint);
    void SetLongField(JObject const &, jfieldID, jlong);
    void SetFloatField(JObject const &, jfieldID, jfloat);
    void SetDoubleField(JObject const &, jfieldID, jdouble);

    jmethodID GetMethodID(JClass const &, string const &name, string const &sig);
    jmethodID GetStaticMethodID(JClass const &, string const &name, string const &sig);

    jobject NewObject(JClass const &, jmethodID, JValues const &);
    jclass GetObjectClass(jobject);

    jboolean CallStaticBooleanMethod(JClass const &, jmethodID, JValues const &);
    jbyte CallStaticByteMethod(JClass const &, jmethodID, JValues const &);
    jchar CallStaticCharMethod(JClass const &, jmethodID, JValues const &);
    jshort CallStaticShortMethod(JClass const &, jmethodID, JValues const &);
    jint CallStaticIntMethod(JClass const &, jmethodID, JValues const &);
    jlong CallStaticLongMethod(JClass const &, jmethodID, JValues const &);
    jfloat CallStaticFloatMethod(JClass const &, jmethodID, JValues const &);
    jdouble CallStaticDoubleMethod(JClass const &, jmethodID, JValues const &);
    object_type CallStaticObjectMethod(JClass const &, jmethodID, JValues const &);
    string_type CallStaticStringMethod(JClass const &, jmethodID, JValues const &);
    array_type CallStaticArrayMethod(JClass const &, jmethodID, JValues const &);
    void CallStaticVoidMethod(JClass const &, jmethodID, JValues const &);

    jboolean CallBooleanMethod(JObject const &, jmethodID, JValues const &);
    jbyte CallByteMethod(JObject const &, jmethodID, JValues const &);
    jchar CallCharMethod(JObject const &, jmethodID, JValues const &);
    jshort CallShortMethod(JObject const &, jmethodID, JValues const &);
    jint CallIntMethod(JObject const &, jmethodID, JValues const &);
    jlong CallLongMethod(JObject const &, jmethodID, JValues const &);
    jfloat CallFloatMethod(JObject const &, jmethodID, JValues const &);
    jdouble CallDoubleMethod(JObject const &, jmethodID, JValues const &);
    object_type CallObjectMethod(JObject const &, jmethodID, JValues const &);
    string_type CallStringMethod(JObject const &, jmethodID, JValues const &);
    array_type CallArrayMethod(JObject const &, jmethodID, JValues const &);
    void CallVoidMethod(JObject const &, jmethodID, JValues const &);

    size_t GetArrayLength(jarray);
    jbyte const *GetBytes(JArray const &);
    jchar const *GetChars(JArray const &);
    void ProcessBytes(JArray const &, ::std::function<void(jbyte const *)>);
    void ProcessChars(JArray const &, ::std::function<void(jchar const *)>);

    jobject NewLocalRef(jobject);
    void DeleteLocalRef(jobject);

    jobject NewGlobalRef(jobject);
    void DeleteGlobalRef(jobject);

    template<typename T>
    inline T NewLocalRef(T v)
    { return (T) NewLocalRef((jobject) v); }

    template<typename T>
    inline T NewGlobalRef(T v)
    { return (T) NewGlobalRef((jobject) v); }

    jsize GetStringUTFLength(jstring);
    string GetStringUTFChars(jstring);
    jstring NewStringUTF(string const &);

    void ExceptionClear();
};

// 获得全局env
extern JEnv Env;

// Jni对象签名
namespace TypeSignature
{
extern const JTypeSignature CLASS;

extern const JTypeSignature STRING;

extern const JTypeSignature OBJECT;

extern const JTypeSignature BOOLEAN;

extern const JTypeSignature BYTE;

extern const JTypeSignature CHAR;

extern const JTypeSignature SHORT;

extern const JTypeSignature INT;

extern const JTypeSignature LONG;

extern const JTypeSignature FLOAT;

extern const JTypeSignature DOUBLE;

extern const JTypeSignature VOID;

extern const JTypeSignature BYTEARRAY;
} // namespace TypeSignature

// 自动处理JNI内部异常
class ExceptionGuard
{
public:

    ExceptionGuard(bool print = true)
        : _print(print)
    {}
    ~ExceptionGuard();

    // 检查是否有异常发生，异常信息通过修改refin的string带出
    // @return true代表发生了异常
    static bool Check();

    // 获得最后捕获的错误信息
    static string GetLastErrorMessage();

private:
    bool _print;
};

// AJNI日志接口
class Logger
{
public:

    static void Debug(string const &);
    static void Info(string const &);
    static void Warn(string const &);
    static void Error(string const &);
    static void Fatal(string const &);
};

AJNI_END

#endif
