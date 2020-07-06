#ifndef __AJNIXX_KOTLIN_H_INCLUDED
#define __AJNIXX_KOTLIN_H_INCLUDED

#include "ast.hpp"

AJNI_BEGIN_NS(kotlin)

using ::AJNI_NS::JMemberField;
using ::AJNI_NS::JMemberMethod;
using ::AJNI_NS::JStaticField;

class JClass : public ::AJNI_NS::JClass
{
public:

    typedef ::AJNI_NS::JClass jvm_class_type;

    JClass(JClassPath const& cp);

    JObject const& object$() const;

    jvm_class_type const& clazz$() const;

protected:

    virtual void _asglobal();

    // kt的comp可以不存在
    shared_ptr<JObject> _object$;
    shared_ptr<jvm_class_type> _clazz$;

    JClassPath _classpath$;

    friend class JEnv;
};

class JStaticMethod : public ::AJNI_NS::JStaticMethod
{
public:

    JStaticMethod(JClass& clz)
    : ::AJNI_NS::JStaticMethod(clz)
    {}

    virtual return_type invoke(args_type const &) const;
};

class JGlobalField
{
public:

    JGlobalField(JClassPath const&);

    // 变量名
    string name;

    // 变量类型
    JTypeSignature stype;

    // get
    return_type operator()() const;

    // set
    void operator()(JVariant const&);

protected:

    ::AJNI_NS::JClass _clazz;
};

class JGlobalMethod
{
public:

    // 此处的classpath其实是文件名路径
    JGlobalMethod(JClassPath const&);

    return_type operator()() const;
    return_type operator()(arg_type const&) const;
    return_type operator()(arg_type const&, arg_type const&) const;
    return_type operator()(arg_type const&, arg_type const&, arg_type const&) const;
    return_type operator()(arg_type const&, arg_type const&, arg_type const&, arg_type const&) const;
    return_type operator()(arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&) const;
    return_type operator()(arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&) const;
    return_type operator()(arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&) const;
    return_type operator()(arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&) const;
    return_type operator()(arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&, arg_type const&) const;

    virtual return_type invoke(args_type const&) const;

    // 函数名
    string name;

    // 返回类型
    JTypeSignature sreturn;

    JMethod::args_signatures_typep sargs;

protected:

    ::AJNI_NS::JClass _clazz;
};

AJNI_END_NS

#endif
