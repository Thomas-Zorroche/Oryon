#pragma once

#include <memory>

#include "Window.hpp"
#include "Editor.hpp"

namespace oryon
{

class Application
{
public:

	Application(int argc, char** argv);

	Window& getWindow() { return * _window; }

	void run();

private:
	std::unique_ptr<Window> _window = nullptr;
	std::unique_ptr<Editor> _editor = nullptr;

};

}
