#ifndef __AJNI_JRE_H_INCLUDED
#define __AJNI_JRE_H_INCLUDED

AJNI_BEGIN_NS(jre)

class Object: public JClass
{
public:
    static JClassPath const CLASSPATH;

    Object(JClassPath const & = CLASSPATH);

    JMemberMethod toString;
};

class ClassLoader: public JClass
{
public:
    static JClassPath const CLASSPATH;

    ClassLoader(JClassPath const & = CLASSPATH);

    JMemberMethod loadClass;
};

class Throwable: public Object
{
public:
    static JClassPath const CLASSPATH;

    Throwable(JClassPath const & = CLASSPATH);
};

class Boolean: public JClass
{
public:
    static JClassPath const CLASSPATH;

    Boolean(JClassPath const & = CLASSPATH);

    JMemberMethod booleanValue;
};

class Number: public JClass
{
public:
    static JClassPath const CLASSPATH;

    Number(JClassPath const & = CLASSPATH);

    JMemberMethod longValue;
};

class Float: public Number
{
public:
    static JClassPath const CLASSPATH;

    Float(JClassPath const & = CLASSPATH);

    JMemberMethod floatValue;
};

class Double: public Number
{
public:
    static JClassPath const CLASSPATH;

    Double(JClassPath const & = CLASSPATH);

    JMemberMethod doubleValue;
};

class Integer: public Number
{
public:
    static JClassPath const CLASSPATH;

    Integer(JClassPath const & = CLASSPATH);

    JMemberMethod intValue;
};

class String: public JClass
{
public:
    static JClassPath const CLASSPATH;

    String(JClassPath const & = CLASSPATH);

    JMemberMethod getBytes;
};

namespace TypeSignature
{
extern const JTypeSignature CALLBACK;
}

class Callback: public JClass
{
public:
    static JClassPath const CLASSPATH;

    Callback(JClassPath const & = CLASSPATH);

    JMemberField id;
};

AJNI_END_NS

#endif
