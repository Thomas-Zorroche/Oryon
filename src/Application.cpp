#include "Application.hpp"
#include "Events/Input.hpp"

#include <GLFW/glfw3.h>


namespace oryon
{

Application::Application(int argc, char** argv)
{
	_window = std::make_unique<Window>(argc, argv, [this](Event& e) {this->onEvent(e); });
	_editor = std::make_unique<Editor>();
}

void Application::run()
{
	float deltaTime = 0.0f;	// Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame

	Input::setWindow(_window->GetNativeWindow());
	_editor->initialize(_window->GetNativeWindow());

	while (!glfwWindowShouldClose(_window->GetNativeWindow()))
	{
		float currentFrame = glfwGetTime();

		_editor->onUpdate();

		/* Swap front and back buffers */
		glfwSwapBuffers(_window->GetNativeWindow());

		/* Poll for and process events */
		glfwPollEvents();
	}


	_editor->free();

}

void Application::onEvent(Event& e)
{
	_editor->onEvent(e);
}


}