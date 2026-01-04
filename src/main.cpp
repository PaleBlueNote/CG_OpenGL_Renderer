#define GLEW_STATIC
#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

#include "../dependencies/include/GL/glew.h"
#include "../dependencies/include/GLFW/glfw3.h"
#include "../dependencies/include/shader.h"
#include "../dependencies/include/j3a.hpp"
#include "../dependencies/include/stb_image.h"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

#pragma comment (lib, "../dependencies/lib/glew32s.lib")
#pragma comment (lib, "../dependencies/lib/glfw3.lib")
#pragma comment (lib, "opengl32.lib")

const float PI = 3.14159265358979;
GLuint vertexBuffer = 0;
GLuint normalBuffer = 0;
GLuint texCoordBuffer = 0;
GLuint vertexArray = 0;
GLuint program = 0;
GLuint indices = 0;
GLuint diffTex = 0;
GLuint bumpTex = 0;

float theta = 0, phi = 0, camDist = 5;
float fovy = 1.047f;
double lastX, lastY;

vec3 lightPos(10, 20, 10);
vec3 lightIntensity(1);

void render(GLFWwindow* window);
void init();

void cusorPosCB(GLFWwindow* window, double xpos, double ypos) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        theta = theta - (ypos - lastY) / height * PI;
        if (theta < -PI / 2 + 0.000001) theta = -PI / 2 + 0.000001;
        if (theta > PI / 2 - 0.000001) theta = PI / 2 - 0.000001;
        phi = phi - (xpos - lastX) / width * PI;
        //printf("%f %f\n", phi, theta);
    }
    lastX = xpos;
    lastY = ypos;
};

void buttonCB(GLFWwindow* window, int button, int action, int mods) {
    glfwGetCursorPos(window, &lastX, &lastY);
};

void scrollCB(GLFWwindow* window, double xoffset, double yoffset) {
    float zoomSpeed = 0.05f;
    fovy -= (float)yoffset * zoomSpeed;
    if (fovy < 0.1f) fovy = 0.1f;
    if (fovy > 3.0f) fovy = 3.0f;

    //camDist = camDist * pow(1.01, yoffset);
};

int main(void) {
    if (!glfwInit())
        return 0;
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 8);
    GLFWwindow* window = glfwCreateWindow(640, 640, "Yoonseokchan_202020769", NULL, NULL);
    glfwMakeContextCurrent(window);
#ifdef _MSC_VER
    glewInit();
#endif
    glfwSetCursorPosCallback(window, cusorPosCB);
    glfwSetMouseButtonCallback(window, buttonCB);
    glfwSetScrollCallback(window, scrollCB);
    glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST);
    init();
    while (!glfwWindowShouldClose(window)) {
        render(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

void render(GLFWwindow* window) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    vec3 cameraPos = vec3(0, 0, camDist);
    cameraPos = rotate(phi, vec3(0, 1, 0)) * rotate(theta, vec3(1, 0, 0)) * vec4(cameraPos, 1);

    mat4 modelMat = mat4(1);
    mat4 viewMat = lookAt(cameraPos, vec3(0, 0, 0), vec3(0, 1, 0));
    mat4 projMat = perspective(fovy, width / (float)height, 0.01f, 100.f);

    //mat4 projMat = perspective(radians(50.f), width / (float)height, 0.1f, 100.f);

    glViewport(0, 0, width, height);
    glClearColor(.02, .07, 0.69, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    GLint modelMatLoc = glGetUniformLocation(program, "modelMat");
    GLint viewMatLoc = glGetUniformLocation(program, "viewMat");
    GLint projMatLoc = glGetUniformLocation(program, "projMat");

    glUniformMatrix4fv(modelMatLoc, 1, false, value_ptr(modelMat));
    glUniformMatrix4fv(viewMatLoc, 1, false, value_ptr(viewMat));
    glUniformMatrix4fv(projMatLoc, 1, false, value_ptr(projMat));

    GLuint lightColorLoc = glGetUniformLocation(program, "lightIntensity");
    GLuint lightPosLoc = glGetUniformLocation(program, "lightPos");
    GLuint cameraPosLoc = glGetUniformLocation(program, "cameraPos");
    GLuint diffColorLoc = glGetUniformLocation(program, "diffColor");
    GLuint shininessLoc = glGetUniformLocation(program, "shininess");

    glUniform3fv(lightColorLoc, 1, value_ptr(lightIntensity));
    glUniform3fv(lightPosLoc, 1, value_ptr(lightPos));
    glUniform3fv(cameraPosLoc, 1, value_ptr(cameraPos));
    glUniform3fv(diffColorLoc, 1, value_ptr(diffuseColor[0]));
    glUniform1f(shininessLoc, shininess[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffTex);
    GLint loc = glGetUniformLocation(program, "diffTex");
    glUniform1i(loc, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bumpTex);
    loc = glGetUniformLocation(program, "bumpTex");
    glUniform1i(loc, 1);

    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
    glDrawElements(GL_TRIANGLES, nTriangles[0]*3, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
}

GLuint loadTexture(std::string filename) {
    GLuint textureID = 0;
    int w = 0, h = 0, n = 0;
    stbi_set_flip_vertically_on_load(true);
    void* buf = stbi_load(filename.c_str(), &w, &h, &n, 4);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(buf);

    return textureID;
}

void init() {
    program = loadShaders("shader.vs", "shader.fs");
    loadJ3A("bunny.j3a");

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, nVertices[0]*sizeof(glm::vec3), vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &texCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(glm::vec2), texCoords[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(glm::vec3), normals[0], GL_STATIC_DRAW);

    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    unsigned int triangle[3] = { 0,1,2 };
    glGenBuffers(1, &indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTriangles[0]*sizeof(glm::u32vec3), triangles[0], GL_STATIC_DRAW);

    diffTex = loadTexture(("C:/openglTest/openglTest/" + diffuseMap[0]));
    bumpTex = loadTexture(("C:/openglTest/openglTest/" + bumpMap[0]));
}