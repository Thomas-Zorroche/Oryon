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
		if (Input::isKeyPressed(KeyCode::A))
		{
			_camera->zoom();
		}
	}


}