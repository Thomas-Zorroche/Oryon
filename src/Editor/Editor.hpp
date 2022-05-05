#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Framebuffer.hpp"
#include <memory>
#include <vector>

#include "../Events/Event.hpp"

#include "Panel.hpp"

// TEMP
#include "GLRenderer/Scene/Scene.hpp"
#include "GLRenderer/Scene/Entity.hpp"
#include "../CameraController.hpp"



namespace oryon
{

	class Editor
	{
	public:
		Editor();

		void initialize(GLFWwindow* window);

		void onUpdate();

		void free();

		void onEvent(Event& e);

	private:
		void renderViewer3DPanel();
		void renderWorldOutliner();
		void renderObjectPanel();
		void renderLightPanel();
		void renderMaterialPanel();

		void renderMenuBar();

		void renderFramebuffer();

		void setupDockspace();

		void onEntitySelectedChanged();

		void nextGuizmoType();

	private:
		bool _dockspaceOpen = true;
	
		std::unique_ptr<Framebuffer> _renderingFramebuffer = nullptr;
		std::unique_ptr<Framebuffer> _depthFramebuffer = nullptr;

		float _viewportWidth = 500.0f;
		float _viewportHeight = 300.0f;

		std::shared_ptr<glrenderer::Scene> _scene = std::make_shared<glrenderer::Scene>();
		std::shared_ptr<CameraController> _cameraController;

		glrenderer::Entity _entitySelected;
		std::string _bufferEntitySelectedName = "";

		int _guizmoType = -1;

		std::vector<Panel> _panels;
	};


}