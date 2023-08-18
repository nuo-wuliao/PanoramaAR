#pragma once
#include <iostream>
#include "mesh.h"
#include "model.h"

struct myModel {
	Model *objmodel;
	std::string modelName;
	int materialId;
	glm::mat4 model, last_model;
	float scale, rotateX, rotateY, rotateZ;
	glm::vec3 translation;
};

class virtualModel
{
public:
	virtualModel();
	~virtualModel();
	virtualModel(std::string obj_filename);

	void loadOBJModel(std::string obj_filename);
	void setupTransform();
	void updateObjModelMat(glm::vec3 side, glm::vec3 up, glm::vec3 forward);

public:
	myModel models[6];
	int modelNum;
	//Model *objmodel;
	//int materialId;
	//glm::mat4 model, last_model;
	//float scale, rotateX, rotateY, rotateZ;
	//glm::vec3 translation;
};

