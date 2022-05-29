
#include "Window.hpp"
#include <string.h>
#include <iostream>



namespace oryon
{

    Window::Window(int argc, char** argv, const EventCallback& callback)
    {
        HandleArgs(argc, argv);
        _windowData.eventCallback = callback;
    }

    int Window::Init()
    {
        /* Initialize the library */
        if (!glfwInit())
            return 0;

        /* Create a windowed mode window and its OpenGL context */
        _glfw_Window = glfwCreateWindow(_windowData.width, _windowData.height, "Oryon", NULL, NULL);
        if (!_glfw_Window)
        {
            std::cerr << "GLFW: Failed to create window" << std::endl;
            glfwTerminate();
            return 0;
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(_glfw_Window);

        /* Disable VSync */
        glfwSwapInterval(0);

        /* Initialize glad: load all OpenGL function pointers */
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return 0;
        }

        /* GLFW callbacks */
        glfwSetWindowUserPointer(_glfw_Window, &_windowData);

        glfwSetKeyCallback(_glfw_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
            case GLFW_PRESS:
            {
                KeyEvent event(key);
                data.eventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                break;
            }
            case GLFW_REPEAT:
            {
                break;
            }
            }
        });

        glfwSetScrollCallback(_glfw_Window, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseScrollEvent event((float)xOffset, (float)yOffset);
            data.eventCallback(event);
        });

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
            _windowData.width = 1280;
            _windowData.height = 720;
        }
        else if (strcmp(size, "-fhd") == 0)
        {
            _windowData.width = 1920;
            _windowData.height = 1080;
        }
    }


} // ns oryon