package dev.anastasioscho.glestriangle

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import java.io.File

class MainActivity: AppCompatActivity() {
    private val nativeLibrary = NativeLibrary()

    private lateinit var customGLSurfaceView: CustomGLSurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)

        customGLSurfaceView = findViewById(R.id.customGLSurfaceView)

        extractFiles()
        nativeLibrary.loadTextureImageFile(filesDir.absolutePath)
    }

    private fun extractFiles() {
        val fileNames = arrayOf("texture-wood.jpg")
        val filesDir = filesDir.absolutePath
        val assetManager = assets

        for (fileName in fileNames) {
            val file = File(filesDir, fileName)

            if (!file.exists()) {
                val bytes = assetManager.open(fileName).readBytes()
                file.writeBytes(bytes)
            }
        }
    }
}
