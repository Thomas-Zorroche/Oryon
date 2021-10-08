﻿#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Framebuffer.hpp"
#include <memory>

// TEMP
#include "GLRenderer/VertexArray.hpp"
#include "GLRenderer/Shader.h"


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

	// TEMP
	std::shared_ptr<glrenderer::VertexArray> _vertexArray = nullptr;
	std::shared_ptr<glrenderer::Shader> _shader = nullptr;

};


}