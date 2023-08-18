#pragma once

#include "renderPass.h"

class RayTracingPass:
	public RenderPass
{
public:
	RayTracingPass() {};
	~RayTracingPass() {};

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
		// new color texture
		glGenTextures(1, &newColorTex);
		glBindTexture(GL_TEXTURE_2D, newColorTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bufferWidth, bufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newColorTex, 0);

		// ir-radiance texture
		glGenTextures(1, &irradTex);
		glBindTexture(GL_TEXTURE_2D, irradTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bufferWidth, bufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, irradTex, 0);

		// irradianceRV texture
		glGenTextures(1, &irradRVTex);
		glBindTexture(GL_TEXTURE_2D, irradRVTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bufferWidth, bufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, irradRVTex, 0);


		glGenTextures(1, &traceDataTex);
		glBindTexture(GL_TEXTURE_2D, traceDataTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, bufferWidth, bufferHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, traceDataTex, 0);


		glGenTextures(1, &traceDataRVTex);
		glBindTexture(GL_TEXTURE_2D, traceDataRVTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, bufferWidth, bufferHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, traceDataRVTex, 0);

		glGenTextures(1, &tempRVTex);
		glBindTexture(GL_TEXTURE_2D, tempRVTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bufferWidth, bufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, tempRVTex, 0);

		// tell OpenGL which color attachments we'll use (of this frame buffer) for rendering 
		unsigned int attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
		glDrawBuffers(6, attachments);
		// create and attach depth buffer (render buffer)
		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, bufferWidth, bufferHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "rt Framebuffer not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void initShader()
	{
		rt1Shader = new Shader("shader/quad.vs", "shader/rt1.fs");//-IBL
		rt2Shader = new Shader("shader/quad.vs", "shader/rt2.fs");//-IBL
		//rt1IBLShader = new Shader("shader/quad.vs", "shader/rt1.fs");//
		//rt2IBLShader = new Shader("shader/quad.vs", "shader/rt2_nobackface.fs");//
		rt1IBLShader = new Shader("shader/quad.vs", "shader/rt1-IBL.fs");//
		rt2IBLShader = new Shader("shader/quad.vs", "shader/rt2-IBL.fs");//
		rt1AdaptShader = new Shader("shader/quad.vs", "shader/rt1-adapt.fs");
		rt2AdaptShader = new Shader("shader/quad.vs", "shader/rt2-adapt.fs");
		filterRT1Shader = new Shader("shader/quad.vs", "shader/filterRT1.fs");
	}

public:
	GLuint newColorTex;
	GLuint irradTex;
	GLuint irradRVTex;
	GLuint tempRVTex;
	GLuint traceDataTex, traceDataRVTex;

	Shader *rt1Shader;
	Shader *rt2Shader;

	Shader *rt1AdaptShader;
	Shader *rt2AdaptShader;
	Shader *filterRT1Shader;

	Shader *rt1IBLShader;
	Shader *rt2IBLShader;
};