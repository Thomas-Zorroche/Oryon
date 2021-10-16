#include "Input.hpp"


namespace oryon {

	GLFWwindow* Input::_window = nullptr;

	void Input::setWindow(GLFWwindow* window)
	{
		_window = window;
	}

	bool Input::isKeyPressed(const KeyCode key)
	{
		auto state = glfwGetKey(_window, static_cast<int32_t>(key));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::isMouseButtonPressed(const MouseCode button)
	{
		auto state = glfwGetMouseButton(_window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::getMousePosition()
	{
		double x, y;
		glfwGetCursorPos(_window, &x, &y);
		return glm::vec2(x, y);
	}
}