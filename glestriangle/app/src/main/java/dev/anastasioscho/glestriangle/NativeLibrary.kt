package dev.anastasioscho.glestriangle

class NativeLibrary {
    companion object {
        init {
            System.loadLibrary("my-library")
        }
    }

    external fun nOnSurfaceCreated()
    external fun nOnSurfaceChanged(width: Int, height: Int)
    external fun nOnDrawFrame()
}
