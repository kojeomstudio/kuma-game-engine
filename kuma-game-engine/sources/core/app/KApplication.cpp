#include "KApplication.h"

#include "rhi/opengl/KGLContext.h"

namespace kuma::core
{

KApplication::KApplication() : Window(800, 600, "Kuma Engine")
{
    if (!gl::KGLContext::Init(Window.GetNativeHandle()))
    {
    }
}

void KApplication::Run()
{
    while (!Window.ShouldClose())
    {
        Window.PollEvents();
        Renderer.Render();
        Window.SwapBuffers();
    }
}

} // namespace kuma::core
