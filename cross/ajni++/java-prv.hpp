#ifndef __AJNI_PRIVATE_GLOBAL_H_INCLUDED
#define __AJNI_PRIVATE_GLOBAL_H_INCLUDED

#ifndef __AJNI_PRIVATE__
#error "禁止在外部引用该文件"
#endif

#include <cross/cross.hpp>
#include <cross/sys.hpp>

AJNI_BEGIN

// 推出后自动释放的控制
class JEnvThreadAutoGuard
{
public:

    JEnvThreadAutoGuard();
    ~JEnvThreadAutoGuard();

    // 绑定环境
    void bind();

    // 释放环境
    void free();

    // 推出线程时是否需要释放env
    bool detach = false;

    // 是否是主线程
    bool ismain = false;

    // 当前线程的env数据
    JNIEnv *env = nullptr;

    // 当前线程遇到的最后错误信息
    string errmsg;

    // 当前线程id
    const ::CROSS_NS::tid_t tid = ::CROSS_NS::get_thread_id();

    // 检查当前环境状态
    void check();

    // 清理所有
    static void Clear();
};

// 正在执行全局初始化
extern bool gs_during_init;

// jni绑定主jvm对象
extern JavaVM *gs_vm;

// 业务层主activity对象
extern jobject gs_activity;

// 业务层android上下文
extern jobject gs_context;

namespace TypeSignature {

    enum struct TS {
        UNKNOWN,
        CLASS,
        STRING,
        OBJECT,
        BOOLEAN,
        BYTE,
        CHAR,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE,
        VOID,
        BYTEARRAY
    };

    // 简化switch写法的工具函数
    extern TS GetTypeForSwitch(JTypeSignature const&);
}

AJNI_END

#endif
