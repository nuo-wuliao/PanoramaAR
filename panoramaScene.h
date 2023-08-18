#pragma once
#include <GLEW/glew.h>
#include <GL/glut.h>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>

class panoramaScene
{
public:
	panoramaScene();
	~panoramaScene();
	panoramaScene(std::string panoPath, std::string panoNormalPath, std::string panoPosPath, bool isdataset);

	void readInPano(std::string panoPath, std::string panoNormalPath, std::string panoPosPath);
	void genDepthMipMapCPU();
	void updatePano(int num);

public:
	cv::Mat panoData, panoNormal, panoPos;
	unsigned int panoWidth, panoHeight;
	GLuint mipmapTex, panoTexId, panoPosTexId, panoNormalTexId;
	int max_level;
	float min_depth;
	bool isDataSet;
};

