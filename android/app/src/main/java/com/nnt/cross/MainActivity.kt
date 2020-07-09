package com.nnt.cross

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.getkeepsafe.relinker.ReLinker

class MainActivity() : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // 加载so
        ReLinker.loadLibrary(this, "tester")

        // 执行测试
        test()
        test_sys()
        test_thread()
        test_time()
    }

    private external fun test()
    private external fun test_sys()
    private external fun test_thread()
    private external fun test_time()
}
