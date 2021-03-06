#pragma once

#include "GLRenderer/Camera.hpp"

#include <memory>
#include <glm/glm.hpp>


namespace oryon {

	class CameraController
	{
	public:
		CameraController(const std::shared_ptr<glrenderer::Camera>& camera = nullptr);

		const std::shared_ptr<glrenderer::Camera>& getCamera() const { return _camera; }
		std::shared_ptr<glrenderer::Camera> getCamera() { return _camera; }

		void onUpdate();
	
	private:
		std::shared_ptr<glrenderer::Camera> _camera;

		glm::vec2 _currentMousePosition = { 0.0f, 0.0f };

	};
}
