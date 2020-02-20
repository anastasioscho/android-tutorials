package dev.anastasioscho.glestriangle

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity

class MainActivity: AppCompatActivity() {
    private lateinit var customGLSurfaceView: CustomGLSurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)

        customGLSurfaceView = findViewById(R.id.customGLSurfaceView)
    }
}
