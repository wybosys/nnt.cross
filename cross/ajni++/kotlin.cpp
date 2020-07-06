#include "ajni++.hpp"
#define __AJNI_PRIVATE__
#include "jnienv.hpp"
#include "ast.hpp"
#include "kotlin.hpp"
#include "java-prv.hpp"
#include "android-prv.hpp"

AJNI_BEGIN_NS(kotlin)

string capitalize(string const& str)
{
    return (char)toupper(str[0]) + str.substr(1);
}

JClass::JClass(JClassPath const& cp)
    : ::AJNI_NS::JClass(cp)
{
    _classpath$ = cp + "$Companion";

    auto clz = Env.FindClass(_classpath$);
    if (clz)
    {
        _clazz$ = clz;

        // 获取静态对象地址
        ::AJNI_NS::JStaticField sf(*this);
        sf.name = "Companion";
        sf.stype = "L" + _classpath$ + ";";
        _object$ = sf()->toObject();
    }
    else
    {
        // 只有存在 companion object 段的 kotlin类 才存在，不存在也不代表错误
    }
}

JObject const& JClass::object$() const
{
    return *_object$;
}

JClass::jvm_class_type const& JClass::clazz$() const
{
    return *_clazz$;
}

void JClass::_asglobal()
{
    jvm_class_type::_asglobal();
    if (_clazz$)
        _clazz$->_asglobal();
    if (_object$)
        _object$->_asglobal();
}

return_type JStaticMethod::invoke(args_type const& args) const
{
    string sig = signature(args, sargs);
    JValues jvals(args);

    auto const& clazz = dynamic_cast<JClass const&>(_clazz);
    auto const& clz = clazz.clazz$();
    auto const& obj$ = clazz.object$();

    auto mid = Env.GetMethodID(clz, name, sig);
    if (!mid)
    {
        Env.ExceptionClear();
        Logger::Error("没有找到函数 " + name + sig);
        return nullptr;
    }

    switch (TypeSignature::GetTypeForSwitch(sreturn))
    {
    case TypeSignature::TS::BOOLEAN:
        return _V((bool)Env.CallBooleanMethod(obj$, mid, jvals));
    case TypeSignature::TS::BYTE:
        return _V(Env.CallByteMethod(obj$, mid, jvals));
    case TypeSignature::TS::CHAR:
        return _V(Env.CallCharMethod(obj$, mid, jvals));
    case TypeSignature::TS::SHORT:
        return _V(Env.CallShortMethod(obj$, mid, jvals));
    case TypeSignature::TS::INT:
        return _V(Env.CallIntMethod(obj$, mid, jvals));
    case TypeSignature::TS::LONG:
        return _V(Env.CallLongMethod(obj$, mid, jvals));
    case TypeSignature::TS::FLOAT:
        return _V(Env.CallFloatMethod(obj$, mid, jvals));
    case TypeSignature::TS::DOUBLE:
        return _V(Env.CallDoubleMethod(obj$, mid, jvals));
    case TypeSignature::TS::STRING:
    {
        auto s = Env.CallStringMethod(obj$, mid, jvals);
        if (!s)
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
            return nullptr;
        }
        return _V(*s);
    }
    case TypeSignature::TS::BYTEARRAY:
    {
        auto v = Env.CallArrayMethod(obj$, mid, jvals);
        if (!v)
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
            return nullptr;
        }
        return _V(v->toString());
    }
    case TypeSignature::TS::VOID:
    {
        Env.CallVoidMethod(obj$, mid, jvals);
        return nullptr;
    }
    case TypeSignature::TS::CLASS:
    case TypeSignature::TS::OBJECT:
    case TypeSignature::TS::UNKNOWN:
        break;
    }

    auto v = Env.CallObjectMethod(obj$, mid, jvals);
    if (!v)
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
        return nullptr;
    }
    return JVariant::FromObject(*v);
}

JGlobalField::JGlobalField(JClassPath const& cp)
    : _clazz(cp + "Kt")
{
    // pass
}

return_type JGlobalField::operator()() const
{
    auto getname = "get" + capitalize(name);
    string sig = JMethod::Signature({}, stype, {});
    JValues jvals;

    auto mid = Env.GetStaticMethodID(_clazz, getname.c_str(), sig.c_str());
    if (!mid)
    {
        Env.ExceptionClear();
        Logger::Error("没有找到全局变量 " + name + stype);
        return nullptr;
    }

    switch (TypeSignature::GetTypeForSwitch(stype))
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
        if (!v)
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
            return nullptr;
        }
        return _V(*v);
    }
    case TypeSignature::TS::VOID:
    {
        Env.CallStaticVoidMethod(_clazz, mid, jvals);
        return nullptr;
    }
    case TypeSignature::TS::CLASS:
    case TypeSignature::TS::OBJECT:
    case TypeSignature::TS::BYTEARRAY:
    case TypeSignature::TS::UNKNOWN:
        break;
    }

    auto v = Env.CallStaticObjectMethod(_clazz, mid, jvals);
    if (!v)
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
        return nullptr;
    }
    return JVariant::FromObject(*v);
}

void JGlobalField::operator()(JVariant const& v)
{
    auto setname = "set" + capitalize(name);
    string sig = JMethod::Signature({ &v }, TypeSignature::VOID, {});
    JValues jvals({ &v });

    auto mid = Env.GetStaticMethodID(_clazz, setname.c_str(), sig.c_str());
    if (!mid)
    {
        Env.ExceptionClear();
        Logger::Error("没有找到全局变量 " + name + stype);
        return;
    }

    Env.CallStaticVoidMethod(_clazz, mid, jvals);
}

JGlobalMethod::JGlobalMethod(JClassPath const& cp)
    : _clazz(cp + "Kt")
{
}

return_type JGlobalMethod::operator()() const
{
    return invoke({});
}

return_type JGlobalMethod::operator()(arg_type const& v) const
{
    return invoke({ &v });
}

return_type JGlobalMethod::operator()(arg_type const& v, arg_type const& v1) const
{
    return invoke({ &v, &v1 });
}

return_type
JGlobalMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2) const
{
    return invoke({ &v, &v1, &v2 });
}

return_type JGlobalMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3) const
{
    return invoke({ &v, &v1, &v2, &v3 });
}

return_type JGlobalMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4 });
}

return_type JGlobalMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4,
    arg_type const& v5) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5 });
}

return_type JGlobalMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5, &v6 });
}

return_type JGlobalMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6, arg_type const& v7) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5, &v6, &v7 });
}

return_type JGlobalMethod::operator()(arg_type const& v, arg_type const& v1, arg_type const& v2,
    arg_type const& v3, arg_type const& v4, arg_type const& v5,
    arg_type const& v6, arg_type const& v7,
    arg_type const& v8) const
{
    return invoke({ &v, &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8 });
}

return_type JGlobalMethod::invoke(args_type const& args) const
{
    string sig = JMethod::Signature(args, sreturn, sargs);
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
        if (!v)
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
            return nullptr;
        }
        return _V(*v);
    }
    case TypeSignature::TS::VOID:
    {
        Env.CallStaticVoidMethod(_clazz, mid, jvals);
        return nullptr;
    }
    case TypeSignature::TS::BYTEARRAY:
    case TypeSignature::TS::OBJECT:
    case TypeSignature::TS::CLASS:
    case TypeSignature::TS::UNKNOWN:
        break;
    }

    auto v = Env.CallStaticObjectMethod(_clazz, mid, jvals);
    if (!v)
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
        return nullptr;
    }
    return JVariant::FromObject(*v);
}

AJNI_END_NS
