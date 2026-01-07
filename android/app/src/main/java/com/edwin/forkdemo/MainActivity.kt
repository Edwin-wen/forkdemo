package com.edwin.forkdemo

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import com.edwin.forkdemo.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.setOnClickListener {
            var pid = testfork()
            if (pid == 0) {
                Log.d("ForkLog", "java子进程：" + android.os.Process.myPid())
            } else {
                Log.d("ForkLog", "java父进程：" + android.os.Process.myPid())
            }
        }
    }

    /**
     * A native method that is implemented by the 'forkdemo' native library,
     * which is packaged with this application.
     */
    external fun testfork(): Int

    companion object {
        // Used to load the 'forkdemo' library on application startup.
        init {
            System.loadLibrary("forkdemo")
        }
    }
}