//
// Created by Anastasios Chondrogiannis on 2020-02-20.
//

#include <jni.h>
#include <GLES3/gl31.h>
#include <android/log.h>
#include <malloc.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define LOG_TAG "native-library"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

bool linkProgram(GLuint program);
bool validateProgram(GLuint program);

static const GLchar vertexShaderSource[] =
        "#version 310 es\n"
        "layout (location = 0) in vec3 pos;\n"
        "out vec4 vColor;\n"
        "uniform mat4 model;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "gl_Position = projection * model * vec4(pos, 1.0);\n"
        "vColor = vec4(clamp(pos, 0.0, 1.0), 1.0);\n"
        "}\n";

static const GLchar fragmentShaderSource[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in vec4 vColor;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "color = vColor;\n"
        "}\n";

GLuint program, triangleVAO, triangleVBO, triangleIBO;
GLint uniformModel, uniformProjection;
glm::mat4 projectionMatrix;

float currentAngle = 0.0f;
float angleStep = 1.0f;

GLuint loadShader(GLenum shaderType, const GLchar* shaderSource) {
    GLuint shader = glCreateShader(shaderType);

    if (shader) {
        glShaderSource(shader, 1, &shaderSource, NULL);
        glCompileShader(shader);

        GLint compileStatus = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

        if (!compileStatus) {
            GLint infoLogLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            if (infoLogLength > 0) {
                GLchar* infoLogBuffer = (GLchar*) malloc(static_cast<size_t>(infoLogLength));

                if (infoLogBuffer != NULL) {
                    glGetShaderInfoLog(shader, infoLogLength, NULL, infoLogBuffer);
                    LOGE("Could not compile the shader %d: %s", shaderType, infoLogBuffer);
                    free(infoLogBuffer);
                }
            }

            glDeleteShader(shader);
            shader = 0;
        }
    }

    return shader;
}

void createProgram() {
    program = 0;

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (!vertexShader || !fragmentShader) {
        LOGE("Could not create the program: vertex or fragment shader failed to compile");
        return;
    }

    program = glCreateProgram();

    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        if (!linkProgram(program) || !validateProgram(program)) {
            LOGE("Could not create the program: program link or validation failed");
            glDeleteProgram(program);
            program = 0;
        }
    }

    uniformModel = glGetUniformLocation(program, "model");
    uniformProjection = glGetUniformLocation(program, "projection");

    return;
}

bool linkProgram(GLuint program) {
    glLinkProgram(program);

    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (!linkStatus) {
        GLint infoLogLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0) {
            GLchar* infoLogBuffer = (GLchar*) malloc(static_cast<size_t >(infoLogLength));

            if (infoLogBuffer != NULL) {
                glGetProgramInfoLog(program, infoLogLength, NULL, infoLogBuffer);
                LOGE("Could not link the program: %s", infoLogBuffer);
                free(infoLogBuffer);
            }
        }

        return false;
    }

    return true;
}

bool validateProgram(GLuint program) {
    glValidateProgram(program);

    GLint validateStatus = GL_FALSE;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);

    if (validateStatus != GL_TRUE) {
        GLint infoLogLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength) {
            GLchar* infoLogBuffer = (GLchar*) malloc(infoLogLength);

            if (infoLogBuffer) {
                glGetProgramInfoLog(program, infoLogLength, NULL, infoLogBuffer);
                LOGE("Could not validate the program: %s", infoLogBuffer);
                free(infoLogBuffer);
            }
        }

        return false;
    }

    return true;
}

void createTriangle() {
    GLuint indices[] = {
            0, 1, 2,
            0, 2, 3,
            0, 3, 4,
            0, 4, 1
    };

    GLfloat vertices[] = {
            0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f
    };

    glGenVertexArrays(1, &triangleVAO);
    glBindVertexArray(triangleVAO);

    glGenBuffers(1, &triangleIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glGenBuffers(1, &triangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GL_FLOAT), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return;
}

extern "C" JNIEXPORT void JNICALL Java_dev_anastasioscho_glestriangle_NativeLibrary_nOnSurfaceCreated(JNIEnv * env, jobject obj) {
    glClearColor(0.0, 1.0, 0.0, 1.0);

    glEnable(GL_DEPTH_TEST);

    createProgram();
    createTriangle();

    return;
}

extern "C" JNIEXPORT void JNICALL Java_dev_anastasioscho_glestriangle_NativeLibrary_nOnSurfaceChanged(JNIEnv * env, jobject obj, jint width, jint height) {
    glViewport(0, 0, width, height);

    projectionMatrix = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

    return;
}

extern "C" JNIEXPORT void JNICALL Java_dev_anastasioscho_glestriangle_NativeLibrary_nOnDrawFrame(JNIEnv * env, jobject obj) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    currentAngle += angleStep;
    if (currentAngle >= 360.0f) currentAngle -= 360.0f;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
    model = glm::rotate(model, glm::radians(currentAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIBO);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(0);

    return;
}
