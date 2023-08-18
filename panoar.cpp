#include "panoar.h"

static float lastX, lastY;
static float mouse_move_xoffset, mouse_move_yoffset;
static float mouse_scroll_yoffset;
static bool mouse_moved = false, mouse_scroll = false, isOutputDebugData = false, isTiming = false;
static bool firstMouseMove, leftMousePressed, rightMousePressed, middleMousePressed;
static bool keyPressedStatus[1024];
static std::string timeArray;
static int timeFrames;
bool isIBL, isAdaptive, startAnime, isAni, backTopos, isJoinVir,isZoomin;
glm::vec3 fixedCamForward;
float s0, v0, s, v, g;


static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keyPressedStatus[key] = true;
		else if (action == GLFW_RELEASE)
			keyPressedStatus[key] = false;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		isOutputDebugData = true;
	}
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{
		isTiming = true;
	}
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	{
		isIBL = !isIBL;
		isAdaptive = false;
	}
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		g = -10.f;
		s0 = 1.0; v0 = 0.0;
		startAnime = true;
	}
	if (key == GLFW_KEY_B && action == GLFW_PRESS)
	{
		backTopos = true;
		isAni = false;
	}
	if (key == GLFW_KEY_U && action == GLFW_PRESS)
	{
		isJoinVir = !isJoinVir;
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		isZoomin = !isZoomin;
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if(!isIBL)
			isAdaptive = !isAdaptive;
		else isAdaptive = false;
	}
}
static void mouse_move_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouseMove)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	if (leftMousePressed) {
		GLfloat xoffset = lastX - xpos;
		GLfloat yoffset = lastY - ypos;
		mouse_move_xoffset = xoffset;
		mouse_move_yoffset = -yoffset;
		mouse_moved = true;
	}

	lastX = xpos;
	lastY = ypos;

	//pCam->handleMouseMove(xoffset, yoffset);
}
static void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	//pCam->handleMouseScroll(yoffset);
	mouse_scroll_yoffset = yoffset;
	mouse_scroll = true;
}
static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse) return;
	leftMousePressed = (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS);
	rightMousePressed = (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS);
	middleMousePressed = (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS);
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  -1.0f, 0.0f, 0.0f, 0.0f,  0.0f, // bottom-left
			1.0f, -1.0f, -1.0f,  -1.0f, 1.0f, 0.0f,  0.0f,  0.0f, // bottom-right 
			1.0f,  1.0f, -1.0f,  -1.0f, 1.0f, 1.0f,  0.0f,  0.0f,  // top-right        
			1.0f,  1.0f, -1.0f,  -1.0f, 1.0f, 1.0f, 0.0f,  0.0f, // top-right
			-1.0f,  1.0f, -1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,  0.0f, // top-left
			-1.0f, -1.0f, -1.0f,  -1.0f, 0.0f, 0.0f, 0.0f,  0.0f, // bottom-left
			// front face
			-1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f, // bottom-left
			1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f, // top-right
			1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  0.0f,  0.0f, // bottom-right
			1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f, // top-right
			-1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f, // bottom-left
			-1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f, // top-left
			// left face
			-1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 1.0f, -1.0f,  0.0f, // top-left
			-1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f, // bottom-left
			-1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f, // top-right
			-1.0f, -1.0f,  1.0f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f, // bottom-right
			// right face
			1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f, // top-left
			1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f, // top-right 
			1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f, // bottom-right        
			1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f, // bottom-right
			1.0f, -1.0f,  1.0f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f, // bottom-left 
			1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f, // top-left    
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f,  0.0f, -1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 1.0f,  0.0f, -1.0f, // top-left
			1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f,  0.0f, -1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,  0.0f,  1.0f, // top-left
			1.0f,  1.0f , 1.0f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 1.0f,  0.0f,  1.0f, // top-right     
			1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,  0.0f,  1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 0.0f,  0.0f,  1.0f // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

PanoAR::PanoAR(int width, int height)
{
	setWindowSize(width, height);
	frameWidth = width / 2;
	frameHeight = height;
	firstMouseMove = true;
	fpsCount = 0; frameCount = 0;
	CreateGLWindow();
	pCam = new Camera(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), -135.f, 0);

	setupCamera();
	/*pPano = new panoramaScene("E:/wj/code/panoramic ray tracing/Real-world video/videoColor/A/569.exr",
		"E:/wj/code/panoramic ray tracing/Real-world video/videoNormal/A/569.exr",
		"E:/wj/code/panoramic ray tracing/Real-world video/videoPos/A/569.exr", false);*/
	/*pPano = new panoramaScene("E:/wj/code/panoramic ray tracing/resources/panorama/data/livingroom/livingroompano.exr",
		"E:/wj/code/panoramic ray tracing/resources/panorama/data/livingroom/result.exr",
		"E:/wj/code/panoramic ray tracing/resources/panorama/data/livingroom/livingroompanoPos.exr", false);*/
	/*pPano = new panoramaScene("E:/wj/code/panoramic ray tracing/resources/panorama/data/702/90.exr",
		"E:/wj/code/panoramic ray tracing/resources/panorama/data/702/newmergeNormal.exr",
		"E:/wj/code/panoramic ray tracing/resources/panorama/data/702/mergePos.exr", false);*/
	/*pPano = new panoramaScene("E:/wj/code/panoramic ray tracing/resources/panorama/data/kitchen/pano.exr",
		"E:/wj/code/panoramic ray tracing/resources/panorama/data/kitchen/panoNormal.exr",
		"E:/wj/code/panoramic ray tracing/resources/panorama/data/kitchen/panoPos.exr", false);*/
	/*pPano = new panoramaScene("D:/wjCode/resources/panorama/data/apt1/fullfill/hdrpano2.exr",
		"D:/wjCode/resources/panorama/data/apt1/fullfill/newNormal.exr",
		"D:/wjCode/resources/panorama/data/apt1/fullfill/panoPos.exr", false);*/
	/*pPano = new panoramaScene("D:/wjCode/resources/panorama/data/apt5/fullfill/hdrpano3.exr",
		"D:/wjCode/resources/panorama/data/apt5/fullfill/newNormal.exr",
		"D:/wjCode/resources/panorama/data/apt5/fullfill/panoPos.exr", false);*/
	/*pPano = new panoramaScene("E:/wj/code/panoramic ray tracing/resources/panorama/data/bedroom/fullfill/hdrpano.exr",
		"E:/wj/code/panoramic ray tracing/resources/panorama/data/bedroom/fullfill/newNormal.exr",
		"E:/wj/code/panoramic ray tracing/resources/panorama/data/bedroom/fullfill/panoPos.exr", false);*/
	/*pPano = new panoramaScene("D:/wjCode/resources/panorama/data/lab/fullfill/pano.exr",
		"D:/wjCode/resources/panorama/data/lab/fullfill/panoNormal.exr",
		"D:/wjCode/resources/panorama/data/lab/fullfill/panoPos.exr", false);*/

	//20220913---------------------------------------------------//
	//pPano = new panoramaScene("C:/Users/admin/Desktop/IPRT Scenes/office/ldrpano.exr",
	//	"C:/Users/admin/Desktop/IPRT Scenes/office/panoNormal.exr",
	//	//"E:/wj/code/panoramic ray tracing/resources/panorama/data/bedroom/fullfill/newNormal.exr",
	//	"C:/Users/admin/Desktop/IPRT Scenes/office/panoPos.exr", false);
	//pPano = new panoramaScene("C:/Users/admin/Desktop/IPRT Scenes/room2/ldrpano.exr",
	//	"C:/Users/admin/Desktop/IPRT Scenes/room2/panoNormal.exr",
	//	"C:/Users/admin/Desktop/IPRT Scenes/room2/panoPos.exr", false);
	pPano = new panoramaScene("E:/wj/projects/03-IPRT-Revision/IPRT Scenes/room1/ldrpano.exr",
		"E:/wj/projects/03-IPRT-Revision/IPRT Scenes/room1/panoNormal.exr",
		"E:/wj/projects/03-IPRT-Revision/IPRT Scenes/room1/panoPos.exr", false);
		/*pPano = new panoramaScene("C:/Users/admin/Desktop/IPRT Scenes/apartment/ldrpano.exr",
			"C:/Users/admin/Desktop/IPRT Scenes/apartment/panoNormal.exr",
			"C:/Users/admin/Desktop/IPRT Scenes/apartment/panoPos.exr", false);*/
	//20220913---------------------------------------------------//


	bgModel = new Model();
	bgModel->loadModel("E:/wj/code/panoramic ray tracing/resources/objmodels/background/sky.obj");

	pvirModel = new virtualModel("modelPath.txt");
	vir_mat = new MyMaterial(glm::vec3(0.8f), glm::vec3(1.66f, 0.88f, 0.52f), glm::vec3(9.22f, 6.27f, 4.84f),
		glm::vec3(0.8f, 0.8f, 0.8f), 0.01f);

	sigma_c = 0.6f; sigma_n = 0.25; sigma_d = 0.15, sigma_p = 5; glo_ratio = 0.01; sdRatio = 0.3f; shadowfade = 2.0f;
	color_alpha = 0.1; moment_alpha = 20.0; lodcount = 0; lod = 0; modu_sigma = 2;
	
	//to do...
	//initialize render passes
	pBGPass = new BackGroundPass();
	pBGPass->init(frameWidth, frameHeight);

	pOBJPass = new OBJPass();
	pOBJPass->init(frameWidth, frameHeight);

	pGBPass = new GBufferPass();
	pGBPass->init(frameWidth, frameHeight);

	pAdaptMaskPass = new AdaptiveMaskPass();
	pAdaptMaskPass->init(frameWidth, frameHeight);

	pRTPass = new RayTracingPass();
	pRTPass->init(frameWidth, frameHeight);

	pOrPass = new OutlierRemovalPass();
	pOrPass->init(frameWidth, frameHeight);

	pSFPass = new SpatialFilteringPass();
	pSFPass->init(frameWidth, frameHeight);

	pTFPass = new TemporalFilteringPass();
	pTFPass->init(frameWidth, frameHeight);

	pTFPassRV = new TemporalFilteringPass();
	pTFPassRV->init(frameWidth, frameHeight);

	pModPass = new ModulatePass();
	pModPass->init(frameWidth, frameHeight);

	pNTPass = new NewTemporalPass();
	pNTPass->init(frameWidth, frameHeight);

	//initialize shaders used in main process
	initShader();


	//init imGUI
	initIMGUI();

	timeFrames = 0;
	isIBL = false;
	isAdaptive = false;
	startAnime = false;
	isAni = false;
	isZoomin = false;
	ani_tmax = 0.7;
	g_2 = 0.01;
	isJoinVir = true;

	// Set up our random number generator by seeding it with the current time 
	auto currentTime = std::chrono::high_resolution_clock::now();
	auto timeInMillisec = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
	mRng = std::mt19937(uint32_t(timeInMillisec.time_since_epoch().count()));
}

void PanoAR::CreateGLWindow()
{
	if (!initGLFW()) exit(0);
	if (!initGL()) exit(0);
	std::cout << "Create GLWindow success!" << std::endl;
}

void PanoAR::setupCamera()
{
	projection = glm::perspective(glm::radians(pCam->Zoom), float(frameWidth) / float(frameHeight), 0.01f, 100.f);
	pCam->Front = glm::vec3(0.86967, -0.398749, 0.290988); //-0.0392496, -0.207912, 0.97736
	camView = pCam->GetViewMatrix();//bunny
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-0.96, -0.32, -0.05), glm::vec3(0, 1, 0));//A
	//pCam->Front = glm::vec3(-0.399025, -0.27228, -0.875581); //A

	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1.41f, -1.055f, 0.005f), glm::vec3(0, 1, 0));//sphere
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.4f, 0, -0.9f), glm::vec3(0, 1, 0));//teapot
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.f, 0.f, 1.0f), glm::vec3(0, 1, 0));//temp
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));//buddha
	last_camView = camView;
	last_projection = projection; 
}

bool PanoAR::initGLFW() {
	if (!glfwInit())
		return false;
	// These hints switch the OpenGL profile to core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	windowPtr = glfwCreateWindow(windowWidth, windowHeight, "PanoAR", NULL, NULL);
	if (!windowPtr) {
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(windowPtr);
	glfwSwapInterval(0);
	glfwSetKeyCallback(windowPtr, key_callback);
	glfwSetCursorPosCallback(windowPtr, mouse_move_callback);
	glfwSetMouseButtonCallback(windowPtr, mouseButtonCallback);
	glfwSetScrollCallback(windowPtr, mouse_scroll_callback);

	return true;
}

bool PanoAR::initGL() {
	glewExperimental = GL_TRUE; // need this to enforce core profile
	GLenum err = glewInit();
	glGetError(); // parse first error
	if (err != GLEW_OK) {// Problem: glewInit failed, something is seriously wrong.
		std::cout << "glewInit failed: " << glewGetErrorString(err) << std::endl;
		return false;
	}
	glViewport(0, 0, frameWidth, frameHeight); // viewport for x,y to normalized device coordinates transformation

	return true;
}

void PanoAR::initIMGUI()
{
	const char* glsl_version = "#version 130";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(windowPtr, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void PanoAR::initShader()
{
	genMipShader = new Shader("shader/quad.vs", "shader/genmip.fs");
	displayShader = new Shader("shader/quad.vs", "shader/display.fs");
}


void PanoAR::do_movements()
{
	bool sceneChange = false;
	if (mouse_moved) {
		pCam->ProcessMouseMovement(mouse_move_xoffset, mouse_move_yoffset);
		mouse_moved = false;
		sceneChange = true;
	}
	if (mouse_scroll) {
		pCam->ProcessMouseScroll(10.f * mouse_scroll_yoffset);
		mouse_scroll = false;
		sceneChange = true;
	}

	if (keyPressedStatus[GLFW_KEY_A]) {
		pvirModel->models[0].translation.x -= 0.01f;// *pCam->Right;
		sceneChange = true;
	}
	if (keyPressedStatus[GLFW_KEY_D]) {
		pvirModel->models[0].translation.x += 0.01f;// * pCam->Right;
		sceneChange = true;
	}
	if (keyPressedStatus[GLFW_KEY_W]) {
		pvirModel->models[0].translation.z += 0.01f;// * pCam->Front;
		sceneChange = true;
	}
	if (keyPressedStatus[GLFW_KEY_S]) {
		pvirModel->models[0].translation.z -= 0.01f;// * pCam->Front;
		sceneChange = true;
	}
	if (keyPressedStatus[GLFW_KEY_Z]) {
		pvirModel->models[0].translation.y -= 0.01f;// * pCam->Up;
		sceneChange = true;
	}
	if (keyPressedStatus[GLFW_KEY_C]) {
		pvirModel->models[0].translation.y += 0.01f;// * pCam->Up;
		sceneChange = true;
	}

	if (keyPressedStatus[GLFW_KEY_Q]) {
		pvirModel->models[0].rotateX += 0.01;
		if (pvirModel->models[0].rotateX >= 2 * PI) pvirModel->models[0].rotateX = 0.f;
		sceneChange = true;
	}
	if (keyPressedStatus[GLFW_KEY_E]) {
		pvirModel->models[0].rotateY += 0.01;
		if (pvirModel->models[0].rotateY >= 2 * PI) pvirModel->models[0].rotateY = 0.f;
		sceneChange = true;
	}
	if (keyPressedStatus[GLFW_KEY_X]) {
		pvirModel->models[0].rotateZ += 0.01;
		if (pvirModel->models[0].rotateZ >= 2 * PI) pvirModel->models[0].rotateZ = 0.f;
		sceneChange = true;
	}

	if (keyPressedStatus[GLFW_KEY_M]) {
		lodcount += 1;
		lod = (lodcount / 10) % 10;
	}


	//pvirModel->models[1].rotateX = pvirModel->models[0].rotateX;
	//pvirModel->models[1].rotateY = pvirModel->models[0].rotateY;
	//pvirModel->models[1].rotateZ = pvirModel->models[0].rotateZ;
	//pvirModel->models[1].scale = pvirModel->models[0].scale;
	//pvirModel->models[1].translation = pvirModel->models[0].translation;

	if (sceneChange) frameNum = 0;
}

void PanoAR::outputInternalVal()
{
	//printf("sigma_c, n, d, p = %f, %f, %f, %f\t", sigma_c, sigma_n, sigma_d, sigma_p);
	//printf("color_alpha, moment_alpha = %f, %f\n", color_alpha, moment_alpha);
	//printf("lod = %d\n", lod);
}

void PanoAR::run()
{
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	auto t_start = std::chrono::system_clock::now();
	glm::vec3 finaltrans = glm::vec3(1.73, -0.939999, 1.45);
	glm::vec3 starttrans = glm::vec3(-0.0499985, -0.944999, 1.45);
	float startRotateY = pvirModel->models[3].rotateY;
	glm::vec3 vr23_bottletrans1 = glm::vec3(-0.630001, -1.07, -1.14);
	glm::vec3 vr23_bottletrans2 = glm::vec3(-0.520001, -0.65, -0.759998);
	glm::vec3 vr23_bottletrans3 = glm::vec3(-0.420001, -1.09, -0.479998);

	glm::vec3 vr23_ani1hangertrans1 = glm::vec3(-1.8, -1.41, -2.27);
	glm::vec3 vr23_ani1Bottletrans1 = glm::vec3(-0.370001, -0.970001, -0.729998);
	glm::vec3 vr23_ani1presentBoxtrans1 = glm::vec3(-0.200001, -0.95, -0.499998);
	glm::vec3 vr23_ani1hangertrans2 = glm::vec3(-1.88, -1.41, -1.77);
	glm::vec3 vr23_ani1Bottletrans2 = glm::vec3(-0.370001, -0.550001, -0.949998);
	glm::vec3 vr23_ani1presentBoxtrans2 = glm::vec3(0.219999, -0.98, -0.559998);


	int countframe = 0, countPano = 0;
	float delta_zoom = 0.2;
	//fixedCamForward = glm::vec3(-0.399025, -0.27228, -0.875581);
	while (!glfwWindowShouldClose(windowPtr))
	{
		fpsCount++; frameCount++;
		if (fpsCount > 10) fpsCount = 0;
		if (timeFrames >= 1000) isTiming = false;
		if (frameCount == 0xFFFFFFFF) frameCount = 1; 
		if (ui_isAccumulation) {
			++frameNum;
			if (frameNum > 600) frameNum = 600;
		}
		else frameNum = 0;
		
		if (isJoinVir) pvirModel->modelNum = 4;
		else pvirModel->modelNum = 0;

		ani_t = glfwGetTime();

		//**************************rotate bottle*****************************//
		//float rx1 = 1.52, ry1 = 6.0, rz1 = 1.12, rx2 = 1.14, ry2 = 0, rz2 = 5.5;//0.85, 0.91, 1.27; 1.66 1.64 5.63003
		//pvirModel->models[0].rotateX = 1.33 + 0.19 * sin(ani_t);
		//pvirModel->models[0].rotateY = 3.0 + 1.0 * sin(ani_t);
		//pvirModel->models[0].rotateZ = 3.31 + PI + (PI-2.19) * sin(ani_t);
		//if (pvirModel->models[0].rotateZ > 2 * PI) pvirModel->models[0].rotateZ -= 2 * PI;
		//if (pvirModel->models[0].rotateZ < 0) pvirModel->models[0].rotateZ += 2 * PI;
		/*float rx1 = 1.34, ry1 = 1.96, rz1 = 5.62, rx2 = 1.12, ry2 = 1.96, rz2 = 1.06;
		pvirModel->models[4].rotateX = 1.23 - 0.11 * sin(ani_t);
		pvirModel->models[4].rotateY = 1.96 + 3.0 * sin(ani_t);;// 3.0 + 1.0 * sin(ani_t);
		pvirModel->models[4].rotateZ = 0.5 * (5.62 + 1.06) + PI + 0.5 * (2* PI - 5.62+1.06) * sin(ani_t);//6.22 + (PI-6.02+1.82)/2 + (PI - 4.02)/2 * sin(ani_t);
		if (pvirModel->models[4].rotateZ > 2 * PI) pvirModel->models[4].rotateZ -= 2 * PI;
		if (pvirModel->models[4].rotateZ < 0) pvirModel->models[4].rotateZ += 2 * PI; */

		//********************************************************************//

		if (startAnime) {
			ani_tStart = glfwGetTime();
			isAni = true;
			startAnime = false;
			fixedCamForward = glm::normalize(pCam->Front);
		}

		if (isAni) {
			float delta_t = ani_t - ani_tStart;

			/**************************VR-23 room1 bottle movement**********************************/
			//if (delta_t > 4) {
			//	delta_t = 0;
			//	isAni = false;
			//}
			//else {
			//	if (delta_t <= 2) {
			//		float ra = delta_t / 2;
			//		pvirModel->models[0].translation = vr23_bottletrans1 + ra * (vr23_bottletrans2 - vr23_bottletrans1);
			//		pvirModel->models[0].rotateX = ra * 0.5;
			//		pvirModel->models[0].rotateZ = -ra * 0.5;
			//	}
			//	else {
			//		float ra = (delta_t - 2) / 2;
			//		pvirModel->models[0].translation = vr23_bottletrans2 + ra * (vr23_bottletrans3 - vr23_bottletrans2);
			//		pvirModel->models[0].rotateX = 0.5 - ra * 0.5;
			//		pvirModel->models[0].rotateZ = -0.5 + ra * 0.5;
			//	}
			//}

			/**************************VR-23 room1 bottle movement**********************************/

			/**************************VR-23 room1 camera rotation**********************************/
			//glm::vec3 target_forward = glm::normalize(glm::vec3(-0.0564153, -0.658689, -0.750297));
			//if (delta_t >= 3) {
			//	delta_t = 3;
			//	isAni = false;
			//}
			////if (delta_t >= 1.4 && delta_t < 1.5) modu_sigma = 1;
			//float theta = delta_t / 3.f;
			//glm::vec3 temp = glm::normalize(fixedCamForward) + delta_t / 3.f * (glm::normalize(target_forward) - glm::normalize(fixedCamForward));
			//temp = glm::normalize(temp);
			//pCam->Front = temp;
			/**************************VR-23 room1 camera rotation**********************************/


			/**************************VR-23 apartment animation**********************************/
			if (delta_t >= 22) {
				delta_t = 0;
				isAni = false;
			}
			//1. camera rotation
			if (delta_t <= 6) {
				float theta = 2 * PI * delta_t / 6.f;
				glm::vec3 up = glm::vec3(0, 1, 0);
				glm::vec3 forward;
				forward = cos(theta) * fixedCamForward + (1 - cos(theta)) *(glm::dot(fixedCamForward, up)) * up +
					glm::cross(sin(theta) * up, fixedCamForward);
				pCam->Front = forward;
			}

			if (delta_t > 6.5) {
				isJoinVir = true;
				pCam->Front = fixedCamForward;
			}

			if (delta_t > 8 && delta_t <= 12)
			{
				float delta_tt = delta_t - 8.f;
				if (delta_tt <= 2) {
					float ra = delta_tt / 2;
					pvirModel->models[0].translation = vr23_ani1hangertrans1 + ra * (vr23_ani1hangertrans2 - vr23_ani1hangertrans1);
				}
				else {
					float ra = (delta_tt - 2) / 2;
					pvirModel->models[0].translation = vr23_ani1hangertrans2 + ra * (vr23_ani1hangertrans1 - vr23_ani1hangertrans2);
				}
			}


			if (delta_t > 13 && delta_t <= 17)
			{
				float delta_tt = delta_t - 13.f;
				if (delta_tt <= 2) {
					float ra = delta_tt / 2;
					pvirModel->models[1].translation = vr23_ani1Bottletrans1 + ra * (vr23_ani1Bottletrans2 - vr23_ani1Bottletrans1);
				}
				else {
					float ra = (delta_tt - 2) / 2;
					pvirModel->models[1].translation = vr23_ani1Bottletrans2 + ra * (vr23_ani1Bottletrans1 - vr23_ani1Bottletrans2);
				}
			}


			if (delta_t > 18 && delta_t <= 22)
			{
				float delta_tt = delta_t - 18.f;
				if (delta_tt <= 2) {
					float ra = delta_tt / 2;
					pvirModel->models[2].translation = vr23_ani1presentBoxtrans1 + ra * (vr23_ani1presentBoxtrans2 - vr23_ani1presentBoxtrans1);
				}
				else {
					float ra = (delta_tt - 2) / 2;
					pvirModel->models[2].translation = vr23_ani1presentBoxtrans2 + ra * (vr23_ani1presentBoxtrans1 - vr23_ani1presentBoxtrans2);
				}
			}

			/**************************VR-23 apartment animation**********************************/

			/**************************teapot rotation********************************/
			/*if (delta_t >= 6) {
				delta_t = 6;
				isAni = false;
			}
			float theta = delta_t / 6.f;
			pvirModel->models[3].rotateY = startRotateY - 4 * PI*theta;*/

			/********************************************************************/

			/****************camera***move2****************************************/
			/*glm::vec3 target_forward = glm::normalize(glm::vec3(0.662578, -0.277315, -0.695764));
			if (delta_t >= 3) {
				delta_t = 3;
				isAni = false;
			}
			if (delta_t >= 1.4 && delta_t < 1.5) modu_sigma = 1;
			float theta = delta_t / 3.f;
			glm::vec3 temp = glm::normalize(fixedCamForward) + delta_t / 3.f * (glm::normalize(target_forward) - glm::normalize(fixedCamForward));
			temp = glm::normalize(temp);
			pCam->Front = temp;*/
			/********************************************************************/


			/****************camera***move1****************************************/
			/*if (delta_t >= 2) {
				delta_t = 2;
				isAni = false;
			}
			float t = delta_t / 2.f;
			glm::vec3 temp;
			temp = glm::normalize(glm::vec3(-0.0392496, -0.207912, 0.97736)) + t * (glm::normalize(glm::vec3(-0.0499985, -0.944999, 1.45)) - 
				glm::normalize(glm::vec3(-0.0392496, -0.207912, 0.97736)));
			pCam->Front = temp;*/

			/****************************************************************************/


			/****************basketball***move****************************************/
			/*if (delta_t >= 8) {
				delta_t = 8;
				isAni = false;
			}
			float t = delta_t / 8.f;
			glm::vec3 temp;
			temp = starttrans + t * (finaltrans - starttrans);
			pvirModel->models[0].translation = temp;
			pvirModel->models[0].rotateZ = -6.5 * PI * t;
			pCam->Front = temp;*/

			/****************************************************************************/


			/****************basketball***bump****************************************/
			/*{
				float delta_t = delta_tt;
				s = s0 + v0 * delta_t + 0.5 * g * delta_t * delta_t;
				v = v0 + g * delta_t;
				if (s <= 0.001 && v < 0) {
					v = -v * 0.8;
					s0 = s;
					v0 = v;
					ani_tStart = ani_t;
					if (abs(v) < 0.01) {
						isAni = false;
						pvirModel->models[2].translation = glm::vec3(1.79, -1.68, 0.939998);
					}
				}
				if(s < 0) pvirModel->models[2].translation = glm::vec3(1.79, -1.68, 0.939998);
				else pvirModel->models[2].translation.y = -1.68 + s;
			}*/
			/***************************************************************/


			/****************apt5***rotate cam*****************************/
			/*if (delta_t >= 6) {
				delta_t = 0;
				isAni = false;
			}
			float theta = 2 * PI * delta_t / 6.f;
			glm::vec3 up = glm::vec3(0, 1, 0);
			glm::vec3 forward;
			forward = cos(theta) * fixedCamForward + (1 - cos(theta)) *(glm::dot(fixedCamForward, up)) * up +
				glm::cross(sin(theta) * up, fixedCamForward);
			pCam->Front = forward;*/
			/*********************************************************************/





			/****************hanger bag***rotate cam2*****************************/

			/*glm::vec3 target_forward = glm::normalize(glm::vec3(-0.05331, -0.563526, -0.824376));
			if (delta_t >= 3) {
				delta_t = 3;
				isAni = false;
			}
			if (delta_t >= 1.4 && delta_t < 1.5) {
				vir_mat->alpha = 0.03;
				g_2 = 1.0f;
			}
			float theta = delta_t / 3.f;
			glm::vec3 temp = fixedCamForward + delta_t / 3.f * (target_forward - fixedCamForward);
			temp = glm::normalize(temp);
			pCam->Front = temp;*/
			//*****************************************************************//

			//**************hanger bag*****rotate cam*****************************//
			/*if (delta_t >= 8) {
				delta_t = 0;
				isAni = false;
			}
			float theta = 2 * PI * delta_t / 8.f;
			glm::vec3 up = glm::vec3(0, 1, 0);
			glm::vec3 forward;
			forward = cos(theta) * fixedCamForward + (1 - cos(theta)) *(glm::dot(fixedCamForward, up)) * up +
				glm::cross(sin(theta) * up, fixedCamForward);
			pCam->Front = forward;*/
			//*****************************************************************//

			//*************************hanger bag*******************************//
			/*float dis = g_2 * delta_t * delta_t;
			pvirModel->models[0].translation.y -= dis;
			if (dis > 2.0) isAni = false;*/
			//*****************************************************************//

			//*************************hanger bag*******************************//
			/*float dis = 60 * delta_t / 2.f;
			if (delta_t >= 2) {
				delta_t = 2;
				isAni = false;
			}
			pCam->Zoom = 30 + dis;*/
			//*****************************************************************//
		}

		if (isZoomin) {
			//float delta_t = ani_t - ani_tStart;
			if(pCam->Zoom == 45) delta_zoom = 0.22;
			else if (pCam->Zoom == 90) delta_zoom = -0.22;
			pCam->Zoom += delta_zoom;
			if (pCam->Zoom <= 45) {
				pCam->Zoom = 45;
				isZoomin = false;
			}
			if (pCam->Zoom >= 90) {
				pCam->Zoom = 90;
				isZoomin = false;
			}
		}
		/*else {
			pCam->Zoom += 0.2;
			if (pCam->Zoom >= 90) pCam->Zoom = 90;
		}*/

		if (backTopos) {
			pvirModel->setupTransform();
			pCam->Front = glm::vec3(1.73, -0.939999, 1.45);
			backTopos = false;
		}

		setFPS();

		glfwPollEvents();
		do_movements();

		outputInternalVal();

		updateCamera();
		//tracing and display image

		render();

		auto t_end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start);
		t_start = t_end;
		double time = 1000.0 * double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
		//std::cout << "total time : " <<
		//	time << " miliseconds." << std::endl;
		if (timeFrames >= 1000) {
			std::ofstream out("time.txt");
			out << timeArray;
			out.close();
			timeFrames = 0;
			timeArray.clear();
		}
		if (isTiming) {
			timeArray += std::to_string(time) + " ";
			timeFrames += 1;
		}
		if (isOutputDebugData) {
			outputDebugTextures();
			isOutputDebugData = false;
		}

		drawGui();

		/*
		countframe++;
		if (countframe >= 100) {
			countframe = 0;
			outputDebugTextures2(countPano);
			countPano++;

			if (countPano >= 90) {
				glfwTerminate();
				return ;
				//glfwSetWindowShouldClose(windowPtr, GL_TRUE);
			}
			//pPano->updatePano(countPano); //updata panorama

			glm::vec3 target_forward = glm::normalize(glm::vec3(0.923141, -0.384296, 0.0112789));
			float ratiotemp = 1.f * countPano / 240.f;
			glm::vec3 temp = fixedCamForward + ratiotemp * (target_forward - fixedCamForward);
			temp = glm::normalize(temp);
			pCam->Front = temp;
			pCam->Zoom = 40 + ratiotemp * (80 - 40);
			if (countPano == 120) {
				pPano->updatePano(90);
				modu_sigma = 2;
				vir_mat->refspec = glm::vec3(0.9, 0.8, 0.7);
			}


			cout << "finised " << countPano << " img..." << endl;
			
		}
		*/

		glfwSwapBuffers(windowPtr);
	}
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(windowPtr);
	glfwTerminate();
}

void PanoAR::setFPS()
{
	float currentFrame = glfwGetTime();
	//pCam->Zoom = 60 + (90 - 60)*sin(0.2f * currentFrame);
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	float ifps = 1.0 / deltaTime;
	//std::cout << "fps: " << ifps <<std::endl;
	if (fpsCount == 0) {
		std::stringstream ss;
		ss << "accframeNum = " << frameNum << ", PanoAR(" << frameWidth << " x " << frameHeight << "): " << ifps << " fps";
		glfwSetWindowTitle(windowPtr, ss.str().c_str());
	}
}

void PanoAR::updateCamera()
{
	last_projection = projection;
	last_camView = camView;
	projection = glm::perspective(glm::radians(pCam->Zoom), float(frameWidth) / float(frameHeight), 0.01f, 100.f);
	camView = pCam->GetViewMatrix();//bunny
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-0.0564153, -0.658689, -0.750297), glm::vec3(0, 1, 0));//A
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.923141, -0.384296, 0.0112789), glm::vec3(0, 1, 0)); //B
	//pCam->Zoom = 80.0f;

	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1.41f, -1.055f, 0.005f), glm::vec3(0, 1, 0));//sphere
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.4f, 0, -0.9f), glm::vec3(0, 1, 0));//teapot
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));//buddha
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-0.96,0,-0.27), glm::vec3(0, 1, 0));//dragon
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.009, 0.026, 1.0), glm::vec3(0, 1, 0));//lucy-apt5
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.66, 0, 0.75), glm::vec3(0, 1, 0));//basketball-apt5
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.119467, -0.30237, -0.945674), glm::vec3(0, 1, 0));//teaser-bedroom
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-0.96, -0.32, -0.05), glm::vec3(0, 1, 0));//lucy-kitchen
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.151844, -0.240228, 0.958767), glm::vec3(0, 1, 0));//lucy-kitchen
	//camView = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-0.0392496, -0.207912, 0.97736), glm::vec3(0, 1, 0));//basket-start
	forward = -glm::inverse(camView)[2];
}

void PanoAR::render()
{
	renderGbuffer();
	mergeGbuffer();
	//display();
	//return;

	genMipmapedFrameDepth();

	if(isAdaptive) adaptiveStencilPass();


	tracingPass();//to do


	outlierRemovalPass();

	spatialFilteringPass();

	//temporalFilteringPass();
	//temporalFilteringRVPass();
	//copyTemporlBufferTexture();
	//return;

	modulatePass();
	glCopyImageSubData(pModPass->accIrradTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pModPass->accHisIrradTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);
	glCopyImageSubData(pModPass->accIrradRVTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pModPass->accHisIrradRVTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);

	newTemporalPass();
	glCopyImageSubData(pNTPass->imageAccTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pNTPass->imageHisTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);

	display();
	return;
}

void PanoAR::renderGbuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pBGPass->fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);

	//render panorama
	pBGPass->panoShader->use();
	pBGPass->panoShader->setMat4("projection", projection);
	pBGPass->panoShader->setMat4("camView", camView);
	pBGPass->panoShader->setMat4("lastProjection", last_projection);
	pBGPass->panoShader->setMat4("lastCamView", last_camView);
	pBGPass->panoShader->setInt("frameWidth", frameWidth);
	pBGPass->panoShader->setInt("frameHeight", frameHeight);
	pBGPass->panoShader->setInt("panoTex", 1);
	pBGPass->panoShader->setInt("panoPosTex", 2);
	pBGPass->panoShader->setInt("panoNormalTex", 3);
	//pGBPass->panoShader->setInt("panoAlbedoTex", 3);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pPano->panoTexId);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pPano->panoPosTexId);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, pPano->panoNormalTexId);
	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, panoAlbedoTex);
	//renderCube();
	bgModel->draw(*pBGPass->panoShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	////render obj model
	glBindFramebuffer(GL_FRAMEBUFFER, pOBJPass->fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);
	for (int i = 0; i < pvirModel->modelNum; i++) {
		pOBJPass->objShader->use();
		pOBJPass->objShader->setMat4("projection", projection);
		pOBJPass->objShader->setMat4("camView", camView);
		pOBJPass->objShader->setMat4("lastProjection", last_projection);
		pOBJPass->objShader->setMat4("lastCamView", last_camView);
		pvirModel->updateObjModelMat(pCam->Right, pCam->Up, pCam->Front);
		pOBJPass->objShader->setMat4("model", pvirModel->models[i].model);
		pOBJPass->objShader->setMat4("lastModel", pvirModel->models[i].last_model);
		pOBJPass->objShader->setInt("frameWidth", frameWidth);
		pOBJPass->objShader->setInt("frameHeight", frameHeight);
		pOBJPass->objShader->setInt("mat", pvirModel->models[i].materialId);
		pOBJPass->objShader->setVec3("rho", vir_mat->rho);
		//pOBJPass->objShader->setInt("", 2);
		/*pOBJPass->objShader->setInt("colorTex", 1);
		pOBJPass->objShader->setInt("worldPosTex", 2);
		pOBJPass->objShader->setInt("normalTex", 3);
		pOBJPass->objShader->setInt("mat_prevCoordTex", 4);
		pOBJPass->objShader->setInt("mipmapedFrameDepthTex", 5);
		pOBJPass->objShader->setInt("modelNormalTex", 6);
		pOBJPass->objShader->setInt("diff_textureTex", 7);*/
		/*glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, pPano->panoPosTexId);*/
		/*glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pGBPass->colorTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, pGBPass->worldPosTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, pGBPass->mipmapedFrameDepthTex);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, pGBPass->modelNormalTex);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, pGBPass->diff_textureTex);*/
		pvirModel->models[i].objmodel->draw(*pOBJPass->objShader);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PanoAR::mergeGbuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pGBPass->fbo);
	glClearColor(0, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	pGBPass->gbShader->use();
	pGBPass->gbShader->setInt("colorTex", 0);
	pGBPass->gbShader->setInt("worldPosTex", 1);
	pGBPass->gbShader->setInt("normalTex", 2);
	pGBPass->gbShader->setInt("mat_prevCoordTex", 3);
	pGBPass->gbShader->setInt("mipmapedFrameDepthTex", 4);
	pGBPass->gbShader->setInt("modelNormalTex", 5);
	pGBPass->gbShader->setInt("diff_textureTex", 6);
	pGBPass->gbShader->setInt("colorTex2", 7);
	pGBPass->gbShader->setInt("worldPosTex2", 8);
	pGBPass->gbShader->setInt("normalTex2", 9);
	pGBPass->gbShader->setInt("mat_prevCoordTex2", 10);
	pGBPass->gbShader->setInt("mipmapedFrameDepthTex2", 11);
	pGBPass->gbShader->setInt("modelNormalTex2", 12);
	pGBPass->gbShader->setInt("diff_textureTex2", 13);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pBGPass->colorTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pBGPass->worldPosTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pBGPass->normalTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, pBGPass->mat_prevCoordTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, pBGPass->mipmapedFrameDepthTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, pBGPass->modelNormalTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, pBGPass->diff_textureTex);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, pOBJPass->colorTex);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, pOBJPass->worldPosTex);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, pOBJPass->normalTex);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, pOBJPass->mat_prevCoordTex);
	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, pOBJPass->mipmapedFrameDepthTex);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, pOBJPass->modelNormalTex);
	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, pOBJPass->diff_textureTex);

	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PanoAR::genMipmapedFrameDepth()
{
	GLuint genMipFbo;
	glGenFramebuffers(1, &genMipFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, genMipFbo);
	int w, h;
	for (int i = 0; i < pGBPass->mip_maxLevel; i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pGBPass->mipmapedFrameDepthTex, i + 1);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "gen mip frame buffer level not complete: " << i + 1 << std::endl;


		glClear(GL_COLOR_BUFFER_BIT);
		genMipShader->use();
		genMipShader->setFloat("inputLevel", i);
		genMipShader->setInt("inputTex", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pGBPass->mipmapedFrameDepthTex);

		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_HEIGHT, &h);
		//std::cout << w << ", "<<h << std::endl;
		genMipShader->setVec2("texSize", w, h);
		renderQuad();

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &genMipFbo);
}

void PanoAR::adaptiveStencilPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pAdaptMaskPass->fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	pAdaptMaskPass->adaptShader->use();
	pAdaptMaskPass->adaptShader->setVec2("iResolution", frameWidth, frameHeight);
	pAdaptMaskPass->adaptShader->setInt("posTex", 0);
	pAdaptMaskPass->adaptShader->setInt("normalTex", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pGBPass->worldPosTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);

	renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PanoAR::tracingPass()
{
	float xOff = 0.0f, yOff = 0.0f;
	xOff = (mRngDist(mRng) - 0.5f) * 1;
	yOff = (mRngDist(mRng) - 0.5f) * 1;

	if (isIBL) {
		glBindFramebuffer(GL_FRAMEBUFFER, pRTPass->fbo);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		//1. tracing the panorama part in the frame
		pRTPass->rt1IBLShader->use();
		pRTPass->rt1IBLShader->setMat4("projection", projection);
		pRTPass->rt1IBLShader->setMat4("camView", camView);
		//pRTPass->rt1IBLShader->setVec3("material.rho", vir_mat->rho);
		pRTPass->rt1IBLShader->setVec3("material.eta", vir_mat->eta);
		pRTPass->rt1IBLShader->setVec3("material.micro_k", vir_mat->micro_k);
		pRTPass->rt1IBLShader->setVec3("material.refspec", vir_mat->refspec);
		pRTPass->rt1IBLShader->setFloat("material.alpha", vir_mat->alpha);
		pRTPass->rt1IBLShader->setVec2("iResolution", frameWidth, frameHeight);
		pRTPass->rt1IBLShader->setVec2("panoSize", pPano->panoWidth, pPano->panoHeight);
		pRTPass->rt1IBLShader->setInt("frameCount", frameCount);
		pRTPass->rt1IBLShader->setFloat("min_depth", pPano->min_depth);
		pRTPass->rt1IBLShader->setFloat("shadowfade", shadowfade);
		pRTPass->rt1IBLShader->setFloat("xOff", xOff);
		pRTPass->rt1IBLShader->setFloat("yOff", yOff);
		pRTPass->rt1IBLShader->setInt("mipmapDepthTex", 0);
		pRTPass->rt1IBLShader->setInt("colorTex", 1);
		pRTPass->rt1IBLShader->setInt("normalTex", 2);
		pRTPass->rt1IBLShader->setInt("worldPosTex", 3);
		pRTPass->rt1IBLShader->setInt("mat_prevCoordTex", 4);
		pRTPass->rt1IBLShader->setInt("panoTex", 5);
		pRTPass->rt1IBLShader->setInt("mipmapedFrameDepthTex", 6);
		pRTPass->rt1IBLShader->setInt("diffTextureTex", 7);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pPano->mipmapTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pGBPass->colorTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, pGBPass->worldPosTex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, pPano->panoTexId);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, pGBPass->mipmapedFrameDepthTex);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, pGBPass->diff_textureTex);
		renderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//2. tracing virtual model part in the frame
		glBindFramebuffer(GL_FRAMEBUFFER, pRTPass->fbo);
		glClear(GL_DEPTH_BUFFER_BIT);
		pRTPass->rt2IBLShader->use();
		pRTPass->rt2IBLShader->setMat4("projection", projection);
		pRTPass->rt2IBLShader->setMat4("camView", camView);
		//pRTPass->rt2IBLShader->setVec3("material.rho", vir_mat->rho);
		pRTPass->rt2IBLShader->setVec3("material.eta", vir_mat->eta);
		pRTPass->rt2IBLShader->setVec3("material.micro_k", vir_mat->micro_k);
		pRTPass->rt2IBLShader->setVec3("material.refspec", vir_mat->refspec);
		pRTPass->rt2IBLShader->setFloat("material.alpha", vir_mat->alpha);
		pRTPass->rt2IBLShader->setVec2("iResolution", frameWidth, frameHeight);
		pRTPass->rt2IBLShader->setVec2("panoSize", pPano->panoWidth, pPano->panoHeight);
		pRTPass->rt2IBLShader->setInt("frameCount", frameCount);
		pRTPass->rt2IBLShader->setFloat("min_depth", pPano->min_depth);
		pRTPass->rt2IBLShader->setFloat("xOff", xOff);
		pRTPass->rt2IBLShader->setFloat("yOff", yOff);
		pRTPass->rt2IBLShader->setInt("mipmapDepthTex", 0);
		pRTPass->rt2IBLShader->setInt("colorTex", 1);
		pRTPass->rt2IBLShader->setInt("normalTex", 2);
		pRTPass->rt2IBLShader->setInt("worldPosTex", 3);
		pRTPass->rt2IBLShader->setInt("mat_prevCoordTex", 4);
		pRTPass->rt2IBLShader->setInt("panoTex", 5);
		pRTPass->rt2IBLShader->setInt("newColorTex", 6);
		pRTPass->rt2IBLShader->setInt("irradTex", 7);
		pRTPass->rt2IBLShader->setInt("irradRVTex", 8);
		pRTPass->rt2IBLShader->setInt("mipmapedFrameDepthTex", 9);
		pRTPass->rt2IBLShader->setInt("traceDataRVTex", 10);
		pRTPass->rt2IBLShader->setInt("diffTextureTex", 11);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pPano->mipmapTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pGBPass->colorTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, pGBPass->worldPosTex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, pPano->panoTexId);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, pRTPass->newColorTex);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, pRTPass->irradTex);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, pRTPass->irradRVTex);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, pGBPass->mipmapedFrameDepthTex);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, pRTPass->traceDataRVTex);
		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_2D, pGBPass->diff_textureTex);
		renderQuad();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else {
		if (isAdaptive) {
			glBindFramebuffer(GL_FRAMEBUFFER, pRTPass->fbo);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			//1. tracing the panorama part in the frame
			pRTPass->rt1AdaptShader->use();
			pRTPass->rt1AdaptShader->setMat4("projection", projection);
			pRTPass->rt1AdaptShader->setMat4("camView", camView);
			//pRTPass->rt1AdaptShader->setVec3("material.rho", vir_mat->rho);
			pRTPass->rt1AdaptShader->setVec3("material.eta", vir_mat->eta);
			pRTPass->rt1AdaptShader->setVec3("material.micro_k", vir_mat->micro_k);
			pRTPass->rt1AdaptShader->setVec3("material.refspec", vir_mat->refspec);
			pRTPass->rt1AdaptShader->setFloat("material.alpha", vir_mat->alpha);
			pRTPass->rt1AdaptShader->setVec2("iResolution", frameWidth, frameHeight);
			pRTPass->rt1AdaptShader->setVec2("panoSize", pPano->panoWidth, pPano->panoHeight);
			pRTPass->rt1AdaptShader->setInt("frameCount", frameCount);
			pRTPass->rt1AdaptShader->setFloat("min_depth", pPano->min_depth);
			pRTPass->rt1AdaptShader->setFloat("shadowfade", shadowfade);
			pRTPass->rt1AdaptShader->setFloat("xOff", xOff);
			pRTPass->rt1AdaptShader->setFloat("yOff", yOff);
			pRTPass->rt1AdaptShader->setInt("mipmapDepthTex", 0);
			pRTPass->rt1AdaptShader->setInt("colorTex", 1);
			pRTPass->rt1AdaptShader->setInt("normalTex", 2);
			pRTPass->rt1AdaptShader->setInt("worldPosTex", 3);
			pRTPass->rt1AdaptShader->setInt("mat_prevCoordTex", 4);
			pRTPass->rt1AdaptShader->setInt("panoTex", 5);
			pRTPass->rt1AdaptShader->setInt("mipmapedFrameDepthTex", 6);
			pRTPass->rt1AdaptShader->setInt("diffTextureTex", 7);
			pRTPass->rt1AdaptShader->setInt("adaptStencilTex", 8);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, pPano->mipmapTex);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pGBPass->colorTex);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, pGBPass->worldPosTex);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, pPano->panoTexId);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, pGBPass->mipmapedFrameDepthTex);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, pGBPass->diff_textureTex);
			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, pAdaptMaskPass->adaptBufferTex);
			renderQuad();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, pRTPass->fbo);
			glClear(GL_DEPTH_BUFFER_BIT);
			pRTPass->filterRT1Shader->use();
			pRTPass->filterRT1Shader->setVec2("iResolution", frameWidth, frameHeight);
			pRTPass->filterRT1Shader->setInt("adaptStencilTex", 0);
			pRTPass->filterRT1Shader->setInt("irradTex", 1);
			pRTPass->filterRT1Shader->setInt("irradRVTex", 2);
			pRTPass->filterRT1Shader->setInt("tempRVTex", 3);
			pRTPass->filterRT1Shader->setInt("colorTex", 4);
			pRTPass->filterRT1Shader->setInt("mat_prevCoordTex", 5);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, pAdaptMaskPass->adaptBufferTex);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pRTPass->irradTex);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, pRTPass->irradRVTex);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, pRTPass->tempRVTex);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, pGBPass->colorTex);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
			renderQuad();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);


			//2. tracing virtual model part in the frame
			glBindFramebuffer(GL_FRAMEBUFFER, pRTPass->fbo);
			glClear(GL_DEPTH_BUFFER_BIT);
			pRTPass->rt2AdaptShader->use();
			pRTPass->rt2AdaptShader->setMat4("projection", projection);
			pRTPass->rt2AdaptShader->setMat4("camView", camView);
			//pRTPass->rt2AdaptShader->setVec3("material.rho", vir_mat->rho);
			pRTPass->rt2AdaptShader->setVec3("material.eta", vir_mat->eta);
			pRTPass->rt2AdaptShader->setVec3("material.micro_k", vir_mat->micro_k);
			pRTPass->rt2AdaptShader->setVec3("material.refspec", vir_mat->refspec);
			pRTPass->rt2AdaptShader->setFloat("material.alpha", vir_mat->alpha);
			pRTPass->rt2AdaptShader->setVec2("iResolution", frameWidth, frameHeight);
			pRTPass->rt2AdaptShader->setVec2("panoSize", pPano->panoWidth, pPano->panoHeight);
			pRTPass->rt2AdaptShader->setInt("frameCount", frameCount);
			pRTPass->rt2AdaptShader->setFloat("min_depth", pPano->min_depth);
			pRTPass->rt2AdaptShader->setFloat("xOff", xOff);
			pRTPass->rt2AdaptShader->setFloat("yOff", yOff);
			pRTPass->rt2AdaptShader->setInt("mipmapDepthTex", 0);
			pRTPass->rt2AdaptShader->setInt("colorTex", 1);
			pRTPass->rt2AdaptShader->setInt("normalTex", 2);
			pRTPass->rt2AdaptShader->setInt("worldPosTex", 3);
			pRTPass->rt2AdaptShader->setInt("mat_prevCoordTex", 4);
			pRTPass->rt2AdaptShader->setInt("panoTex", 5);
			pRTPass->rt2AdaptShader->setInt("newColorTex", 6);
			pRTPass->rt2AdaptShader->setInt("irradTex", 7);
			pRTPass->rt2AdaptShader->setInt("irradRVTex", 8);
			pRTPass->rt2AdaptShader->setInt("mipmapedFrameDepthTex", 9);
			pRTPass->rt2AdaptShader->setInt("traceDataRVTex", 10);
			pRTPass->rt2AdaptShader->setInt("diffTextureTex", 11);
			pRTPass->rt2AdaptShader->setInt("adaptStencilTex", 12);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, pPano->mipmapTex);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pGBPass->colorTex);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, pGBPass->worldPosTex);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, pPano->panoTexId);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, pRTPass->newColorTex);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, pRTPass->irradTex);
			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, pRTPass->irradRVTex);
			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, pGBPass->mipmapedFrameDepthTex);
			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D, pRTPass->traceDataRVTex);
			glActiveTexture(GL_TEXTURE11);
			glBindTexture(GL_TEXTURE_2D, pGBPass->diff_textureTex);
			glActiveTexture(GL_TEXTURE12);
			glBindTexture(GL_TEXTURE_2D, pAdaptMaskPass->adaptBufferTex);
			renderQuad();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, pRTPass->fbo);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			//1. tracing the panorama part in the frame
			pRTPass->rt1Shader->use();
			pRTPass->rt1Shader->setMat4("projection", projection);
			pRTPass->rt1Shader->setMat4("camView", camView);
			//pRTPass->rt1Shader->setVec3("material.rho", vir_mat->rho);
			pRTPass->rt1Shader->setVec3("material.eta", vir_mat->eta);
			pRTPass->rt1Shader->setVec3("material.micro_k", vir_mat->micro_k);
			pRTPass->rt1Shader->setVec3("material.refspec", vir_mat->refspec);
			pRTPass->rt1Shader->setFloat("material.alpha", vir_mat->alpha);
			pRTPass->rt1Shader->setVec2("iResolution", frameWidth, frameHeight);
			pRTPass->rt1Shader->setVec2("panoSize", pPano->panoWidth, pPano->panoHeight);
			pRTPass->rt1Shader->setInt("frameCount", frameCount);
			pRTPass->rt1Shader->setFloat("min_depth", pPano->min_depth);
			pRTPass->rt1Shader->setFloat("shadowfade", shadowfade);
			pRTPass->rt1Shader->setFloat("xOff", xOff);
			pRTPass->rt1Shader->setFloat("yOff", yOff);
			pRTPass->rt1Shader->setInt("mipmapDepthTex", 0);
			pRTPass->rt1Shader->setInt("colorTex", 1);
			pRTPass->rt1Shader->setInt("normalTex", 2);
			pRTPass->rt1Shader->setInt("worldPosTex", 3);
			pRTPass->rt1Shader->setInt("mat_prevCoordTex", 4);
			pRTPass->rt1Shader->setInt("panoTex", 5);
			pRTPass->rt1Shader->setInt("mipmapedFrameDepthTex", 6);
			pRTPass->rt1Shader->setInt("diffTextureTex", 7);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, pPano->mipmapTex);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pGBPass->colorTex);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, pGBPass->worldPosTex);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, pPano->panoTexId);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, pGBPass->mipmapedFrameDepthTex);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, pGBPass->diff_textureTex);
			renderQuad();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//2. tracing virtual model part in the frame
			glBindFramebuffer(GL_FRAMEBUFFER, pRTPass->fbo);
			glClear(GL_DEPTH_BUFFER_BIT);
			pRTPass->rt2Shader->use();
			pRTPass->rt2Shader->setMat4("projection", projection);
			pRTPass->rt2Shader->setMat4("camView", camView);
			//pRTPass->rt2Shader->setVec3("material.rho", vir_mat->rho);
			pRTPass->rt2Shader->setVec3("material.eta", vir_mat->eta);
			pRTPass->rt2Shader->setVec3("material.micro_k", vir_mat->micro_k);
			pRTPass->rt2Shader->setVec3("material.refspec", vir_mat->refspec);
			pRTPass->rt2Shader->setFloat("material.alpha", vir_mat->alpha);
			pRTPass->rt2Shader->setVec2("iResolution", frameWidth, frameHeight);
			pRTPass->rt2Shader->setVec2("panoSize", pPano->panoWidth, pPano->panoHeight);
			pRTPass->rt2Shader->setInt("frameCount", frameCount);
			pRTPass->rt2Shader->setFloat("min_depth", pPano->min_depth);
			pRTPass->rt2Shader->setFloat("xOff", xOff);
			pRTPass->rt2Shader->setFloat("yOff", yOff);
			pRTPass->rt2Shader->setInt("mipmapDepthTex", 0);
			pRTPass->rt2Shader->setInt("colorTex", 1);
			pRTPass->rt2Shader->setInt("normalTex", 2);
			pRTPass->rt2Shader->setInt("worldPosTex", 3);
			pRTPass->rt2Shader->setInt("mat_prevCoordTex", 4);
			pRTPass->rt2Shader->setInt("panoTex", 5);
			pRTPass->rt2Shader->setInt("newColorTex", 6);
			pRTPass->rt2Shader->setInt("irradTex", 7);
			pRTPass->rt2Shader->setInt("irradRVTex", 8);
			pRTPass->rt2Shader->setInt("mipmapedFrameDepthTex", 9);
			pRTPass->rt2Shader->setInt("traceDataRVTex", 10);
			pRTPass->rt2Shader->setInt("diffTextureTex", 11);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, pPano->mipmapTex);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pGBPass->colorTex);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, pGBPass->worldPosTex);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, pPano->panoTexId);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, pRTPass->newColorTex);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, pRTPass->irradTex);
			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, pRTPass->irradRVTex);
			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, pGBPass->mipmapedFrameDepthTex);
			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D, pRTPass->traceDataRVTex);
			glActiveTexture(GL_TEXTURE11);
			glBindTexture(GL_TEXTURE_2D, pGBPass->diff_textureTex);
			renderQuad();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
}

void PanoAR::outlierRemovalPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pOrPass->fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	pOrPass->orShader->use();
	pOrPass->orShader->setInt("halfKernel", pOrPass->halfKernel);
	pOrPass->orShader->setVec2("iResolution", frameWidth, frameHeight);
	pOrPass->orShader->setInt("rtIrradTex", 0);
	pOrPass->orShader->setInt("rtIrradRVTex", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pRTPass->irradTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pRTPass->irradRVTex);

	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PanoAR::spatialFilteringPass()
{

	glCopyImageSubData(pOrPass->orIrradTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pSFPass->temIrradTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);
	glCopyImageSubData(pOrPass->orIrradRVTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pSFPass->temIrradRVTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);

	for (int i = 0; i < sigma_p; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pSFPass->fbo);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		pSFPass->spfShader->use();
		pSFPass->spfShader->setInt("level", i);
		//pSFPass->spfShader->setFloat("sigma_p", sigma_p);
		pSFPass->spfShader->setFloat("sigma_c", sigma_c);
		pSFPass->spfShader->setFloat("sigma_n", sigma_n);
		pSFPass->spfShader->setFloat("sigma_d", sigma_d);
		pSFPass->spfShader->setFloat("glo_ratio", glo_ratio);
		pSFPass->spfShader->setFloat("sdRatio", sdRatio);
		pSFPass->spfShader->setVec2("iResolution", frameWidth, frameHeight);
		pSFPass->spfShader->setInt("rtIrradTex", 0);
		pSFPass->spfShader->setInt("rtIrradRVTex", 1);
		pSFPass->spfShader->setInt("normalTex", 2);
		pSFPass->spfShader->setInt("worldPosTex", 3);
		pSFPass->spfShader->setInt("mat_prevCoordsTex", 4);
		pSFPass->spfShader->setInt("ratioTex", 5);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pSFPass->temIrradTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pSFPass->temIrradRVTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, pGBPass->worldPosTex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, pRTPass->newColorTex);
		renderQuad();

		glCopyImageSubData(pSFPass->spatialIrradTex, GL_TEXTURE_2D, 0, 0, 0, 0,
			pSFPass->temIrradTex, GL_TEXTURE_2D, 0, 0, 0, 0,
			frameWidth, frameHeight, 1);
		glCopyImageSubData(pSFPass->spatialIrradRVTex, GL_TEXTURE_2D, 0, 0, 0, 0,
			pSFPass->temIrradRVTex, GL_TEXTURE_2D, 0, 0, 0, 0,
			frameWidth, frameHeight, 1);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void PanoAR::temporalFilteringPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pTFPass->fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	pTFPass->temporalShader->use();
	pTFPass->temporalShader->setFloat("color_alpha_min", color_alpha);
	pTFPass->temporalShader->setFloat("moment_alpha_min", moment_alpha);
	pTFPass->temporalShader->setFloat("g_2", g_2);
	pTFPass->temporalShader->setInt("modelMat", pvirModel->models[0].materialId);
	pTFPass->temporalShader->setVec2("iResolution", frameWidth, frameHeight);
	pTFPass->temporalShader->setInt("frameColorTex", 0);
	pTFPass->temporalShader->setInt("mat_prevCoordsTex", 1);
	pTFPass->temporalShader->setInt("modelNormalTex", 2);
	pTFPass->temporalShader->setInt("preMat_CoordsTex", 3);
	pTFPass->temporalShader->setInt("preNormalTex", 4);
	pTFPass->temporalShader->setInt("color_historyTex", 5);
	pTFPass->temporalShader->setInt("moment_historyTex", 6);
	pTFPass->temporalShader->setInt("history_lenthTex", 7);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pSFPass->spatialIrradTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, pTFPass->preMat_PreCoordsTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, pTFPass->preNormalTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, pTFPass->color_historyTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, pTFPass->moment_historyTex);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, pTFPass->history_lengthTex);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PanoAR::temporalFilteringRVPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pTFPassRV->fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	pTFPassRV->temporalShader->use();
	pTFPassRV->temporalShader->setFloat("color_alpha_min", color_alpha);
	pTFPassRV->temporalShader->setFloat("moment_alpha_min", moment_alpha);
	pTFPassRV->temporalShader->setFloat("g_2", g_2);
	pTFPassRV->temporalShader->setInt("modelMat", pvirModel->models[0].materialId);
	pTFPassRV->temporalShader->setVec2("iResolution", frameWidth, frameHeight);
	pTFPassRV->temporalShader->setInt("frameColorTex", 0);
	pTFPassRV->temporalShader->setInt("mat_prevCoordsTex", 1);
	pTFPassRV->temporalShader->setInt("modelNormalTex", 2);
	pTFPassRV->temporalShader->setInt("preMat_CoordsTex", 3);
	pTFPassRV->temporalShader->setInt("preNormalTex", 4);
	pTFPassRV->temporalShader->setInt("color_historyTex", 5);
	pTFPassRV->temporalShader->setInt("moment_historyTex", 6);
	pTFPassRV->temporalShader->setInt("history_lenthTex", 7);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pSFPass->spatialIrradRVTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pGBPass->normalTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, pTFPassRV->preMat_PreCoordsTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, pTFPassRV->preNormalTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, pTFPassRV->color_historyTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, pTFPassRV->moment_historyTex);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, pTFPassRV->history_lengthTex);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PanoAR::copyTemporlBufferTexture()
{
	glCopyImageSubData(pTFPass->color_accTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pTFPass->color_historyTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);
	glCopyImageSubData(pTFPass->moment_accTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pTFPass->moment_historyTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);
	glCopyImageSubData(pTFPass->history_length_updataTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pTFPass->history_lengthTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);

	glCopyImageSubData(pGBPass->normalTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pTFPass->preNormalTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);
	glCopyImageSubData(pGBPass->mat_prevCoordTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pTFPass->preMat_PreCoordsTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);

	glCopyImageSubData(pTFPassRV->color_accTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pTFPassRV->color_historyTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);
	glCopyImageSubData(pTFPassRV->moment_accTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pTFPassRV->moment_historyTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);
	glCopyImageSubData(pTFPassRV->history_length_updataTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pTFPassRV->history_lengthTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);

	glCopyImageSubData(pGBPass->normalTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pTFPassRV->preNormalTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);
	glCopyImageSubData(pGBPass->mat_prevCoordTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		pTFPassRV->preMat_PreCoordsTex, GL_TEXTURE_2D, 0, 0, 0, 0,
		frameWidth, frameHeight, 1);
}

void PanoAR::modulatePass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pModPass->fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	pModPass->modulateShader->use();
	//pModPass->modulateShader->setVec3("rho", vir_mat->rho);
	pModPass->modulateShader->setVec3("refspec", vir_mat->refspec);
	pModPass->modulateShader->setBool("isAcc", ui_isAccumulation);
	pModPass->modulateShader->setInt("frameNum", frameNum);
	pModPass->modulateShader->setInt("modu_sigma", modu_sigma);
	pModPass->modulateShader->setInt("irradTex", 0);
	pModPass->modulateShader->setInt("irradRVTex", 1);
	pModPass->modulateShader->setInt("colorTex", 2);
	pModPass->modulateShader->setInt("mat_prevCoordsTex", 3);
	pModPass->modulateShader->setInt("rtIrradTex", 4);
	pModPass->modulateShader->setInt("rtIrradRVTex", 5);
	pModPass->modulateShader->setInt("accHisIrradTex", 6);
	pModPass->modulateShader->setInt("accHisIrradRVTex", 7);
	pModPass->modulateShader->setInt("albedoTex", 8);
	pModPass->modulateShader->setInt("diffTextureTex", 9);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pSFPass->spatialIrradTex);//temporalIrradBuffer.color_historyTex
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pSFPass->spatialIrradRVTex);//temporalIrradRVBuffer.color_historyTex
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pGBPass->colorTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, pRTPass->irradTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, pRTPass->irradRVTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, pModPass->accHisIrradTex);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, pModPass->accHisIrradRVTex);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, pGBPass->albedoTex);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, pGBPass->diff_textureTex);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void PanoAR::newTemporalPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pNTPass->fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	pNTPass->newTemporalShader->use();
	pNTPass->newTemporalShader->setVec2("iResolution", frameWidth, frameHeight);
	pNTPass->newTemporalShader->setFloat("g_2", g_2);
	pNTPass->newTemporalShader->setInt("imageTex", 0);
	pNTPass->newTemporalShader->setInt("imageAccTex", 1);
	pNTPass->newTemporalShader->setInt("mat_prevCoordsTex", 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pModPass->modulateColorTex);//temporalIrradBuffer.color_historyTex
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pNTPass->imageHisTex);//temporalIrradRVBuffer.color_historyTex
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void PanoAR::display()
{
	glViewport(0, 0, windowWidth, windowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	displayShader->use();
	displayShader->setFloat("lod", lod);
	displayShader->setInt("accTex", 0);
	displayShader->setInt("targetTex", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pModPass->modulateColorTex);//pModPass->accColorTex diff_textureTex
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pNTPass->imageAccTex);//pModPass->modulateColorTex 
	renderQuad();

	glViewport(0, 0, frameWidth, frameHeight);
}

static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoMove;
static bool ui_autoresize = true;
static bool ui_hide = false;

void PanoAR::drawGui()
{
	// Dear imgui new frame
	{
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	// Dear imgui define
	{
		ImVec2 minSize(300.f, 400.f);
		ImVec2 maxSize((float)windowWidth * 0.5, (float)windowHeight);
		ImGui::SetNextWindowSizeConstraints(minSize, maxSize);
		ImGui::SetNextWindowPos(ui_hide ? ImVec2(-1000.f, -1000.f) : ImVec2(0.f, 0.f));

		if (ImGui::IsKeyPressed('H')) {
			ui_hide = !ui_hide;
		}

		ImGui::Begin("Control Panel", 0, windowFlags);
		ImGui::SetWindowFontScale(1);

		// Capture keyboard
		if (ImGui::IsKeyPressed(' ')) {
			ImGui::SetWindowCollapsed(!ImGui::IsWindowCollapsed());
		}

		ImGui::Checkbox("Auto-Resize", &ui_autoresize);
		if (ui_autoresize) {
			windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
		}
		else {
			windowFlags &= ~ImGuiWindowFlags_AlwaysAutoResize;
		}

		ImGui::Checkbox("accumulation", &ui_isAccumulation);

		if (ImGui::CollapsingHeader("Virtual Model", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderInt("mat_id", &pvirModel->models[0].materialId, 1, 2);
			ImGui::SliderFloat("scale.", &pvirModel->models[0].scale, 0.001f, 1.0f);
			ImGui::SliderFloat("rho.r", &vir_mat->rho.x, 0.f, 1.f);
			ImGui::SliderFloat("rho.g", &vir_mat->rho.y, 0.f, 1.f);
			ImGui::SliderFloat("rho.b", &vir_mat->rho.z, 0.f, 1.f);

			ImGui::SliderFloat("refspec.r", &vir_mat->refspec.x, 0.f, 1.f);
			ImGui::SliderFloat("refspec.g", &vir_mat->refspec.y, 0.f, 1.f);
			ImGui::SliderFloat("refspec.b", &vir_mat->refspec.z, 0.f, 1.f);


			ImGui::SliderFloat("alpha", &vir_mat->alpha, 0.f, 0.3f);
		}

		if (ImGui::CollapsingHeader("Outlier Removal", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderInt("halfKernel.", &pOrPass->halfKernel, 0, 15);
		}

		if (ImGui::CollapsingHeader("Spatial filter", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderInt("depth", &sigma_p, 1, 7);
			ImGui::SliderFloat("sigma_c.", &sigma_c, 0.0f, 3.0f);
			ImGui::SliderFloat("sigma_n", &sigma_n, 0.0f, 1.0f);
			ImGui::SliderFloat("sigma_d", &sigma_d, 0.0f, 1.0f);
			ImGui::SliderFloat("glo_ratio", &glo_ratio, 0.01f, 1.0f);
			ImGui::SliderFloat("sdRatio.", &sdRatio, 0.f, 20.0f);
			ImGui::SliderFloat("shadowfade.", &shadowfade, 0.f, 20.0f);
		}

		if (ImGui::CollapsingHeader("Temporal Filter", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("color_alpha.", &color_alpha, 0.0f, 1.0f);
			ImGui::SliderFloat("moment_alpha.", &moment_alpha, 0.0f, 20.0f);
			ImGui::SliderInt("modu_sigma.", &modu_sigma, 1, 16);
			ImGui::SliderFloat("g_2.", &g_2, 0.01, 5.0);
		}

		ImGui::End();
	}

	// Dear imgui render
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

void PanoAR::outputDebugTextures()
{
	//cv::Mat debugData = cv::Mat(cv::Size(frameWidth, frameHeight), CV_32FC3);
	//cv::Mat debugRVData = cv::Mat(cv::Size(frameWidth, frameHeight), CV_32FC3);

	//glBindTexture(GL_TEXTURE_2D, pRTPass->traceDataTex);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, debugData.ptr());

	//cv::flip(debugData, debugData, 0);
	//string name = "debugdata.exr";
	//imwrite(name, debugData);

	//glBindTexture(GL_TEXTURE_2D, pRTPass->traceDataRVTex);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, debugRVData.ptr());

	//cv::flip(debugRVData, debugRVData, 0);
	//string name2 = "debugRVData.exr";
	//imwrite(name2, debugRVData);

	cv::Mat debugData = cv::Mat(cv::Size(frameWidth, frameHeight), CV_32FC3);
	cv::Mat debugRVData = cv::Mat(cv::Size(frameWidth, frameHeight), CV_32FC3);

	glBindTexture(GL_TEXTURE_2D, pModPass->modulateColorTex);// pRTPass->traceDataRVTex  pGBPass->mat_prevCoordTex
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_FLOAT, debugData.ptr());

	cv::flip(debugData, debugData, 0);
	string name = "result.exr";
	imwrite(name, debugData);

	std::ofstream out("result_paras.txt");
	for (int i = 0; i < 6; i++) {
		out << i << "\n"<< pvirModel->models[i].modelName <<endl;
		out <<"scale " << pvirModel->models[i].scale << endl;
		out << "rotateX " << pvirModel->models[i].rotateX << endl;
		out << "rotateY " << pvirModel->models[i].rotateY << endl;
		out << "rotateZ " << pvirModel->models[i].rotateZ << endl;
		out << "translation " << pvirModel->models[i].translation.x << ", " << pvirModel->models[i].translation.y << ", " 
			<< pvirModel->models[i].translation.z <<endl;
		out << "camForward " << forward.x << ", " << forward.y << ", " << forward.z << endl;
		out << "camZoom " << pCam->Zoom << endl;
	}
	out.close();

	//glBindTexture(GL_TEXTURE_2D, pGBPass->mat_prevCoordTex);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, debugRVData.ptr());

	////cv::flip(debugRVData, debugRVData, 0);
	//string name2 = "debugRVData.exr";
	//imwrite(name2, debugRVData);
}

void PanoAR::outputDebugTextures2(int num)
{


	cv::Mat debugData = cv::Mat(cv::Size(frameWidth, frameHeight), CV_32FC3);
	//cv::Mat debugRVData = cv::Mat(cv::Size(frameWidth, frameHeight), CV_32FC3);

	glBindTexture(GL_TEXTURE_2D, pModPass->modulateColorTex);// pRTPass->traceDataRVTex
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_FLOAT, debugData.ptr());

	cv::flip(debugData, debugData, 0);
	string name = "E:/wj/code/panoramic ray tracing/Real-world video/video/M/" + std::to_string(num) +  ".exr";
	imwrite(name, debugData);
}