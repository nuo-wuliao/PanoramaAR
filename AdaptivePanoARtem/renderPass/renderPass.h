#pragma once
#include <GLEW/glew.h>
#include <iostream>
#include "../shader.h"

#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

class RenderPass
{
public:
	RenderPass() {};
	~RenderPass() {};

	virtual void init(int width, int height) {};
public:
	GLuint fbo;
	int bufferWidth, bufferHeight;
};

