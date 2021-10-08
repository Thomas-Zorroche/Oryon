#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace oryon
{

class Window
{
public:
    Window(int argc, char** argv);

    unsigned int Width() const { return _windowData.width; }
    unsigned int Height() const { return _windowData.height; }

    GLFWwindow* GetNativeWindow() { return _glfw_Window; }

    int Init();

private:
    void HandleArgs(int argc, char** argv);
    void InitScreenSize(char* size);

private:
    GLFWwindow* _glfw_Window = nullptr;

    struct WindowData
    {
        unsigned int width = 1920;
        unsigned int height = 1080;
    };

    WindowData _windowData;

};

}   // ns editor




