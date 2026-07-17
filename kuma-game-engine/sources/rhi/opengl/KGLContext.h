#pragma once

struct GLFWwindow;

namespace kuma::gl
{

class KGLContext
{
public:
    static bool Init(GLFWwindow* Window);
};

} // namespace kuma::gl
