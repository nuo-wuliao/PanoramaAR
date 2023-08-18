#include "virtualModel.h"

/*************************VR23 teaser*******************************/
int mat_array[6] = { 2,2,1,1,1,2 };
float scale[6] = { 1.0, 0.248, 0.264, 0.521, 0.521, 0.212 };
float rotateY[6] = { 5.29, 0.5, 0.14, 0.34, 0.34, 6.06 };
float rotateX[6] = { 0,0,0,0,0,0 };
float rotateZ[6] = { 0,0,0,0,0,0 };
glm::vec3 translation[6] = {
	glm::vec3(-1.8, -1.41, -2.27), //hanger
	glm::vec3(-0.370001, -0.970001, -0.729998), //bottle
	glm::vec3(-0.200001, -0.95, -0.499998), //presentbox
	glm::vec3(-0.480001, -0.7, -2.42),//tissuebox
	glm::vec3(-0.480001, -0.7, -2.42), 
	glm::vec3(-0.06, -0.85, -1.175)
};
/*************************hanger bag end*******************************/

/*************************hanger bag start*******************************/
//int mat_array[6] = { 2,1,1,2,1,2};
//float scale[6] = { 0.248, 0.264, 0.027, 1.0, 0.521, 0.212};
//float rotateY[6] = { 0, 4.75001, 5.29, 3.32, 0.34, 6.06 };
//float rotateX[6] = { 0,0,0,0.02,0,0 };
//float rotateZ[6] = { 0,0,0,6.17005,0,0 };
//glm::vec3 translation[6] = {
//	glm::vec3(-0.630001, -1.06, -1.14), //bottle
//	glm::vec3(-0.230001, -1.12, -0.509998), //present Box
//	glm::vec3(1.79, -1.68, 0.939998),//glm::vec3(-1.3863, -3.37444, 1.9639),//-1.3863, -3.37444, , 1.9639 1.68252  -0.44, -1.37, 2.61
//	glm::vec3(1.95, -1.82, 0.709998),
//	glm::vec3(-0.480001, -0.7, -2.42), //tissuebox
//	glm::vec3(-0.06, -0.85, -1.175)
//};
/*************************hanger bag end*******************************/

/*************************apt5 start*******************************/
//int mat_array[6] = { 1,1,2,2,2,1 };
//float scale[6] = { 0.027,  0.444, 0.243, 0.166, 0.166, 0.258 };
//float rotateY[6] = { 0, 0, 5.18, 4.74, 5.5, 0.38 };
//glm::vec3 translation[6] = {
//	glm::vec3(1.73, -0.939999, 1.45),//-0.0499985, -0.944999, 1.45
//	glm::vec3(-0.48, -1.113, 1.67),
//	glm::vec3(1.73, -1.065, 1.16),
//	glm::vec3(0.605, -0.355, -0.4),
//	glm::vec3(0.395001, -0.35, -0.624998),
//	glm::vec3(0.28, -0.907993, -1.15)
//};
/*************************apt5 end*******************************/

//int mat_array[6] = { 1,2,2,1,2,1 };
//float scale[6] = { 0.011,0.029, 0.228, 0.02, 0.027, 0.258 };
//float rotateY[6] = { 2.1,5.62, 5.5, 4.74, 0.38,0.38 };
//glm::vec3 translation[6] = {
//	glm::vec3(-1.1113, -1.00949, -2.63111),//sphere -0.9163, -0.360499, -0.0561002 //-0.109999, -0.896993, -1.16
//	glm::vec3(2.36001, -0.862992, -0.165),//-0.0499985, -0.944999, 1.45
//	glm::vec3(2.20001, -0.824992, 0.51),
//	glm::vec3(1.69, -1.47501, -0.39),
//	glm::vec3(-1.08, -0.988992, -1.995),
//	glm::vec3(0.28, -0.907993, -1.15)
//};

virtualModel::virtualModel()
{
}


virtualModel::~virtualModel()
{
}

virtualModel::virtualModel(std::string obj_filename)
{
	/*last_model = glm::mat4(1.f);
	model = glm::mat4(1.f);
	loadOBJModel(obj_filename);
	materialId = 1;
	setupTransform();*/
	modelNum = 6;
	loadOBJModel(obj_filename);
	//materialId = 1;
	setupTransform();
}

void virtualModel::loadOBJModel(std::string obj_filename)
{
	/*std::ifstream modelPath(obj_filename);
	if (!modelPath) {
		std::cerr << "Error :: could not read model path file." << std::endl;
		return;
	}
	std::string modelFilePath;
	std::getline(modelPath, modelFilePath);
	if (modelFilePath.empty()) {
		std::cerr << "Error::model path empty" << std::endl;
		return;
	}
	objmodel = new Model();
	if (!objmodel->loadModel(modelFilePath)) {
		return;
	}*/

	std::ifstream modelPath(obj_filename);
	if (!modelPath) {
		std::cerr << "Error :: could not read model path file." << std::endl;
		return;
	}
	for (int i = 0; i < modelNum; i++) {
		std::string modelFilePath;
		std::getline(modelPath, modelFilePath);
		if (modelFilePath.empty()) {
			std::cerr << "Error::model path empty" << std::endl;
			return;
		}
		models[i].objmodel = new Model();
		models[i].modelName = modelFilePath;
		if (!models[i].objmodel->loadModel(modelFilePath)) {
			return;
		}
	}

}

void virtualModel::setupTransform()
{
	for (int i = 0; i < modelNum; i++) {
		models[i].last_model = glm::mat4(1.f);
		models[i].model = glm::mat4(1.f);
		models[i].materialId = mat_array[i];
		models[i].scale = scale[i];
		models[i].translation = translation[i];
		models[i].rotateX = rotateX[i];
		models[i].rotateY = rotateY[i];
		models[i].rotateZ = rotateZ[i];
	}

	//models[0].rotateX = 6.26;
	//models[0].rotateZ = 6.22;

	//bunny
	/*scale = 0.035f;
	translation = glm::vec3(-0.5563f, -0.2135f, -0.3261f);
	rotateX = 0.f;
	rotateY = 1.5f;
	rotateZ = 0.f;*/
	//translateX = -0.5363f;// 0.0;//-1.2;//-0.5063f;//-2.35; // 0.6f;// -2.35, 0.8, 0.6 
	//translateY = -0.0335f;//0.01;//-0.5;//-0.2135f;// 0.8; //  1.2f;// 0.0, 0.01, -0.9
	//translateZ = -0.2961f;//-0.9;// -0.11;//-0.2961f;//0.6;//  -4.0f;// (-0.0063f, -0.2135f, -0.5961f)//-1.2, -0.5, -0.11

	//sphere
	//scale = 0.2f;
	//translation = glm::vec3(-1.41f, -1.055f, 0.005f);//-1.2f, -0.5f, -0.11f
	//rotateX = 0.f;
	//rotateY = 0.f;
	//rotateZ = 0.f;

	//teapot
	//scale = 0.006f;
	//translation = glm::vec3(0.21f, -0.11f, -0.37f);//-1.2f, -0.5f, -0.11f
	//rotateX = 0.f;
	//rotateY = 2.72f;
	//rotateZ = 0.f;

	//buddha
	//scale = 0.01f;
	//translation = glm::vec3(0.6f, -0.8f, 1.3f);//-1.2f, -0.5f, -0.11f
	//rotateX = 0.f;
	//rotateY = 4.f;
	//rotateZ = 0.f;

	////dragon
	//scale = 0.024f;
	//translation = glm::vec3(-1.02f, -0.18f, -0.136f);//-1.2f, -0.5f, -0.11f
	//rotateX = 0.f;
	//rotateY = 4.54f;
	//rotateZ = 0.f;

	//temp

	//scale = 0.006f;
	//translation = glm::vec3(0.21, -0.11, -0.37);//-1.2f, -0.5f, -0.11f
	//rotateX = 0.f;
	//rotateY = 2.72f;
	//rotateZ = 0.f;
}

void virtualModel::updateObjModelMat(glm::vec3 side, glm::vec3 up, glm::vec3 forward)
{
	for (int i = 0; i < modelNum; i++) {
		models[i].last_model = models[i].model;
		glm::mat4 temp = glm::mat4(1.f);
		temp = glm::translate(temp, models[i].translation);
		temp = glm::scale(temp, glm::vec3(models[i].scale));
		temp = glm::rotate(temp, models[i].rotateX, glm::vec3(1, 0, 0));
		temp = glm::rotate(temp, models[i].rotateZ, glm::vec3(0, 0, 1));
		temp = glm::rotate(temp, models[i].rotateY, glm::vec3(0, 1, 0));
		models[i].model = temp;
	}
	/*this->last_model = this->model;
	glm::mat4 temp = glm::mat4(1.f);
	temp = glm::translate(temp, translation);
	temp = glm::scale(temp, glm::vec3(scale));
	temp = glm::rotate(temp, rotateX, glm::vec3(1,0,0));
	temp = glm::rotate(temp, rotateY, glm::vec3(0, 1, 0));
	temp = glm::rotate(temp, rotateZ, glm::vec3(0, 0, 1));
	this->model = temp;*/
}