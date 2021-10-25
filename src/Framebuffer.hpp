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

		Type _type;
	};

}