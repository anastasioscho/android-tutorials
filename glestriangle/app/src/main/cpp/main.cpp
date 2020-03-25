//
// Created by Anastasios Chondrogiannis on 2020-02-20.
//

#define STB_IMAGE_IMPLEMENTATION

#import "stb_image.h"
#include <string>
#include <jni.h>
#include <GLES3/gl31.h>
#include <android/log.h>
#include <malloc.h>
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define LOG_TAG "native-library"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

bool linkProgram(GLuint program);
bool validateProgram(GLuint program);

static const GLchar vertexShaderSource[] =
        "#version 310 es\n"
        "layout (location = 0) in vec3 pos;\n"
        "layout (location = 1) in vec2 texCoords;\n"
        "layout (location = 2) in vec3 norm;\n"
        "out vec4 vColor;\n"
        "out vec2 vTexCoords;\n"
        "out vec3 vNorm;\n"
        "out vec3 vFragPos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 view;\n"
        "void main()\n"
        "{\n"
        "gl_Position = projection * view * model * vec4(pos, 1.0);\n"
        "vColor = vec4(clamp(pos, 0.0, 1.0), 1.0);\n"
        "vTexCoords = texCoords;\n"
        "vNorm = mat3(transpose(inverse(model))) * norm;\n"
        "vFragPos = (model * vec4(pos, 1.0)).xyz;\n"
        "}\n";

static const GLchar fragmentShaderSource[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in vec4 vColor;\n"
        "in vec2 vTexCoords;\n"
        "in vec3 vNorm;\n"
        "in vec3 vFragPos;\n"
        "out vec4 color;\n"
        "struct DirectionalLight\n"
        "{\n"
        "vec3 color;\n"
        "float intensity;\n"
        "vec3 direction;\n"
        "float diffuseIntensity;\n"
        "};\n"
        "struct Material\n"
        "{\n"
        "float specularIntensity;\n"
        "float shininess;\n"
        "};\n"
        "uniform sampler2D textureSampler;\n"
        "uniform DirectionalLight directionalLight;\n"
        "uniform Material material;\n"
        "uniform vec3 cameraPosition;\n"
        "void main()\n"
        "{\n"
        "vec4 ambientColor = vec4(directionalLight.color, 1.0f) * directionalLight.intensity;\n"
        "float diffuseFactor = max(dot(normalize(vNorm), normalize(directionalLight.direction)), 0.0f);\n"
        "vec4 diffuseColor = vec4(directionalLight.color, 1.0f) * directionalLight.diffuseIntensity * diffuseFactor;\n"
        "vec4 specularColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"
        "if (diffuseFactor > 0.0f) {\n"
        "vec3 fragToEye = normalize(cameraPosition - vFragPos);\n"
        "vec3 reflectedVertex = normalize(reflect(directionalLight.direction, normalize(vNorm)));\n"
        "float specularFactor = dot(fragToEye, reflectedVertex);\n"
        "if (specularFactor > 0.0f) {\n"
        "specularFactor = pow(specularFactor, material.shininess);"
        "specularColor = vec4(directionalLight.color * material.specularIntensity * specularFactor, 1.0f);\n"
        "}\n"
        "}\n"
        "color = texture(textureSampler, vTexCoords) * (ambientColor + diffuseColor + specularColor);\n"
        "}\n";

GLuint program, triangleVAO, triangleVBO, triangleIBO, texture;
GLint uniformModel, uniformProjection, uniformView;
GLint uniformLightColor, uniformLightIntensity, uniformLightDirection, uniformDiffuseLightIntensity;
GLint uniformMaterialSpecularIntensity, uniformMaterialShininess, uniformCameraPosition;
glm::mat4 projectionMatrix;

unsigned char *texture_image_data;
int texture_image_width;
int texture_image_height;
int texture_image_depth;

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
    uniformView = glGetUniformLocation(program, "view");

    uniformLightColor = glGetUniformLocation(program, "directionalLight.color");
    uniformLightIntensity = glGetUniformLocation(program, "directionalLight.intensity");
    uniformLightDirection = glGetUniformLocation(program, "directionalLight.direction");
    uniformDiffuseLightIntensity = glGetUniformLocation(program, "directionalLight.diffuseIntensity");

    uniformMaterialSpecularIntensity = glGetUniformLocation(program, "material.specularIntensity");
    uniformMaterialShininess = glGetUniformLocation(program, "material.shininess");
    uniformCameraPosition = glGetUniformLocation(program, "cameraPosition");

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

void calculateAverageNormals(unsigned int *indices, unsigned int totalIndices, GLfloat *vertices, unsigned int totalVertices, unsigned int vertexLength, unsigned int normalOffset) {
    for (size_t i = 0; i < totalIndices; i += 3) {
        unsigned int in0 = indices[i] * vertexLength;
        unsigned int in1 = indices[i + 1] * vertexLength;
        unsigned int in2 = indices[i + 2] * vertexLength;

        glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
        glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
        glm::vec3 normal = glm::cross(v1, v2);
        normal = glm::normalize(normal);

        in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;

        vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
        vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
        vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
    }

    for (size_t i = 0; i < (totalVertices / vertexLength); i++) {
        unsigned int nOffset = (i * vertexLength) + normalOffset;
        glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
    }
}

void createTriangle() {
    GLuint indices[] = {
            0, 1, 2,
            0, 2, 3,
            0, 3, 4,
            0, 4, 1
    };

    GLfloat vertices[] = {
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, 1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f
    };

    calculateAverageNormals(indices, 12, vertices, 40, 8, 5);

    glGenVertexArrays(1, &triangleVAO);
    glBindVertexArray(triangleVAO);

    glGenBuffers(1, &triangleIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glGenBuffers(1, &triangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, 40 * sizeof(GL_FLOAT), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (void*)(sizeof(GL_FLOAT) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (void*)(sizeof(GL_FLOAT) * 5));
    glEnableVertexAttribArray(2);

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

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_image_width, texture_image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(texture_image_data);

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

    glUniform3f(uniformLightColor, 1.0f, 1.0f, 1.0f);
    glUniform1f(uniformLightIntensity, 1.0f);
    glUniform3f(uniformLightDirection, 2.0f, 0.3f, -2.0f);
    glUniform1f(uniformDiffuseLightIntensity, 1.0f);

    currentAngle += angleStep;
    if (currentAngle >= 360.0f) currentAngle -= 360.0f;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    float camX = sin(glm::radians(currentAngle)) * 2.5f;
    float camZ = cos(glm::radians(currentAngle)) * 2.5f;

    glm::vec3 cameraPosition = glm::vec3(camX, 0.0f, camZ);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraTarget, worldUp);
    glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glUniform1f(uniformMaterialSpecularIntensity, 1.0f);
    glUniform1f(uniformMaterialShininess, 32.0f);
    glUniform3f(uniformCameraPosition, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIBO);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(0);

    return;
}

extern "C" JNIEXPORT void JNICALL Java_dev_anastasioscho_glestriangle_NativeLibrary_loadTextureImageFile(JNIEnv * env, jobject obj, jstring aFilesDir) {
    const char *files_dir_UTF_chars = env->GetStringUTFChars(aFilesDir, NULL);

    std::string files_dir(files_dir_UTF_chars);

    env->ReleaseStringUTFChars(aFilesDir, files_dir_UTF_chars);

    std::string texture_image_file_path = files_dir + "/texture-wood.jpg";

    texture_image_data = stbi_load(texture_image_file_path.c_str(), &texture_image_width, &texture_image_height, &texture_image_depth, 0);

    if (!texture_image_data) {
        LOGE("Failed to load texture image file: %s", texture_image_file_path.c_str());
    }

    return;
}
