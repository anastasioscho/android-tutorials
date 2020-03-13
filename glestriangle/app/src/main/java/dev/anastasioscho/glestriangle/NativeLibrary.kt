package dev.anastasioscho.glestriangle

class NativeLibrary {
    companion object {
        init {
            System.loadLibrary("native-library")
        }
    }

    external fun nOnSurfaceCreated()
    external fun nOnSurfaceChanged(width: Int, height: Int)
    external fun nOnDrawFrame()

    external fun loadTextureImageFile(aFilesDir: String);
}
