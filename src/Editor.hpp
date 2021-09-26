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

	void draw();

	void free();

private:
	void drawSettingsPanel();

	void drawViewer3DPanel();

	void setupDockspace();


private:
	bool _dockspaceOpen = true;
};


}