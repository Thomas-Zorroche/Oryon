#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Framebuffer.hpp"
#include <memory>

// TEMP
#include "GLRenderer/Scene/Scene.hpp"
#include "GLRenderer/Scene/Entity.hpp"
#include "CameraController.hpp"



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
	void drawWorldOutliner();
	void drawObjectPanel();
	void drawLightPanel();
	void drawMaterialPanel();

	void drawMenuBar();

	void renderFramebuffer();

	void setupDockspace();

	void onEntitySelectedChanged();

private:
	bool _dockspaceOpen = true;
	
	std::unique_ptr<Framebuffer> _framebuffer = nullptr;

	float _viewportWidth = 500.0f;
	float _viewportHeight = 300.0f;

	std::shared_ptr<glrenderer::Scene> _scene = std::make_shared<glrenderer::Scene>();
	std::shared_ptr<CameraController> _cameraController;

	glrenderer::Entity _entitySelected;
	std::string _bufferEntitySelectedName = "";
};


}