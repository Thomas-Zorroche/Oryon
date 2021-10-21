#pragma once

#include "Input.hpp"

namespace oryon {

	class KeyEvent;
	class MouseScrollEvent;

	class Event
	{
	public:
		virtual ~Event() = default;

		virtual KeyEvent* isKeyEvent() = 0;
		virtual MouseScrollEvent* isScrollEvent() = 0;
	};

	class KeyEvent : public Event
	{
	public:
		KeyEvent(const KeyCode keycode) : _keyCode(keycode) {}

		KeyCode getKeyCode() const { return _keyCode; }

		KeyEvent* isKeyEvent() override { return this; }
		MouseScrollEvent* isScrollEvent() override { return nullptr; }

	private:
		KeyCode _keyCode;
	};


	class MouseScrollEvent : public Event
	{
	public:
		MouseScrollEvent(const float xOffset, const float yOffset)
			: _xOffset(xOffset), _yOffset(yOffset) {}

		float getXOffset() const { return _xOffset; }
		float getYOffset() const { return _yOffset; }

		KeyEvent* isKeyEvent() override { return nullptr; }
		MouseScrollEvent* isScrollEvent() override { return this; }

	private:
		float _xOffset, _yOffset;
	};

}