#ifndef __AJNI_VARIANT_H_INCLUDED
#define __AJNI_VARIANT_H_INCLUDED

#include "com++.hpp"
#include "ast.hpp"
#include <vector>
#include <functional>
#include <atomic>

AJNI_BEGIN

namespace kotlin
{
class JClass;
}

class JVariant;
class JArray;

typedef ::std::vector<JVariant const *> args_type;

class JObject
{
public:

    JObject();
    JObject(JObject const &);

    // 释放引用计数
    virtual ~JObject();

    // 作为程序返回值输出
    virtual jobject asReturn() const;

    // 转换成具体的Variant类型，和Variant(JObject)不同，转换会读取具体对象内部信息，返回C++数据构成的Variant对象
    static shared_ptr<JVariant> Extract(jobject);
    static shared_ptr<JVariant> Extract(jobjectArray);
    static shared_ptr<JVariant> Extract(jbooleanArray);
    static shared_ptr<JVariant> Extract(jbyteArray);
    static shared_ptr<JVariant> Extract(jcharArray);
    static shared_ptr<JVariant> Extract(jshortArray);
    static shared_ptr<JVariant> Extract(jintArray);
    static shared_ptr<JVariant> Extract(jlongArray);
    static shared_ptr<JVariant> Extract(jfloatArray);
    static shared_ptr<JVariant> Extract(jdoubleArray);

    // 将var转换成obj，如果本身就是obj则直接返回，如果是pod类型，则转换为对应的Java对象
    static shared_ptr<JObject> Putin(shared_ptr<JVariant> const &);

    // 是否时空对象
    bool isnil() const;

    JObject &operator=(JObject const &);

protected:

    // 只允许特殊情况使用
    class _JGlobalObject
    {
    public:

        _JGlobalObject(JObject const &);

        void grab();
        bool drop();

        shared_ptr<JObject> gobj; // 全局对象

    private:
        ::std::atomic<int> _refs; // 引用计数
    };

    // 自动引用计数
    explicit JObject(jobject, bool local = true);

    void _reset(jobject, bool local = true);

    jobject _obj = nullptr;
    bool _local = true;

    // 提权到global
    virtual void _asglobal();

    friend class JEnv;
    friend class JEnvPrivate;
    friend class JClass;
    friend class JArray;
    friend class JVariant;
    friend class kotlin::JClass;
    template<typename T> friend
    class JEntry;
};

class JString
{
public:

    JString();
    JString(string const &);
    JString(JString const &);

    ~JString();

    inline operator const string &() const
    {
        return _str;
    }

    // 作为程序返回值输出
    jstring asReturn() const;

protected:

    void _reset(jstring);

    string _str;

    friend class JEnv;
    friend class JVariant;
};

class JArray
{
public:

    // 数组中数据类型
    enum struct VT
    {
        UNKNOWN,
        OBJECT,
        BOOLEAN,
        BYTE,
        CHAR,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE
    };

    JArray();
    ~JArray();

    // 获取数组长度
    inline size_t size() const
    {
        return _sz;
    }

    // array转换成string
    string toString() const;

    typedef ::COMXX_NS::Variant<>::bytes_type bytes_type;

    // 转换为bytes对象
    shared_ptr<bytes_type> toBytes() const;

    // 数据类型
    const VT vt = VT::UNKNOWN;

protected:

    void _reset(jobjectArray, size_t);
    void _reset(jbooleanArray, size_t);
    void _reset(jbyteArray, size_t);
    void _reset(jcharArray, size_t);
    void _reset(jshortArray, size_t);
    void _reset(jintArray, size_t);
    void _reset(jlongArray, size_t);
    void _reset(jfloatArray, size_t);
    void _reset(jdoubleArray, size_t);

private:

    JObject _arr;
    size_t _sz;

    friend class JEnv;
    friend class JObject;
};

class JVariant;

class JValue
{
public:

    JValue(JVariant const &);

    JValue(JValue const &);

    ~JValue();

    inline operator jvalue() const
    {
        return _val;
    }

private:

    jvalue _val = {0};
    bool _free = false;
    size_t _fnidx = 0; // 如果是函数对象，保存函数的本地索引
};

class JValues
{
public:

    JValues() = default;
    JValues(::std::initializer_list<args_type::value_type> const &);
    JValues(args_type const &);

    typedef shared_ptr<JValue> value_type;

    inline size_t size() const
    {
        return _vals.size();
    }

private:

    ::std::vector<value_type> _vals;
    ::std::vector<jvalue> _jvals;

    // 返回jni函数需要的参数列表
    inline jvalue const *_args() const
    {
        return &_jvals[0];
    }

    friend class JEnv;
};

typedef ptrdiff_t integer;

typedef double number;

class JVariant
{
private:

    class JComVariantTypes: public ::com::VariantTypes<>
    {
    public:
        typedef _jobject object_type;
        typedef JString string_type;
    };

    class JComFunctionTypes: public ::COMXX_NS::FunctionTypes<
        JVariant,
        shared_ptr < JVariant>,
                             JVariant const&
    > {};

public:

    typedef ::COMXX_NS::Variant<JComVariantTypes> variant_type;
    typedef ::COMXX_NS::Function<JComFunctionTypes> function_type;

    enum struct VT
    {
        NIL,
        OBJECT,
        BOOLEAN,
        INTEGER,
        NUMBER,
        STRING,
        FUNCTION,
        ARRAY
    };

    const VT vt;

    JVariant();

    JVariant(nullptr_t);

    JVariant(const char s[]);

    JVariant(jstring);

    JVariant(string const &);

    JVariant(bool);

    JVariant(jchar);

    JVariant(jbyte);

    JVariant(jshort);

    JVariant(jint);

    JVariant(jlong);

    JVariant(jfloat);

    JVariant(jdouble);

    JVariant(jobject);

    JVariant(shared_ptr<JArray> const &);

    JVariant(function_type::fun0_type);
    JVariant(function_type::fun1_type);
    JVariant(function_type::fun2_type);
    JVariant(function_type::fun3_type);
    JVariant(function_type::fun4_type);
    JVariant(function_type::fun5_type);
    JVariant(function_type::fun6_type);
    JVariant(function_type::fun7_type);
    JVariant(function_type::fun8_type);
    JVariant(function_type::fun9_type);

    string toString() const;

    integer toInteger() const;

    number toNumber() const;

    bool toBool() const;

    inline operator string() const
    {
        return toString();
    }

    inline operator variant_type const &() const
    {
        return _var;
    }

    inline shared_ptr<function_type> toFunction() const
    {
        return _fun;
    }

    inline shared_ptr<JArray> toArray() const
    {
        return _arr;
    }

    shared_ptr<JObject> toObject() const;
    static shared_ptr<JVariant> FromObject(JObject const &);

    inline bool isnil() const
    {
        return vt == VT::NIL;
    }

    // 获得签名
    JTypeSignature signature() const;

private:

    variant_type _var;
    shared_ptr<function_type> _fun;
    shared_ptr<JArray> _arr;

};

template<typename T>
inline shared_ptr<JVariant> _V(T const &v)
{
    return make_shared<JVariant>(v);
}

template<typename _CharT, typename _Traits>
static ::std::basic_ostream<_CharT, _Traits> &
operator<<(::std::basic_ostream<_CharT, _Traits> &stm, JVariant const &v)
{
    switch (v.vt) {
        default:
            break;
        case JVariant::VT::STRING:
            stm << v.toString();
            break;
        case JVariant::VT::INTEGER:
            stm << v.toInteger();
            break;
        case JVariant::VT::NUMBER:
            stm << v.toNumber();
            break;
        case JVariant::VT::BOOLEAN:
            stm << v.toBool();
            break;
        case JVariant::VT::OBJECT:
            stm << v.toObject();
            break;
    }
    return stm;
}

template<typename _CharT, typename _Traits>
static ::std::basic_ostream<_CharT, _Traits> &
operator<<(::std::basic_ostream<_CharT, _Traits> &stm, shared_ptr<JVariant> const &v)
{
    if (!v)
        return stm;
    return stm << *v;
}

AJNI_END

COMXX_BEGIN

template<>
inline jobject grab<jobject>(jobject obj)
{
    return ::AJNI_NS::Env.NewLocalRef(obj);
}

template<>
inline bool drop<jobject>(jobject obj)
{
    ::AJNI_NS::Env.DeleteLocalRef(obj);
    return true;
}

COMXX_END

#endif
