#include "ajni++.hpp"
#define __AJNI_PRIVATE__
#include "jnienv.hpp"
#include "ast.hpp"
#include "variant.hpp"
#include "jre.hpp"
#include "com++.hpp"
#include "android-prv.hpp"
#include <sstream>
#include <cmath>

#include <cross/cross.hpp>
#include <cross/str.hpp>

AJNI_BEGIN

extern shared_ptr<JVariant> ReadToVariant(jobject _obj);

JObject::JObject()
{
    // pass
}

JObject::JObject(jobject obj, bool local)
    : _obj(obj), _local(local)
{
    if (_obj)
    {
        if (_local)
            _obj = Env.NewLocalRef(_obj);
        else
            _obj = Env.NewGlobalRef(_obj);
    }
}

JObject::JObject(JObject const& r)
    : _obj(r._obj), _local(r._local)
{
    if (_obj)
    {
        if (_local)
            _obj = Env.NewLocalRef(_obj);
        else
            _obj = Env.NewGlobalRef(_obj);
    }
}

JObject::~JObject()
{
    if (_obj)
    {
        if (_local)
            Env.DeleteLocalRef(_obj);
        else
            Env.DeleteGlobalRef(_obj);
    }
}

jobject JObject::asReturn() const
{
    return _local ? Env.NewLocalRef(_obj) : _obj;
}

bool JObject::isnil() const
{
    return _obj == nullptr;
}

JObject& JObject::operator=(JObject const& r)
{
    if (_obj == r._obj)
        return *this;

    auto old = _obj;
    auto oldlocal = _local;

    _obj = r._obj;
    _local = r._local;
    if (_obj)
    {
        if (_local)
            _obj = Env.NewLocalRef(_obj);
        else
            _obj = Env.NewGlobalRef(_obj);
    }

    if (old)
    {
        if (oldlocal)
            Env.DeleteLocalRef(old);
        else
            Env.DeleteGlobalRef(old);
    }

    return *this;
}

void JObject::_reset(jobject obj, bool local)
{
    if (_obj == obj)
        return;

    auto old = _obj;
    auto oldlocal = _local;

    _obj = obj;
    _local = local;
    if (_obj)
    {
        if (_local)
            _obj = Env.NewLocalRef(_obj);
        else
            _obj = Env.NewGlobalRef(_obj);
    }

    if (old)
    {
        if (oldlocal)
            Env.DeleteLocalRef(old);
        else
            Env.DeleteGlobalRef(old);
    }
}

void JObject::_asglobal()
{
    if (!_local || !_obj)
        return;
    auto old = _obj;
    _obj = Env.NewGlobalRef(old);
    if (old)
        Env.DeleteLocalRef(old);
    _local = false;
}

JObject::_JGlobalObject::_JGlobalObject(JObject const& r)
    : _refs(1)
{
    auto t = make_shared<JObject>();
    t->_reset(r._obj, false); // 转换成global对象
    gobj = t;
}

void JObject::_JGlobalObject::grab()
{
    ++_refs;
}

bool JObject::_JGlobalObject::drop()
{
    if (--_refs == 0)
    {
        // 需要释放
        gobj = nullptr;
        return true;
    }
    return false;
}

JString::JString()
{
    // pass
}

JString::JString(JString const& r)
    : _str(r._str)
{
    // pass
}

JString::JString(string const& str)
    : _str(str)
{
    // pass
}

JString::~JString()
{
    // pass
}

void JString::_reset(jstring v)
{
    _str = Env.GetStringUTFChars(v);
}

jstring JString::asReturn() const
{
    return Env.NewStringUTF(_str);
}

JArray::JArray()
{
    // pass
}

JArray::~JArray()
{
    // pass
}

void JArray::_reset(jobjectArray arr, size_t sz)
{
    _sz = sz;
    _arr._reset(arr);
    const_cast<VT&>(vt) = VT::OBJECT;
}

void JArray::_reset(jbooleanArray arr, size_t sz)
{
    _sz = sz;
    _arr._reset(arr);
    const_cast<VT&>(vt) = VT::BOOLEAN;
}

void JArray::_reset(jbyteArray arr, size_t sz)
{
    _sz = sz;
    _arr._reset(arr);
    const_cast<VT&>(vt) = VT::BYTE;
}

void JArray::_reset(jcharArray arr, size_t sz)
{
    _sz = sz;
    _arr._reset(arr);
    const_cast<VT&>(vt) = VT::CHAR;
}

void JArray::_reset(jshortArray arr, size_t sz)
{
    _sz = sz;
    _arr._reset(arr);
    const_cast<VT&>(vt) = VT::SHORT;
}

void JArray::_reset(jintArray arr, size_t sz)
{
    _sz = sz;
    _arr._reset(arr);
    const_cast<VT&>(vt) = VT::INT;
}

void JArray::_reset(jlongArray arr, size_t sz)
{
    _sz = sz;
    _arr._reset(arr);
    const_cast<VT&>(vt) = VT::LONG;
}

void JArray::_reset(jfloatArray arr, size_t sz)
{
    _sz = sz;
    _arr._reset(arr);
    const_cast<VT&>(vt) = VT::FLOAT;
}

void JArray::_reset(jdoubleArray arr, size_t sz)
{
    _sz = sz;
    _arr._reset(arr);
    const_cast<VT&>(vt) = VT::DOUBLE;
}

string JArray::toString() const
{
    if (_arr.isnil())
        return "";
    if (vt == VT::BYTE)
    {
        string r;
        Env.ProcessBytes(*this, [&](jbyte const* bs)
        {
            r.assign((char const*)bs, _sz);
        });
        return r;
    }
    else if (vt == VT::CHAR)
    {
        string r;
        Env.ProcessChars(*this, [&](jchar const* cs)
        {
            r.assign((char const*)cs, _sz);
        });
        return r;
    }
    return "";
}

shared_ptr<JArray::bytes_type> JArray::toBytes() const
{
    if (vt == VT::BYTE)
    {
        auto r = make_shared<bytes_type>();
        r->resize(_sz);
        Env.ProcessBytes(*this, [&](jbyte const* bs)
        {
            ::std::memcpy(&r->at(0), bs, _sz);
        });
        return r;
    }
    else if (vt == VT::CHAR)
    {
        auto r = make_shared<bytes_type>();
        r->resize(_sz);
        Env.ProcessChars(*this, [&](jchar const* cs)
        {
            ::std::memcpy(&r->at(0), cs, _sz);
        });
        return r;
    }
    return nullptr;
}

JValue::JValue(JValue const& r)
    : _val(r._val), _fnidx(r._fnidx)
{
    if (_fnidx)
    {
        Env.context().function_grab(_fnidx);
    }
}

JValue::JValue(JVariant const& var)
{
    JVariant::variant_type const& comvar = var;
    typedef JVariant::variant_type::VT VT;
    switch (comvar.vt)
    {
    case VT::INT:
    case VT::UINT:
        _val.i = comvar.toInt();
        break;
    case VT::FLOAT:
        _val.f = comvar.toFloat();
        break;
    case VT::DOUBLE:
        _val.d = comvar.toDouble();
        break;
    case VT::SHORT:
    case VT::USHORT:
        _val.s = comvar.toShort();
        break;
    case VT::LONG:
    case VT::ULONG:
        _val.j = comvar.toLong();
        break;
    case VT::LONGLONG:
    case VT::ULONGLONG:
        _val.j = (jlong)comvar.toLonglong();
        break;
    case VT::CHAR:
    case VT::UCHAR:
        _val.c = comvar.toChar();
        break;
    case VT::BOOLEAN:
        _val.z = comvar.toBool();
        break;
    case VT::OBJECT:
    {
        _val.l = comvar.toObject();
    }
        break;
    case VT::STRING:
    {
        _val.l = Env.NewStringUTF(comvar.toString());
        _free = true;
    }
        break;
    case VT::NIL:
        break;
    case VT::FUNCTION:
    {
        auto cls = Env.context().register_class<jre::Callback>();
        JEntry<jre::Callback> cb(*cls->construct());
        // 将当前的函数保存到全局监听，执行结束后进行释放
        _fnidx = Env.context().add(var.toFunction());
        cb->id(cb, (jlong)_fnidx);
        _val.l = cb.asReturn();
    }
        break;
    default:
        Logger::Error("ajnixx: 不支持类型转换 " + ::CROSS_NS::tostr((int)comvar.vt));
        break;
    }
}

JValue::~JValue()
{
    if (_free && _val.l)
    {
        Env.DeleteLocalRef(_val.l);
        _val.l = nullptr;
    }

    if (_fnidx)
    {
        Env.context().function_drop(_fnidx);
        _fnidx = 0;
    }
}

JValues::JValues(::std::initializer_list<args_type::value_type> const& vars)
{
    for (auto& e : vars)
    {
        auto t = make_shared<JValue>(*e);
        _vals.emplace_back(t);
        _jvals.emplace_back(*t);
    }
}

JValues::JValues(args_type const& vars)
{
    for (auto& e : vars)
    {
        auto t = make_shared<JValue>(*e);
        _vals.emplace_back(t);
        _jvals.emplace_back(*t);
    }
}

JVariant::JVariant()
    : vt(VT::NIL)
{
}

JVariant::JVariant(nullptr_t)
    : vt(VT::NIL)
{
}

JVariant::JVariant(const char s[])
    : vt(VT::STRING), _var(s)
{
}

JVariant::JVariant(bool v)
    : vt(VT::BOOLEAN), _var(v)
{
}

JVariant::JVariant(jchar v)
    : vt(VT::INTEGER), _var(v)
{
}

JVariant::JVariant(jbyte v)
    : vt(VT::INTEGER), _var(v)
{
}

JVariant::JVariant(jshort v)
    : vt(VT::INTEGER), _var(v)
{
}

JVariant::JVariant(jint v)
    : vt(VT::INTEGER), _var(v)
{
}

JVariant::JVariant(jlong v)
    : vt(VT::INTEGER), _var(v)
{
}

JVariant::JVariant(jfloat v)
    : vt(VT::NUMBER), _var(v)
{
}

JVariant::JVariant(jdouble v)
    : vt(VT::NUMBER), _var(v)
{
}

JVariant::JVariant(jobject v)
    : vt(VT::OBJECT), _var(v)
{
}

JVariant::JVariant(shared_ptr<JArray> const& arr)
    : vt(VT::ARRAY), _arr(arr), _var((jobject)0)
{
}

JVariant::JVariant(jstring v)
    : vt(VT::STRING), _var(Env.GetStringUTFChars(v))
{
}

JVariant::JVariant(string const& v)
    : vt(VT::STRING), _var(JString(v))
{
}

JVariant::JVariant(function_type::fun0_type fn)
    : vt(VT::FUNCTION), _fun(make_shared<function_type>(fn)), _var((variant_type::func_type)0)
{
}

JVariant::JVariant(function_type::fun1_type fn)
    : vt(VT::FUNCTION), _fun(make_shared<function_type>(fn)), _var((variant_type::func_type)0)
{
}

JVariant::JVariant(function_type::fun2_type fn)
    : vt(VT::FUNCTION), _fun(make_shared<function_type>(fn)), _var((variant_type::func_type)0)
{
}

JVariant::JVariant(function_type::fun3_type fn)
    : vt(VT::FUNCTION), _fun(make_shared<function_type>(fn)), _var((variant_type::func_type)0)
{
}

JVariant::JVariant(function_type::fun4_type fn)
    : vt(VT::FUNCTION), _fun(make_shared<function_type>(fn)), _var((variant_type::func_type)0)
{
}

JVariant::JVariant(function_type::fun5_type fn)
    : vt(VT::FUNCTION), _fun(make_shared<function_type>(fn)), _var((variant_type::func_type)0)
{
}

JVariant::JVariant(function_type::fun6_type fn)
    : vt(VT::FUNCTION), _fun(make_shared<function_type>(fn)), _var((variant_type::func_type)0)
{
}

JVariant::JVariant(function_type::fun7_type fn)
    : vt(VT::FUNCTION), _fun(make_shared<function_type>(fn)), _var((variant_type::func_type)0)
{
}

JVariant::JVariant(function_type::fun8_type fn)
    : vt(VT::FUNCTION), _fun(make_shared<function_type>(fn)), _var((variant_type::func_type)0)
{
}

JVariant::JVariant(function_type::fun9_type fn)
    : vt(VT::FUNCTION), _fun(make_shared<function_type>(fn)), _var((variant_type::func_type)0)
{
}

string JVariant::toString() const
{
    if (_var.vt == variant_type::VT::STRING)
        return _var.toString();

    if (_var.vt == variant_type::VT::FLOAT ||
        _var.vt == variant_type::VT::DOUBLE)
    {
        return ::CROSS_NS::tostr(toNumber());
    }

    return ::CROSS_NS::tostr((int)toInteger());
}

integer JVariant::toInteger() const
{
    switch (_var.vt)
    {
    case variant_type::VT::INT:
        return _var.toInt();
    case variant_type::VT::UINT:
        return _var.toUInt();
    case variant_type::VT::LONG:
        return _var.toLong();
    case variant_type::VT::ULONG:
        return _var.toULong();
    case variant_type::VT::SHORT:
        return _var.toShort();
    case variant_type::VT::USHORT:
        return _var.toUShort();
    case variant_type::VT::LONGLONG:
        return (integer)_var.toLonglong();
    case variant_type::VT::ULONGLONG:
        return (integer)_var.toULonglong();
    case variant_type::VT::CHAR:
        return _var.toChar();
    case variant_type::VT::UCHAR:
        return _var.toUChar();
    case variant_type::VT::BOOLEAN:
        return _var.toBool();
    case variant_type::VT::FLOAT:
        return (integer)round(_var.toFloat());
    case variant_type::VT::DOUBLE:
        return (integer)round(_var.toDouble());
    case variant_type::VT::STRING:
        return ::CROSS_NS::toint(_var.toString());
    default:
        break;
    }
    return 0;
}

number JVariant::toNumber() const
{
    switch (_var.vt)
    {
    case variant_type::VT::FLOAT:
        return _var.toFloat();
    case variant_type::VT::DOUBLE:
        return _var.toDouble();
    case variant_type::VT::STRING:
        return ::CROSS_NS::todouble(_var.toString());
    default:
        break;
    }
    return toInteger();
}

bool JVariant::toBool() const
{
    if (_var.vt == variant_type::VT::BOOLEAN)
        return _var.toBool();
    return toNumber() != 0;
}

JTypeSignature JVariant::signature() const
{
    // 函数类型
    if (vt == VT::FUNCTION)
    {
        return jre::TypeSignature::CALLBACK;
    }

    // 普通pod数据类型
    switch (_var.vt)
    {
    case variant_type::VT::BOOLEAN:
        return TypeSignature::BOOLEAN;
    case variant_type::VT::INT:
    case variant_type::VT::UINT:
        return TypeSignature::INT;
    case variant_type::VT::SHORT:
    case variant_type::VT::USHORT:
        return TypeSignature::SHORT;
    case variant_type::VT::LONG:
    case variant_type::VT::ULONG:
    case variant_type::VT::LONGLONG:
    case variant_type::VT::ULONGLONG:
        return TypeSignature::LONG;
    case variant_type::VT::FLOAT:
        return TypeSignature::FLOAT;
    case variant_type::VT::DOUBLE:
        return TypeSignature::DOUBLE;
    case variant_type::VT::CHAR:
    case variant_type::VT::UCHAR:
        return TypeSignature::CHAR;
    case variant_type::VT::STRING:
        return TypeSignature::STRING;
    case variant_type::VT::NIL:
        return TypeSignature::VOID;
    default:
        break;
    }

    return TypeSignature::OBJECT;
}

shared_ptr<JObject> JVariant::toObject() const
{
    auto r = make_shared<JObject>();
    r->_reset(_var.toObject());
    return r;
}

shared_ptr<JVariant> JVariant::FromObject(JObject const& obj)
{
    return make_shared<JVariant>(obj._obj);
}

AJNI_END
