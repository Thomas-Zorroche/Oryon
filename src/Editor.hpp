#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Framebuffer.hpp"
#include <memory>

// TEMP
#include "GLRenderer/Mesh.hpp"
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

	void renderFramebuffer();

	void setupDockspace();

	void showImGuiDemoWindow();


private:
	bool _dockspaceOpen = true;
	
	std::unique_ptr<Framebuffer> _framebuffer = nullptr;

	float _viewportWidth = 500.0f;
	float _viewportHeight = 300.0f;

	// TEMP
	//std::shared_ptr<glrenderer::VertexArray> _vertexArray = nullptr;
	std::shared_ptr<glrenderer::Mesh> _mesh = nullptr;

	std::shared_ptr<glrenderer::Shader> _shader = nullptr;
	glm::mat4 _projection;

};


}