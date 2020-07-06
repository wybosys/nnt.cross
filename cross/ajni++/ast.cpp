#include "ajni++.hpp"
#define __AJNI_PRIVATE__
#include "jnienv.hpp"
#include "ast.hpp"
#include "jre.hpp"
#include "variant.hpp"
#include "java-prv.hpp"
#include "android-prv.hpp"
#include <atomic>

#include <cross/cross.hpp>
#include <cross/str.hpp>

AJNI_BEGIN

JField::JField(JClass& clz)
    : _clazz(clz)
{
    // pass
}

return_type JStaticField::operator()() const
{
    auto fid = Env.GetStaticFieldID(_clazz, name.c_str(), stype.c_str());
    if (!fid)
    {
        Env.ExceptionClear();
        Logger::Error("没有找到静态变量 " + name + stype);
        return nullptr;
    }

    switch (TypeSignature::GetTypeForSwitch(stype))
    {
    case TypeSignature::TS::BOOLEAN:
        return _V((bool)Env.GetStaticBooleanField(_clazz, fid));
    case TypeSignature::TS::BYTE:
        return _V(Env.GetStaticByteField(_clazz, fid));
    case TypeSignature::TS::CHAR:
        return _V(Env.GetStaticCharField(_clazz, fid));
    case TypeSignature::TS::SHORT:
        return _V(Env.GetStaticShortField(_clazz, fid));
    case TypeSignature::TS::INT:
        return _V(Env.GetStaticIntField(_clazz, fid));
    case TypeSignature::TS::LONG:
        return _V(Env.GetStaticLongField(_clazz, fid));
    case TypeSignature::TS::FLOAT:
        return _V(Env.GetStaticFloatField(_clazz, fid));
    case TypeSignature::TS::DOUBLE:
        return _V(Env.GetStaticDoubleField(_clazz, fid));
    case TypeSignature::TS::STRING:
    {
        auto v = Env.GetStaticStringField(_clazz, fid);
        if (!v)
            return nullptr;
        return _V(*v);
    }
    case TypeSignature::TS::BYTEARRAY:
    {
        auto v = Env.GetStaticArrayField(_clazz, fid);
        if (!v)
            return nullptr;
        return _V(v->toString());
    }
    case TypeSignature::TS::UNKNOWN:
    case TypeSignature::TS::CLASS:
    case TypeSignature::TS::VOID:
    case TypeSignature::TS::OBJECT:
        break;
    }

    jobject v = Env.GetStaticObjectField(_clazz, fid);
    if (!v)
        return nullptr;
    return _V(v);
}

void JStaticField::operator()(JObject& obj, arg_type const& v)
{
    auto fid = Env.GetStaticFieldID(_clazz, name.c_str(), stype.c_str());
    if (!fid)
    {
        Env.ExceptionClear();
        Logger::Error("没有找到静态变量 " + name + stype);
        return;
    }

    switch (TypeSignature::GetTypeForSwitch(stype))
    {
    case TypeSignature::TS::BOOLEAN:
    {
        Env.SetStaticBooleanField(_clazz, fid, v.toBool());
    }
        break;
    case TypeSignature::TS::BYTE:
    {
        Env.SetStaticByteField(_clazz, fid, v.toInteger());
    }
        break;
    case TypeSignature::TS::CHAR:
    {
        Env.SetStaticCharField(_clazz, fid, v.toInteger());
    }
        break;
    case TypeSignature::TS::SHORT:
    {
        Env.SetStaticShortField(_clazz, fid, v.toInteger());
    }
        break;
    case TypeSignature::TS::INT:
    {
        Env.SetStaticIntField(_clazz, fid, v.toInteger());
    }
        break;
    case TypeSignature::TS::LONG:
    {
        Env.SetStaticLongField(_clazz, fid, v.toInteger());
    }
        break;
    case TypeSignature::TS::FLOAT:
    {
        Env.SetStaticFloatField(_clazz, fid, v.toNumber());
    }
        break;
    case TypeSignature::TS::DOUBLE:
    {
        Env.SetStaticDoubleField(_clazz, fid, v.toNumber());
    }
        break;
    case TypeSignature::TS::STRING:
    {
        Env.SetStaticObjectField(_clazz, fid, v);
    }
        break;
    case TypeSignature::TS::UNKNOWN:
    case TypeSignature::TS::OBJECT:
    case TypeSignature::TS::VOID:
    case TypeSignature::TS::BYTEARRAY:
    case TypeSignature::TS::CLASS:
    {
        Env.SetStaticObjectField(_clazz, fid, v);
    }
        break;
    }
}

return_type JMemberField::operator()(JObject& obj) const
{
    auto fid = Env.GetFieldID(_clazz, name.c_str(), stype.c_str());
    if (!fid)
    {
        Env.ExceptionClear();
        Logger::Error("没有找到成员变量 " + name + stype);
        return nullptr;
    }

    switch (TypeSignature::GetTypeForSwitch(stype))
    {
    case TypeSignature::TS::BOOLEAN:
        return _V((bool)Env.GetBooleanField(obj, fid));
    case TypeSignature::TS::BYTE:
        return _V(Env.GetByteField(obj, fid));
    case TypeSignature::TS::CHAR:
        return _V(Env.GetCharField(obj, fid));
    case TypeSignature::TS::SHORT:
        return _V(Env.GetShortField(obj, fid));
    case TypeSignature::TS::INT:
        return _V(Env.GetIntField(obj, fid));
    case TypeSignature::TS::LONG:
        return _V(Env.GetLongField(obj, fid));
    case TypeSignature::TS::FLOAT:
        return _V(Env.GetFloatField(obj, fid));
    case TypeSignature::TS::DOUBLE:
        return _V(Env.GetDoubleField(obj, fid));
    case TypeSignature::TS::STRING:
    {
        auto v = Env.GetStringField(obj, fid);
        return v ? _V(*v) : nullptr;
    }
    case TypeSignature::TS::BYTEARRAY:
    {
        auto arr = Env.GetArrayField(obj, fid);
        return arr ? _V(arr->toString()) : nullptr;
    }
    case TypeSignature::TS::VOID:
    case TypeSignature::TS::CLASS:
    case TypeSignature::TS::UNKNOWN:
    case TypeSignature::TS::OBJECT:
        break;
    }

    auto v = Env.GetObjectField(obj, fid);
    return v ? JVariant::FromObject(*v) : nullptr;
}

void JMemberField::operator()(JObject& obj, arg_type const& v)
{
    auto fid = Env.GetFieldID(_clazz, name.c_str(), stype.c_str());
    if (!fid)
    {
        Env.ExceptionClear();
        Logger::Error("没有找到成员变量 " + name + stype);
        return;
    }

    switch (TypeSignature::GetTypeForSwitch(stype))
    {
    case TypeSignature::TS::BOOLEAN:
    {
        Env.SetBooleanField(obj, fid, v.toBool());
    }
        break;
    case TypeSignature::TS::BYTE:
    {
        Env.SetByteField(obj, fid, v.toInteger());
    }
        break;
    case TypeSignature::TS::CHAR:
    {
        Env.SetCharField(obj, fid, v.toInteger());
    }
        break;
    case TypeSignature::TS::SHORT:
    {
        Env.SetShortField(obj, fid, v.toInteger());
    }
        break;
    case TypeSignature::TS::INT:
    {
        Env.SetIntField(obj, fid, v.toInteger());
    }
        break;
    case TypeSignature::TS::LONG:
    {
        Env.SetLongField(obj, fid, v.toInteger());
    }
        break;
    case TypeSignature::TS::FLOAT:
    {
        Env.SetFloatField(obj, fid, v.toNumber());
    }
        break;
    case TypeSignature::TS::DOUBLE:
    {
        Env.SetDoubleField(obj, fid, v.toNumber());
    }
        break;
    case TypeSignature::TS::STRING:
    {
        Env.SetStringField(obj, fid, v.toString());
    }
        break;
    case TypeSignature::TS::OBJECT:
    case TypeSignature::TS::UNKNOWN:
    case TypeSignature::TS::VOID:
    case TypeSignature::TS::CLASS:
    case TypeSignature::TS::BYTEARRAY:
    {
        Env.SetObjectField(obj, fid, v.toObject());
    }
        break;
    }
}

return_type JConstructMethod::operator()() const
{
    return invoke({});
}

return_type JConstructMethod::operator()(arg_type const& v) const
{
    return invoke({ &v });
}

return_type JConstructMethod::operator()(arg_type const& v, arg_type const& v1) const
{
    return invoke({ &v, &v1 });
}

return_type
JConstructMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2) const
{
    return invoke({ &v, &v1, &v2 });
}

return_type JConstructMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3) const
{
    return invoke({ &v, &v1, &v2, &v3 });
}

return_type JConstructMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4 });
}

return_type JConstructMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4,
    arg_type const& v5) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5 });
}

return_type JConstructMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5, &v6 });
}

return_type JConstructMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6, arg_type const& v7) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5, &v6, &v7 });
}

return_type JConstructMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6, arg_type const& v7,
    arg_type const& v8) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8 });
}

return_type JStaticMethod::operator()() const
{
    return invoke({});
}

return_type JStaticMethod::operator()(arg_type const& v) const
{
    return invoke({ &v });
}

return_type JStaticMethod::operator()(arg_type const& v, arg_type const& v1) const
{
    return invoke({ &v, &v1 });
}

return_type
JStaticMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2) const
{
    return invoke({ &v, &v1, &v2 });
}

return_type JStaticMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3) const
{
    return invoke({ &v, &v1, &v2, &v3 });
}

return_type JStaticMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4 });
}

return_type JStaticMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4,
    arg_type const& v5) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5 });
}

return_type JStaticMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5, &v6 });
}

return_type JStaticMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6, arg_type const& v7) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5, &v6, &v7 });
}

return_type JStaticMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6, arg_type const& v7,
    arg_type const& v8) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8 });
}

JMethod::JMethod(JClass& clz)
    : _clazz(clz)
{
}

string JMethod::signature(args_type const& args, args_signatures_typep const& predefs) const
{
    return Signature(args, sreturn, predefs);
}

string JMethod::Signature(args_type const& args, JTypeSignature const& sreturn,
    args_signatures_typep const& predefs)
{
    if (predefs)
    {
        ::std::vector<string> tss(predefs->begin(), predefs->end());
        string sig = "(" + ::CROSS_NS::implode(tss, "") + ")" + sreturn;
        return sig;
    }

    ::std::vector<string> ps;
    for (auto& e : args)
    {
        ps.emplace_back(e->signature());
    }

    string sig = "(" + ::CROSS_NS::implode(ps, "") + ")" + sreturn;
    return sig;
}

return_type JMemberMethod::operator()(JObject& obj) const
{
    return invoke(obj, {});
}

return_type JMemberMethod::operator()(JObject& obj, arg_type const& v) const
{
    return invoke(obj, { &v });
}

return_type JMemberMethod::operator()(JObject& obj, arg_type const& v, arg_type const& v1) const
{
    return invoke(obj, { &v, &v1 });
}

return_type JMemberMethod::operator()(JObject& obj, arg_type const& v, arg_type const& v1,
    arg_type const& v2) const
{
    return invoke(obj, { &v, &v1, &v2 });
}

return_type
JMemberMethod::operator()(JObject& obj, arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3) const
{
    return invoke(obj, { &v, &v1, &v2, &v3 });
}

return_type
JMemberMethod::operator()(JObject& obj, arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4) const
{
    return invoke(obj, { &v, &v1, &v2, &v3, &v4 });
}

return_type
JMemberMethod::operator()(JObject& obj, arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5) const
{
    return invoke(obj, { &v, &v1, &v2, &v3, &v4, &v5 });
}

return_type
JMemberMethod::operator()(JObject& obj, arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6) const
{
    return invoke(obj, { &v, &v1, &v2, &v3, &v4, &v5, &v6 });
}

return_type
JMemberMethod::operator()(JObject& obj, arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6, arg_type const& v7) const
{
    return invoke(obj, { &v, &v1, &v2, &v3, &v4, &v5, &v6, &v7 });
}

return_type
JMemberMethod::operator()(JObject& obj, arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6, arg_type const& v7, arg_type const& v8) const
{
    return invoke(obj, { &v, &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8 });
}

return_type JConstructMethod::invoke(args_type const& args) const
{
    string sig = signature(args, sargs);
    JValues jvals(args);

    auto mid = Env.GetMethodID(_clazz, "<init>", sig);
    if (!mid)
    {
        Env.ExceptionClear();
        Logger::Error("没有找到构造函数 " + name + " " + sig);
        return nullptr;
    }

    return _V(Env.NewObject(_clazz, mid, jvals));
}

return_type JStaticMethod::invoke(args_type const& args) const
{
    string sig = signature(args, sargs);
    JValues jvals(args);

    auto mid = Env.GetStaticMethodID(_clazz, name, sig);
    if (!mid)
    {
        Env.ExceptionClear();
        Logger::Error("没有找到函数 " + name + sig);
        return nullptr;
    }

    switch (TypeSignature::GetTypeForSwitch(sreturn))
    {
    case TypeSignature::TS::BOOLEAN:
        return _V((bool)Env.CallStaticBooleanMethod(_clazz, mid, jvals));
    case TypeSignature::TS::BYTE:
        return _V(Env.CallStaticByteMethod(_clazz, mid, jvals));
    case TypeSignature::TS::CHAR:
        return _V(Env.CallStaticCharMethod(_clazz, mid, jvals));
    case TypeSignature::TS::SHORT:
        return _V(Env.CallStaticShortMethod(_clazz, mid, jvals));
    case TypeSignature::TS::INT:
        return _V(Env.CallStaticIntMethod(_clazz, mid, jvals));
    case TypeSignature::TS::LONG:
        return _V(Env.CallStaticLongMethod(_clazz, mid, jvals));
    case TypeSignature::TS::FLOAT:
        return _V(Env.CallStaticFloatMethod(_clazz, mid, jvals));
    case TypeSignature::TS::DOUBLE:
        return _V(Env.CallStaticDoubleMethod(_clazz, mid, jvals));
    case TypeSignature::TS::STRING:
    {
        auto v = Env.CallStaticStringMethod(_clazz, mid, jvals);
        if (v == nullptr)
        {
            if (!nullable)
            {
                if (ExceptionGuard::Check())
                {
                    Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 遇到异常: " +
                        ExceptionGuard::GetLastErrorMessage());
                }
                else
                {
                    Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 返回null");
                }
            }
            return nullptr;
        }
        return _V(*v);
    }
    case TypeSignature::TS::BYTEARRAY:
    {
        auto v = Env.CallStaticArrayMethod(_clazz, mid, jvals);
        if (v == nullptr)
        {
            if (!nullable)
            {
                if (ExceptionGuard::Check())
                {
                    Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 遇到异常: " +
                        ExceptionGuard::GetLastErrorMessage());
                }
                else
                {
                    Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 返回null");
                }
            }
            return nullptr;
        }
        return _V(v->toString());
    }
    case TypeSignature::TS::VOID:
    {
        Env.CallStaticVoidMethod(_clazz, mid, jvals);
        return nullptr;
    }
    case TypeSignature::TS::UNKNOWN:
    case TypeSignature::TS::CLASS:
    case TypeSignature::TS::OBJECT:
        break;
    }

    auto v = Env.CallStaticObjectMethod(_clazz, mid, jvals);
    if (v == nullptr)
    {
        if (!nullable)
        {
            if (ExceptionGuard::Check())
            {
                Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 遇到异常: " +
                    ExceptionGuard::GetLastErrorMessage());
            }
            else
            {
                Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 返回null");
            }
        }
        return nullptr;
    }
    return JVariant::FromObject(*v);
}

return_type JMemberMethod::invoke(JObject& obj, args_type const& args) const
{
    string sig = signature(args, sargs);
    JValues jvals(args);

    auto mid = Env.GetMethodID(_clazz, name, sig);
    if (!mid)
    {
        Env.ExceptionClear();
        Logger::Error("没有找到函数 " + name + sig);
        return nullptr;
    }

    switch (TypeSignature::GetTypeForSwitch(sreturn))
    {
    case TypeSignature::TS::BOOLEAN:
        return _V((bool)Env.CallBooleanMethod(obj, mid, jvals));
    case TypeSignature::TS::BYTE:
        return _V(Env.CallByteMethod(obj, mid, jvals));
    case TypeSignature::TS::CHAR:
        return _V(Env.CallCharMethod(obj, mid, jvals));
    case TypeSignature::TS::SHORT:
        return _V(Env.CallShortMethod(obj, mid, jvals));
    case TypeSignature::TS::INT:
        return _V(Env.CallIntMethod(obj, mid, jvals));
    case TypeSignature::TS::LONG:
        return _V(Env.CallLongMethod(obj, mid, jvals));
    case TypeSignature::TS::FLOAT:
        return _V(Env.CallFloatMethod(obj, mid, jvals));
    case TypeSignature::TS::DOUBLE:
        return _V(Env.CallDoubleMethod(obj, mid, jvals));
    case TypeSignature::TS::STRING:
    {
        auto s = Env.CallStringMethod(obj, mid, jvals);
        if (s == nullptr)
        {
            if (!nullable)
            {
                if (ExceptionGuard::Check())
                {
                    Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 遇到异常: " +
                        ExceptionGuard::GetLastErrorMessage());
                }
                else
                {
                    Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 返回null");
                }
            }
            return nullptr;
        }
        return _V(*s);
    }
    case TypeSignature::TS::BYTEARRAY:
    {
        auto v = Env.CallArrayMethod(obj, mid, jvals);
        if (v == nullptr)
        {
            if (!nullable)
            {
                if (ExceptionGuard::Check())
                {
                    Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 遇到异常: " +
                        ExceptionGuard::GetLastErrorMessage());
                }
                else
                {
                    Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 返回null");
                }
            }
            return nullptr;
        }
        return _V(v->toString());
    }
    case TypeSignature::TS::VOID:
    {
        Env.CallVoidMethod(obj, mid, jvals);
        return nullptr;
    }
    case TypeSignature::TS::CLASS:
    case TypeSignature::TS::OBJECT:
    case TypeSignature::TS::UNKNOWN:
        break;
    }

    auto v = Env.CallObjectMethod(obj, mid, jvals);
    if (v == nullptr)
    {
        if (!nullable)
        {
            if (ExceptionGuard::Check())
            {
                Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 遇到异常: " +
                    ExceptionGuard::GetLastErrorMessage());
            }
            else
            {
                Logger::Critical("调用Java层方法 " + name + "@" + _clazz.name() + " 返回null");
            }
        }
        return nullptr;
    }
    return JVariant::FromObject(*v);
}

JClass::JClass(JClassPath const& cp)
    : _clazzpath(cp), construct(*this)
{
    if (!cp.empty())
    {
        auto clz = Env.FindClass(cp);
        if (clz == nullptr)
        {
            Logger::Critical("没有找到类 " + cp);
        }
        else
        {
            _clazz = clz->_clazz;
        }
    }
}

JClassName JClass::name() const
{
    return _clazzpath;
}

JClassPath const& JClass::path() const
{
    return _clazzpath;
}

bool JClass::exists() const
{
    return !_clazz.isnil();
}

void JClass::_asglobal()
{
    _clazz._asglobal();
}

class JContextPrivate
{
public:

    JContextPrivate()
        : index_functions(0)
    {
    }

    class FunctionType
    {
    public:

        typedef shared_ptr<JVariant::function_type> function_type;

        FunctionType(function_type _fn)
            : fn(_fn), referencedCount(1)
        {
        }

        function_type fn;
        ::std::atomic<size_t> referencedCount;
    };

    typedef shared_ptr<FunctionType> function_type;
    typedef ::std::map<size_t, function_type> functions_type;
    functions_type functions;

    ::std::atomic<size_t> index_functions;
    ::std::mutex mtx_functions;

    // 全局注册类列表
    typedef ::std::map<JClassPath, JContext::class_typep> classes_type;
    classes_type classes;
};

JContext::JContext()
{
    NNT_CLASS_CONSTRUCT();
}

JContext::~JContext()
{
    NNT_CLASS_DESTORY();
}

bool JContext::add(class_typep const& cls)
{
    if (!cls->exists())
        return false;

    // 将类提权到global
    cls->_asglobal();

    // 添加到线程级的全局类管理器中
    d_ptr->classes[cls->path()] = cls;
    return true;
}

JContext::class_typep JContext::find_class(JClassPath const& ph) const
{
    auto const& clss = d_ptr->classes;
    auto fnd = clss.find(ph);
    return fnd == clss.end() ? nullptr : fnd->second;
}

void JContext::clear()
{
    d_ptr->classes.clear();
    d_ptr->functions.clear();
    d_ptr->index_functions = 0;
}

size_t JContext::add(shared_ptr<function_type> const& fn)
{
    size_t idx = ++d_ptr->index_functions;
    NNT_AUTOGUARD(d_ptr->mtx_functions);
    d_ptr->functions[idx] = make_shared<private_class_type::FunctionType>(fn);
    return idx;
}

void JContext::function_grab(function_index_type fnid)
{
    NNT_AUTOGUARD(d_ptr->mtx_functions);
    auto fnd = d_ptr->functions.find(fnid);
    if (fnd != d_ptr->functions.end())
    {
        ++fnd->second->referencedCount;
    }
    else
    {
        Logger::Error("没有找到函数索引 " + ::CROSS_NS::tostr((int)fnid));
    }
}

bool JContext::function_drop(function_index_type fnid)
{
    NNT_AUTOGUARD(d_ptr->mtx_functions);
    auto fnd = d_ptr->functions.find(fnid);
    if (fnd != d_ptr->functions.end())
    {
        if (--fnd->second->referencedCount == 0)
        {
            d_ptr->functions.erase(fnd);
            return true;
        }
    }
    else
    {
        Logger::Error("没有找到函数索引 " + ::CROSS_NS::tostr((int)fnid));
    }
    return false;
}

shared_ptr<JContext::function_type> JContext::find_function(function_index_type fnid) const
{
    NNT_AUTOGUARD(d_ptr->mtx_functions);
    auto fnd = d_ptr->functions.find(fnid);
    return fnd == d_ptr->functions.end() ? nullptr : fnd->second->fn;
}

string tostr(jstring str)
{
    Env.Check();
    return Env.GetStringUTFChars(str);
}

AJNI_END
