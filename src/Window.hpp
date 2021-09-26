#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace oryon
{

class Window
{
public:
    Window(int argc, char** argv);

    void SetWidth(float width) { _width = width; }
    void SetHeight(float height) { height = _height; }

    float Width() const { return _width; }
    float Height() const { return _height; }

    GLFWwindow* GetNativeWindow() { return _glfw_Window; }

    int Init();

private:
    void HandleArgs(int argc, char** argv);
    void InitScreenSize(char* size);

private:
    GLFWwindow* _glfw_Window = nullptr;

    float _width = 1920.0f;
    float _height = 1080.0f;
};

}   // ns editor




