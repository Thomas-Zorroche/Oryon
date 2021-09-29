#include "Application.hpp"

#include <GLFW/glfw3.h>


int main(int argc, char** argv)
{
	oryon::Application app(argc, argv);

	if (!app.getWindow().Init())
		return 1;

	app.run();

	return 0;
}

