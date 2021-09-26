#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace oryon
{

class Editor
{
public:
	Editor() = default;

	void initialize(GLFWwindow* window);

	void draw() const;

	void free();

private:


};


}