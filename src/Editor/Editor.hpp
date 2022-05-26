#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "GLRenderer/Framebuffer.hpp"
#include <memory>
#include <vector>
#include <functional>

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

	void Initialize(GLFWwindow* window, 
		const std::shared_ptr<class glrenderer::RendererContext>& rendererContext, 
		const std::shared_ptr<class glrenderer::Scene>& scene,
		const std::shared_ptr<class glrenderer::Camera>& camera);

	void OnUpdate();

	void Draw();

	void Free();

	void OnEvent(Event& e);

	std::vector<Panel>& GetPanels() { return _panels; }

	const glrenderer::Entity& GetEntitySelected() const { return _entitySelected; }

public:
// Events
	// Scene
	using ImportModelCallback = std::function<bool(const std::string&)>;
	using RenameEntityCallback = std::function<void(glrenderer::Entity& entity, const std::string& name)>;
	using CreateEntityCallback = std::function<void(glrenderer::EBaseEntityType)>;
	ImportModelCallback SC_ImportModel;
	RenameEntityCallback SC_RenameEntity;
	CreateEntityCallback SC_CreateEntity;

	// RendererContext
	using ResizeRenderBufferCallback = std::function<void(uint32_t, uint32_t)>;
	using GetViewportBufferCallback = std::function<unsigned int()>;
	ResizeRenderBufferCallback RC_ResizeRenderBuffer;
// End of events

private:
	void renderViewer3DPanel();
	void renderWorldOutliner();
	void renderObjectPanel();
	void renderLightPanel();
	void renderMaterialPanel();
	void renderPerformancePanel();

	void renderMenuBar();

	void setupDockspace();

	void onEntitySelectedChanged();

	void nextGuizmoType();

private:
	bool _dockspaceOpen = true;
	
	float _viewportWidth = 500.0f;
	float _viewportHeight = 300.0f;

	std::shared_ptr<CameraController> _cameraController;

	glrenderer::Entity _entitySelected;

	std::string _bufferEntitySelectedName = "";

	int _guizmoType = -1;

	std::vector<Panel> _panels;

	uint32_t _renderBufferTextureID = 0;
};

}