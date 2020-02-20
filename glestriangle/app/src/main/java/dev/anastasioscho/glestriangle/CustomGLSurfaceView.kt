package dev.anastasioscho.glestriangle

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class CustomGLSurfaceView(context: Context, attrs: AttributeSet): GLSurfaceView(context, attrs) {
    private val customGLRenderer = CustomGLRenderer()
    private val nativeLibrary = NativeLibrary()

    init {
        setEGLContextClientVersion(3)
        setRenderer(customGLRenderer)
    }

    inner class CustomGLRenderer: Renderer {
        override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
            nativeLibrary.nOnSurfaceCreated()
        }

        override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
            nativeLibrary.nOnSurfaceChanged(width, height)
        }

        override fun onDrawFrame(gl: GL10?) {
            nativeLibrary.nOnDrawFrame()
        }
    }
}
