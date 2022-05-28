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

	void OnUpdate(std::shared_ptr<glrenderer::Scene>& scene);

	void Draw();

	void Free();

	void OnEvent(Event& e);

	std::vector<Panel>& GetPanels() { return _panels; }

	const glrenderer::Entity& GetEntitySelected() const { return _entitySelected; }

public:
// Events
	// Scene
	using ImportModelCallback = std::function<bool(const std::string&, const uint32_t&)>;
	using RenameEntityCallback = std::function<void(glrenderer::Entity&, const std::string&)>;
	using CreateEntityCallback = std::function<glrenderer::Entity(glrenderer::EBaseEntityType)>;
	using UpdateLightCallback = std::function<void(const std::vector<std::shared_ptr<glrenderer::PointLight>>&)>;
	using DuplicateCallback = std::function<glrenderer::Entity(glrenderer::Entity)>;
	ImportModelCallback SC_ImportModel;
	RenameEntityCallback SC_RenameEntity;
	CreateEntityCallback SC_CreateEntity;
	UpdateLightCallback SC_UpdateLight;
	DuplicateCallback SC_Duplicate;

	// RendererContext
	using ResizeRenderBufferCallback = std::function<void(uint32_t, uint32_t)>;
	using GetViewportBufferCallback = std::function<unsigned int()>;
	ResizeRenderBufferCallback RC_ResizeRenderBuffer;
// End of events

private:
	void renderViewer3DPanel();
	void renderWorldOutliner(std::shared_ptr<glrenderer::Scene>& scene);
	void renderObjectPanel();
	void renderLightPanel();
	void renderMaterialPanel();
	void renderPerformancePanel();
	void renderParticuleSystemPanel(std::shared_ptr<glrenderer::Scene>& scene);

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

	std::shared_ptr<glrenderer::PointLight> _pointLightSelected = nullptr;

	std::string _bufferEntitySelectedName = "";

	int _guizmoType = 0;

	std::vector<Panel> _panels = {};
	Panel _particuleSystemPanel;

	int _particuleSystemSelectedID = -1;

	uint32_t _renderBufferTextureID = 0;

	std::vector<std::string> _groupLabels = { "default" };

	bool _canDuplicate = true;
};

}