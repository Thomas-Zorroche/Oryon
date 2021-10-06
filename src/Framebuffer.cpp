#include "Framebuffer.hpp"

#include <glad/glad.h>

#include <iostream>

namespace oryon
{

    Framebuffer::Framebuffer(float width, float height)
    {
        _id = 4;
        glGenFramebuffers(1, &_id);
        int a = 3;
        glBindFramebuffer(GL_FRAMEBUFFER, _id);
        {
            // Create Texture
            glGenTextures(1, &_textureID);
            glBindTexture(GL_TEXTURE_2D, _textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Attach Texture to the framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureID, 0);

            glGenRenderbuffers(1, &_rboID);
            glBindRenderbuffer(GL_RENDERBUFFER, _rboID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rboID);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::resize(float width, float height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _id);
        {
            glBindTexture(GL_TEXTURE_2D, _textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);

            glBindRenderbuffer(GL_RENDERBUFFER, _rboID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::bind(float viewportWidth, float viewportHeight)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _id);
        glViewport(0, 0, viewportWidth, viewportHeight);
    }

    void Framebuffer::unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::free()
    {
        glDeleteFramebuffers(1, &_id);
        glDeleteTextures(1, &_textureID);
        glDeleteRenderbuffers(1, &_rboID);
    }

}

