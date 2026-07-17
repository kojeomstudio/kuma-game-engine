#pragma once

#include "platform/window/KWindow.h"
#include "renderer/KRenderer.h"

namespace kuma::core
{

class KApplication
{
public:
    KApplication();
    ~KApplication() = default;

    KApplication(const KApplication&) = delete;
    KApplication& operator=(const KApplication&) = delete;

    void Run();

private:
    platform::KWindow Window;
    renderer::KRenderer Renderer;
};

} // namespace kuma::core
