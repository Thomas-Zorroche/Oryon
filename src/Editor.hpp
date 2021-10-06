#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Framebuffer.hpp"
#include <memory>

namespace oryon
{

class Editor
{
public:
	Editor();

	void initialize(GLFWwindow* window);

	void draw();

	void free();

private:
	void drawSettingsPanel();

	void drawViewer3DPanel();

	void setupDockspace();


private:
	bool _dockspaceOpen = true;
	
	std::unique_ptr<Framebuffer> _framebuffer = nullptr;

	float _viewportWidth = 500.0f;
	float _viewportHeight = 300.0f;
};


}