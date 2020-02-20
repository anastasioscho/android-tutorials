//
// Created by Anastasios Chondrogiannis on 2020-02-20.
//

#include <jni.h>
#include <GLES3/gl3.h>

extern "C" JNIEXPORT void JNICALL Java_dev_anastasioscho_glestriangle_NativeLibrary_nOnSurfaceCreated(JNIEnv * env, jobject obj) {
    glClearColor(0.0, 1.0, 0.0, 1.0);

    return;
}

extern "C" JNIEXPORT void JNICALL Java_dev_anastasioscho_glestriangle_NativeLibrary_nOnSurfaceChanged(JNIEnv * env, jobject obj, jint width, jint height) {
    glViewport(0, 0, width, height);

    return;
}

extern "C" JNIEXPORT void JNICALL Java_dev_anastasioscho_glestriangle_NativeLibrary_nOnDrawFrame(JNIEnv * env, jobject obj) {
    glClear(GL_COLOR_BUFFER_BIT);

    return;
}
