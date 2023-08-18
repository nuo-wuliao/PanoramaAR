#pragma once
#include "renderPass.h"

class AdaptiveMaskPass :
	public RenderPass
{
public:
	AdaptiveMaskPass() {};
	~AdaptiveMaskPass() {};

	virtual void init(int width, int height)
	{
		this->bufferWidth = width;
		this->bufferHeight = height;

		if (!this->fbo) glGenFramebuffers(1, &this->fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		bindTextures();

		initShader();
	}

	void bindTextures()
	{
		// color texture
		glGenTextures(1, &adaptBufferTex);
		glBindTexture(GL_TEXTURE_2D, adaptBufferTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bufferWidth, bufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, adaptBufferTex, 0);

		// tell OpenGL which color attachments we'll use (of this frame buffer) for rendering 
		unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, attachments);
		// create and attach depth buffer (render buffer)
		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, bufferWidth, bufferHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
		// finally check if frame buffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "AdaptiveMask Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void initShader()
	{
		adaptShader = new Shader("shader/quad.vs", "shader/adaptive.fs");
	}

public:
	GLuint adaptBufferTex;
	

	Shader *adaptShader;
};