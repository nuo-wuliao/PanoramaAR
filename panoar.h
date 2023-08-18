#pragma once
#include <iostream>
#include <sstream>
#include <chrono>
#include <time.h>
#include <random>
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "camera.h"
#include "panoramaScene.h"
#include "virtualModel.h"
#include "renderPass/gBufferPass.h"
#include "renderPass/rayTracingPass.h"
#include "renderPass/spatialFilteringPass.h"
#include "renderPass/temperalFilteringPass.h"
#include "renderPass/modulatePass.h"
#include "renderPass/outlierRemovalPass.h"
#include "renderPass/backgroundPass.h"
#include "renderPass/objPass.h"
#include "renderPass/adaptiveMaskPass.h"
#include "renderPass/newTemporalPass.h"

using namespace std;
#define PI 3.141592653f
#define IMGUI_IMPL_OPENGL_LOADER_GLEW

struct MyMaterial {
	glm::vec3 rho, eta, micro_k, refspec;
	float alpha;

	MyMaterial(glm::vec3 _rho, glm::vec3 _eta, glm::vec3 _micro_k, glm::vec3 _refspec, float _alpha)
	{
		rho = _rho;
		eta = _eta;
		micro_k = _micro_k;
		refspec = _refspec;
		alpha = _alpha;
	}
};

class PanoAR
{
public:
	PanoAR() {};
	PanoAR(int width, int height);
	~PanoAR() {};

	//init panorama and virtual model resource
	void CreateGLWindow();
	void setupCamera();
	bool initGLFW();
	bool initGL();
	void initIMGUI();
	void initShader();
	void do_movements();
	void setWindowSize(int width, int height) {
		windowWidth = width;
		windowHeight = height;
	}
	void outputInternalVal();

	//rendering passes
	void run();
	void setFPS();
	void updateCamera();
	void render();
	void renderGbuffer();
	void mergeGbuffer();
	void genMipmapedFrameDepth();
	void adaptiveStencilPass();
	void tracingPass();
	void outlierRemovalPass();
	void spatialFilteringPass();
	void temporalFilteringPass();
	void temporalFilteringRVPass();
	void newTemporalPass();
	void copyTemporlBufferTexture();
	void modulatePass();
	void display();
	void drawGui();
	void outputDebugTextures();
	void outputDebugTextures2(int num);

public:
	//window attributes and parameters
	float deltaTime, lastFrame;
	int fpsCount, frameCount;
	int windowWidth;
	int windowHeight;
	int frameWidth, frameHeight;
	GLFWwindow *windowPtr;

	//camera 
	Camera *pCam;
	glm::mat4 projection, camView;
	glm::vec4 forward;
	glm::mat4 last_projection, last_camView;

	//panorama scene
	panoramaScene *pPano;

	//virtual obj model
	Model *bgModel;
	virtualModel *pvirModel;
	MyMaterial *vir_mat;

	//render passes
	BackGroundPass *pBGPass;
	OBJPass *pOBJPass;
	GBufferPass *pGBPass;
	AdaptiveMaskPass *pAdaptMaskPass;
	RayTracingPass *pRTPass;
	SpatialFilteringPass *pSFPass;
	TemporalFilteringPass *pTFPass, *pTFPassRV;
	NewTemporalPass *pNTPass;
	ModulatePass *pModPass;
	OutlierRemovalPass *pOrPass;

	//shader
	Shader *genMipShader, *displayShader;

	//ui
	bool ui_isAccumulation;
	int frameNum;
	//parameters
	float sigma_c, sigma_n, sigma_d, glo_ratio, color_alpha, moment_alpha, sdRatio, shadowfade;
	int sigma_p, modu_sigma;
	unsigned int lodcount, lod;

	float ani_t, g_2;
	float ani_tmax, ani_tStart;

	std::uniform_real_distribution<float> mRngDist;     ///< We're going to want random #'s in [0...1] (the default distribution)
	std::mt19937 mRng;                                  ///< Our random number generate.  Set up in initialize()

};

