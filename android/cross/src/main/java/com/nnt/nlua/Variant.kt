package com.nnt.cross

class Object(val idx: Int) {

    // 释放C++层对应的对象
    protected fun finalize() {
        jni_finalize(idx)
    }

    private external fun jni_finalize(idx: Int)

    // 调用函数
    fun call(): Any? {
        return jni_call0(idx)
    }

    private external fun jni_call0(idx: Int): Any?

    fun call(p0: Any?): Any? {
        return jni_call1(idx, p0)
    }

    private external fun jni_call1(idx: Int, p0: Any?): Any?

    fun call(p0: Any?, p1: Any?): Any? {
        return jni_call2(idx, p0, p1)
    }

    private external fun jni_call2(idx: Int, p0: Any?, p1: Any?): Any?

    fun call(p0: Any?, p1: Any?, p2: Any?): Any? {
        return jni_call3(idx, p0, p1, p2)
    }

    private external fun jni_call3(idx: Int, p0: Any?, p1: Any?, p2: Any?): Any?

    fun call(p0: Any?, p1: Any?, p2: Any?, p3: Any?): Any? {
        return jni_call4(idx, p0, p1, p2, p3)
    }

    private external fun jni_call4(idx: Int, p0: Any?, p1: Any?, p2: Any?, p3: Any?): Any?

    fun call(p0: Any?, p1: Any?, p2: Any?, p3: Any?, p4: Any?): Any? {
        return jni_call5(idx, p0, p1, p2, p3, p4)
    }

    private external fun jni_call5(idx: Int, p0: Any?, p1: Any?, p2: Any?, p3: Any?, p4: Any?): Any?

    fun call(p0: Any?, p1: Any?, p2: Any?, p3: Any?, p4: Any?, p5: Any?): Any? {
        return jni_call6(idx, p0, p1, p2, p3, p4, p5)
    }

    private external fun jni_call6(
        idx: Int,
        p0: Any?,
        p1: Any?,
        p2: Any?,
        p3: Any?,
        p4: Any?,
        p5: Any?
    ): Any?

    fun call(p0: Any?, p1: Any?, p2: Any?, p3: Any?, p4: Any?, p5: Any?, p6: Any?): Any? {
        return jni_call7(idx, p0, p1, p2, p3, p4, p5, p6)
    }

    private external fun jni_call7(
        idx: Int,
        p0: Any?,
        p1: Any?,
        p2: Any?,
        p3: Any?,
        p4: Any?,
        p5: Any?,
        p6: Any?
    ): Any?

    fun call(p0: Any?, p1: Any?, p2: Any?, p3: Any?, p4: Any?, p5: Any?, p6: Any?, p7: Any?): Any? {
        return jni_call8(idx, p0, p1, p2, p3, p4, p5, p6, p7)
    }

    private external fun jni_call8(
        idx: Int,
        p0: Any?,
        p1: Any?,
        p2: Any?,
        p3: Any?,
        p4: Any?,
        p5: Any?,
        p6: Any?,
        p7: Any?
    ): Any?

    fun call(
        p0: Any?,
        p1: Any?,
        p2: Any?,
        p3: Any?,
        p4: Any?,
        p5: Any?,
        p6: Any?,
        p7: Any?,
        p8: Any?
    ): Any? {
        return jni_call9(idx, p0, p1, p2, p3, p4, p5, p6, p7, p8)
    }

    private external fun jni_call9(
        idx: Int,
        p0: Any?,
        p1: Any?,
        p2: Any?,
        p3: Any?,
        p4: Any?,
        p5: Any?,
        p6: Any?,
        p7: Any?,
        p8: Any?
    ): Any?
}
