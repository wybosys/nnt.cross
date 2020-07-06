package com.nnt.cross

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.getkeepsafe.relinker.ReLinker

class MainActivity() : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // 加载so
        ReLinker.loadLibrary(this, "tester")

        // 创建lua环境
        Context.shared.create()

        // 执行测试
        test0()
        test1()
    }

    fun test0() {
        // 加载包中的lua脚本
        val stm = resources.openRawResource(R.raw.test0)
        Context.shared.load(stm.readBytes())

        // 获得脚本中的函数
        val obj = Context.shared.global("Test0")!!
        var r = obj.call()
        r = obj.call(123)
        r = obj.call(null)
    }

    fun test1() {
        val stm = resources.openRawResource(R.raw.test1)
        Context.shared.load(stm.readBytes())

        // 测试socket
        val obj = Context.shared.global("Test1")!!
        obj.call()
    }
}
