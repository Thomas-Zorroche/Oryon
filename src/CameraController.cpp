#pragma once

#include "CameraController.hpp"
#include "Events/Input.hpp"


namespace oryon {

	CameraController::CameraController(const std::shared_ptr<glrenderer::Camera>& camera)
		: _camera(camera)
	{
		if (!camera)
		{
			_camera = std::make_shared<glrenderer::Camera>();
		}
	}

	void CameraController::onUpdate()
	{
		if (Input::isKeyPressed(Key::LeftAlt))
		{
			const glm::vec2& mousePosition = Input::getMousePosition();
			glm::vec2 delta = (mousePosition - _currentMousePosition);
			_currentMousePosition = mousePosition;

			// Pan
			if (Input::isMouseButtonPressed(Mouse::ButtonMiddle))
				_camera->pan(delta);
			// Zoom
			else if (Input::isMouseButtonPressed(Mouse::ButtonRight))
				_camera->zoom(delta.y);
			// Rotate
			else if (Input::isMouseButtonPressed(Mouse::ButtonLeft))
				_camera->rotate(delta);
		}

		_camera->updateVectors();
	}

}