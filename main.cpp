#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GL_ERROR_CASE(glerror)\
    case glerror: snprintf(error, sizeof(error), "%s", #glerror)

void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char* argv[]){
    glfwSetErrorCallback(error_callback);
    GLFWwindow* window;

    if(!glfwInit()){
        return -1;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(640, 480, "Space Invaders", NULL, NULL);
    if(!window){
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    GLenum err = GLEW_OK;
    if(err != GLEW_OK){
        fprintf(stderr, "Error initializing GLEW.\n");
        glfwTerminate();
        return -1;
    }

    int glVersion[2] = {-1, 1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;

}