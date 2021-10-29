#include "Framebuffer.hpp"

#include <glad/glad.h>

#include <iostream>

namespace oryon
{
    std::unique_ptr<Framebuffer> Framebuffer::createRenderingBuffer(float width, float height)
    {
        return std::make_unique<Framebuffer>(width, height, Framebuffer::Type::Rendering);
    }

    std::unique_ptr<Framebuffer> Framebuffer::createDepthBuffer(float width, float height)
    {
        return std::make_unique<Framebuffer>(width, height, Framebuffer::Type::Depth);
    }

    Framebuffer::Framebuffer(float width, float height, Type type)
        : _type(type)
    {
        _width = width;
        _height = height;

        glGenFramebuffers(1, &_id);
        glBindFramebuffer(GL_FRAMEBUFFER, _id);
        {
            // Create Texture
            glGenTextures(1, &_textureID);
            glBindTexture(GL_TEXTURE_2D, _textureID);
            if (type == Type::Rendering) 
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }
            else if (type == Type::Depth) 
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            }


            // Attach Texture to the framebuffer
            if (type == Type::Rendering)
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureID, 0);
            else if (type == Type::Depth)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _textureID, 0);
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);
            }

            // RenderBuffer (DEPTH + STENCIL)
            if (type == Type::Rendering)
            {
                glGenRenderbuffers(1, &_rboID);
                glBindRenderbuffer(GL_RENDERBUFFER, _rboID);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rboID);
            }

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::resize(float width, float height)
    {
        _width = width;
        _height = height;

        glBindFramebuffer(GL_FRAMEBUFFER, _id);
        {
            glBindTexture(GL_TEXTURE_2D, _textureID);
            if (_type == Type::Rendering)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            else if (_type == Type::Depth)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);

            if (_type == Type::Rendering)
            {
                glBindRenderbuffer(GL_RENDERBUFFER, _rboID);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::bind(float viewportWidth, float viewportHeight)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _id);
        glViewport(0, 0, _width, _height);
    }

    void Framebuffer::unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::free()
    {
        glDeleteFramebuffers(1, &_id);
        glDeleteTextures(1, &_textureID);
        if (_type == Type::Rendering)
            glDeleteRenderbuffers(1, &_rboID);
    }

}

