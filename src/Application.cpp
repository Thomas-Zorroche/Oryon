#include "Application.hpp"

#include <GLFW/glfw3.h>


namespace oryon
{

Application::Application(int argc, char** argv)
{
	_window = std::make_unique<Window>(argc, argv);
	_editor = std::make_unique<Editor>();
}

void Application::run()
{
	float deltaTime = 0.0f;	// Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame
	glEnable(GL_DEPTH_TEST);

	_editor->initialize(_window->GetNativeWindow());

	while (!glfwWindowShouldClose(_window->GetNativeWindow()))
	{
		float currentFrame = glfwGetTime();

		_editor->draw();

		/* Swap front and back buffers */
		glfwSwapBuffers(_window->GetNativeWindow());

		/* Poll for and process events */
		glfwPollEvents();
	}


	_editor->free();

}

}