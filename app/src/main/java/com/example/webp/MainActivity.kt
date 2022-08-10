package com.example.webp

import android.media.MediaMuxer
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import java.io.File

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    override fun onResume() {
        super.onResume()
        val file = File(filesDir.path+"/144.webp")
        Log.d("", file.exists().toString())
        val bitmap = WebPDecoder.instance!!.decodeWebP(filesDir.path + "/144.webp")
        Log.d("d", "e")
    }
}