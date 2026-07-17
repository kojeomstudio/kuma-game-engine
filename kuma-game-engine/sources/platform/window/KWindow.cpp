#include "KWindow.h"

#include <cstdio>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace kuma::platform
{

static void ErrorCallback(int Error, const char* Description)
{
    std::fprintf(stderr, "[GLFW Error %d] %s\n", Error, Description);
}

KWindow::KWindow(int Width, int Height, const std::string& Title) : Width(Width), Height(Height)
{
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit())
    {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    Window = glfwCreateWindow(Width, Height, Title.c_str(), nullptr, nullptr);
    if (!Window)
    {
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(Window);
}

KWindow::~KWindow()
{
    if (Window)
    {
        glfwDestroyWindow(Window);
    }
    glfwTerminate();
}

bool KWindow::ShouldClose() const
{
    return glfwWindowShouldClose(Window) == GLFW_TRUE;
}

void KWindow::SwapBuffers()
{
    glfwSwapBuffers(Window);
}

void KWindow::PollEvents()
{
    glfwPollEvents();
}

bool KWindow::IsKeyPressed(int Key) const
{
    return glfwGetKey(Window, Key) == GLFW_PRESS;
}

} // namespace kuma::platform
