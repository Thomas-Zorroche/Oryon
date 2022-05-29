#pragma once

#include <memory>
#include <chrono>

#include "Window.hpp"
#include "Editor/Editor.hpp"

#include "Events/Event.hpp"

#include "GLRenderer/Renderer/RendererContext.hpp"
#include "GLRenderer/Scene/Scene.hpp"

namespace oryon
{

class Application
{
public:

	Application(int argc, char** argv);

	Window& GetWindow() { return * _window; }

	void Run();

	void OnEvent(Event& e);

	void CreateEditorPanels(std::vector<Panel>& panels);

private:
	std::unique_ptr<Window> _window = nullptr;

	std::unique_ptr<Editor> _editor = nullptr;

	std::shared_ptr<glrenderer::Scene> _scene = nullptr;

	std::shared_ptr<glrenderer::RendererContext> _rendererContext = nullptr;

	std::shared_ptr<glrenderer::Camera> _camera = nullptr;

private:
// Profiling
	std::vector<std::chrono::milliseconds> _elpasedTimes = {};
};

}
