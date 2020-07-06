#include "ajni++.hpp"
#include "jnienv.hpp"
#include "ast.hpp"
#include "jre.hpp"

AJNI_BEGIN_NS(jre)

JClassPath const Object::CLASSPATH = "java/lang/Object";

Object::Object(JClassPath const &cp)
    : JClass(cp), toString(*this)
{
    toString.name = "toString";
    toString.sreturn = ::AJNI_NS::TypeSignature::STRING;
}

JClassPath const ClassLoader::CLASSPATH = "java/lang/ClassLoader";

ClassLoader::ClassLoader(JClassPath const &cp)
    : JClass(cp), loadClass(*this)
{
    loadClass.name = "loadClass";
    loadClass.sreturn = ::AJNI_NS::TypeSignature::CLASS;
    loadClass.sargs = make_shared<JMethod::args_signatures_type>();
    loadClass.sargs->emplace_back(::AJNI_NS::TypeSignature::STRING);
}

JClassPath const Throwable::CLASSPATH = "java/lang/Throwable";

Throwable::Throwable(JClassPath const &cp)
    : Object(cp)
{
}

JClassPath const Boolean::CLASSPATH = "java/lang/Boolean";

Boolean::Boolean(JClassPath const &cp)
    : JClass(cp),
      booleanValue(*this)
{
    booleanValue.name = "booleanValue";
    booleanValue.sreturn = ::AJNI_NS::TypeSignature::BOOLEAN;
}

JClassPath const Number::CLASSPATH = "java/lang/Number";

Number::Number(JClassPath const &cp)
    : JClass(cp),
      longValue(*this)
{
    longValue.name = "longValue";
    longValue.sreturn = ::AJNI_NS::TypeSignature::LONG;
}

JClassPath const Float::CLASSPATH = "java/lang/Float";

Float::Float(JClassPath const &cp)
    : Number(cp),
      floatValue(*this)
{
    floatValue.name = "floatValue";
    floatValue.sreturn = ::AJNI_NS::TypeSignature::FLOAT;
}

JClassPath const Double::CLASSPATH = "java/lang/Double";

Double::Double(JClassPath const &cp)
    : Number(cp),
      doubleValue(*this)
{
    doubleValue.name = "doubleValue";
    doubleValue.sreturn = ::AJNI_NS::TypeSignature::DOUBLE;
}

JClassPath const Integer::CLASSPATH = "java/lang/Integer";

Integer::Integer(JClassPath const &cp)
    : Number(cp),
      intValue(*this)
{
    intValue.name = "intValue";
    intValue.sreturn = ::AJNI_NS::TypeSignature::INT;
}

JClassPath const String::CLASSPATH = "java/lang/String";

String::String(JClassPath const &cp)
    : JClass(cp),
      getBytes(*this)
{
    getBytes.name = "getBytes";
    getBytes.sreturn = ::AJNI_NS::TypeSignature::BYTEARRAY;
}

namespace TypeSignature
{
const JTypeSignature CALLBACK = "Lcom/nnt/ajnixx/Callback;";
}

JClassPath const Callback::CLASSPATH = "com/nnt/ajnixx/Callback";

Callback::Callback(JClassPath const &cp)
    : JClass(cp),
      id(*this)
{
    id.name = "id";
    id.stype = ::AJNI_NS::TypeSignature::LONG;
}

AJNI_END_NS
