#pragma once

#include <string>

struct GLFWwindow;

namespace kuma::platform
{

class KWindow
{
public:
    KWindow(int Width, int Height, const std::string& Title);
    ~KWindow();

    KWindow(const KWindow&) = delete;
    KWindow& operator=(const KWindow&) = delete;

    bool ShouldClose() const;
    void SwapBuffers();
    void PollEvents();
    bool IsKeyPressed(int Key) const;

    GLFWwindow* GetNativeHandle() { return Window; }

    int GetWidth() const { return Width; }
    int GetHeight() const { return Height; }

private:
    GLFWwindow* Window = nullptr;
    int Width = 0;
    int Height = 0;
};

} // namespace kuma::platform
