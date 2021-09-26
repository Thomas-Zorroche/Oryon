
#include "Window.hpp"
#include <string.h>
#include <iostream>

namespace oryon
{

Window::Window(int argc, char** argv)
{
    HandleArgs(argc, argv);
}

int Window::Init()
{
    /* Initialize the library */
    if (!glfwInit())
        return 0;

    /* Create a windowed mode window and its OpenGL context */
    _glfw_Window = glfwCreateWindow((int)_width, (int)_height, "Oryon", NULL, NULL);
    if (!_glfw_Window)
    {
        std::cerr << "GLFW: Failed to create window" << std::endl;
        glfwTerminate();
        return 0;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(_glfw_Window);

    /* Initialize glad: load all OpenGL function pointers */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return 0;
    }

    return 1;
}

/*
* ===============================================================
* Private Functions
* ===============================================================
*/
void Window::HandleArgs(int argc, char** argv)
{
    for (size_t i = 1; i < (size_t)argc; i++)
    {
        if (strcmp(argv[i], "-hd") == 0 || strcmp(argv[i], "-fhd") == 0)
            InitScreenSize(argv[i]);
    }
}

void Window::InitScreenSize(char* size)
{
    if (strcmp(size, "-hd") == 0)
    {
        _width = 1280.0f;
        _height = 720.0f;
    }
    else if (strcmp(size, "-fhd") == 0)
    {
        _width = 1920.0f;
        _height = 1080.0f;
    }
}


} // ns editor