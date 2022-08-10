package com.example.webp

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import com.example.webp.WebPDecoder
import java.nio.ByteBuffer
import kotlin.math.sqrt

class WebPDecoder private constructor() {
    fun decodeWebP(file: String): Bitmap? {
        val bitmapBytes = decodeWebpFile(file)
        val pixelss = ByteArray(660*860*4)
        bitmapBytes.copyInto(pixelss)
        val pixels = IntArray(660*860)
        ByteBuffer.wrap(pixelss).asIntBuffer()[pixels]
        return Bitmap.createBitmap(pixels, 66, 86, Bitmap.Config.ARGB_8888)
    }

    companion object {
        var instance: WebPDecoder? = null
            get() {
                if (field == null) {
                    synchronized(WebPDecoder::class.java) {
                        if (field == null) {
                            field = WebPDecoder()
                        }
                    }
                }
                return field
            }
            private set

        external fun decodeWebpFile(file: String): ByteArray
    }

    init {
        System.loadLibrary("webp2png")
    }
}