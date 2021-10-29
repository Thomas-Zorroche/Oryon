#pragma once

#include <memory>

namespace oryon
{

	class Framebuffer
	{
	public:
		// Possess color, depth and stencil (RenderBuffer) attachments. 
		// Used for main rendering framebuffer
		static std::unique_ptr<Framebuffer> createRenderingBuffer(float width, float height);

		// Possess only depth texture attachment.
		// Used for shadow mapping
		static std::unique_ptr<Framebuffer> createDepthBuffer(float width, float height);

		unsigned int getId() const { return _id; }

		unsigned int getTextureId() const { return _textureID; }

		void resize(float width, float height);

		void bind(float viewportWidth, float viewportHeight);

		void unbind();

		void free();

		float getWidth() const { return _width; }
		float getHeight() const { return _height; }

		enum Type
		{
			Rendering,
			Depth
		};

		Framebuffer::Framebuffer(float width, float height, Type type);

	private:
		unsigned int _id = 0;
		unsigned int _textureID = 0;
		unsigned int _rboID = 0;

		float _width = 1024.0f;
		float _height = 1024.0f;

		Type _type;
	};

}