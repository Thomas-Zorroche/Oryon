#include "Application.hpp"
#include "Events/Input.hpp"

#include <GLFW/glfw3.h>

#include "GLRenderer/Renderer/ForwardRenderer.hpp"
#include "GLRenderer/Renderer/DeferredRenderer.hpp"

namespace oryon
{

Application::Application(int argc, char** argv)
{
	_window = std::make_unique<Window>(argc, argv, [this](Event& e) {this->OnEvent(e); });
}

void Application::Run()
{
	_editor = std::make_unique<Editor>();
	_rendererContext = std::make_shared<glrenderer::RendererContext>();
	_scene = std::make_unique<glrenderer::Scene>(_rendererContext);
	_camera = std::make_unique <glrenderer::Camera>();

	_rendererContext->SetEvents(_scene);


	Input::setWindow(_window->GetNativeWindow());
	_editor->Initialize(_window->GetNativeWindow(), _rendererContext, _scene, _camera);

	//static const std::string modelPath = "C:/dev/gltf-models/Sponza/Sponza.gltf";
	//_scene->ImportModel(modelPath);
	//_scene->CreateBaseEntity(glrenderer::EBaseEntityType::PointLight);

	// Default Cube
	//auto cube = _scene->createEntity("Cube");
	//cube.addComponent<glrenderer::MeshComponent>(glrenderer::Mesh::createMesh(glrenderer::MeshShape::Cube));
	//auto& transformCube = cube.getComponent<glrenderer::TransformComponent>();
	//transformCube.location.y += 2.0f;

	CreateEditorPanels(_editor->GetPanels());

	float deltaTime = 0.0f;	// Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame
	while (!glfwWindowShouldClose(_window->GetNativeWindow()))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		_editor->OnUpdate(_scene);
		
		_rendererContext->RenderScene(_camera, _scene->GetScene(), _editor->GetEntitySelected());

		_editor->Draw();

		/* Swap front and back buffers */
		glfwSwapBuffers(_window->GetNativeWindow());
		
		/* Poll for and process events */
		glfwPollEvents();
	}

	_editor->Free();
	_rendererContext->Free();
}

void Application::OnEvent(Event& e)
{
	_editor->OnEvent(e);
}

void Application::CreateEditorPanels(std::vector<Panel>& panels)
{
	panels.push_back(Panel("Rendering", {
		{ "Renderer", _rendererContext->GetBridge() }, // Node
		{ "Shadow", _rendererContext->GetShadowProperties()->GetBridge() } // Node
	}));

	panels.push_back(Panel("Scene", {
		{ "Lighting", _scene->GetBridge() } // Node
	}));
}

}