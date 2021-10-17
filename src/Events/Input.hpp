#pragma once

#include <glm/glm.hpp>

#include <GLFW/glfw3.h>

namespace oryon {

	enum KeyCode
	{
		LeftShift = 340,
		LeftCtrl = 341,
		LeftAlt = 342

	};

	enum MouseCode
	{
		ButtonLeft = 0,
		ButtonRight = 1,
		ButtonMiddle = 2
	};

	class Input
	{
	public:
		
		static bool isKeyPressed(KeyCode key);

		static bool isMouseButtonPressed(MouseCode button);

		static glm::vec2 getMousePosition();

		static void setWindow(GLFWwindow* window);

	private:
		static GLFWwindow* _window;
	};

}