#include "KGLContext.h"

#include <cstdio>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

namespace kuma::gl
{

bool KGLContext::Init(GLFWwindow* Window)
{
    if (!Window)
    {
        std::fprintf(stderr, "[KGLContext] null window handle\n");
        return false;
    }

    glfwMakeContextCurrent(Window);

    int Version = gladLoadGL(glfwGetProcAddress);
    if (Version == 0)
    {
        std::fprintf(stderr, "[KGLContext] Failed to initialize OpenGL context (gladLoadGL)\n");
        return false;
    }

    int Major = Version / 10;
    int Minor = Version % 10;
    std::printf("[KGLContext] OpenGL %d.%d loaded: %s\n", Major, Minor, glGetString(GL_VENDOR));
    std::printf("[KGLContext] Renderer: %s\n", glGetString(GL_RENDERER));
    std::printf("[KGLContext] Version: %s\n", glGetString(GL_VERSION));
    return true;
}

} // namespace kuma::gl
