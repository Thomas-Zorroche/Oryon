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
		// Blender Navigation
		const glm::vec2& mousePosition = Input::getMousePosition();
		glm::vec2 delta = (mousePosition - _currentMousePosition);
		_currentMousePosition = mousePosition;

		// Pan --> Shift + MMB
		if (Input::isKeyPressed(KeyCode::LeftShift) && Input::isMouseButtonPressed(MouseCode::ButtonMiddle))
			_camera->pan(delta);

		// Zoom --> Ctrl + MMB
		else if (Input::isKeyPressed(KeyCode::LeftCtrl) && Input::isMouseButtonPressed(MouseCode::ButtonMiddle))
			_camera->zoom(delta.y);

		// Rotate --> MMB
		else if (Input::isMouseButtonPressed(MouseCode::ButtonMiddle))
			_camera->rotate(delta);

		_camera->updateVectors();
	}

}