#include "panoramaScene.h"

inline void readImgfromEXR(cv::Mat &m, std::string filename)
{
	cv::Mat src = cv::imread(filename, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
	cv::Mat alpha = cv::Mat(src.size(), CV_MAKE_TYPE(src.type(), 1), cv::Scalar(1.0));
	m = cv::Mat(src.size(), CV_MAKE_TYPE(src.type(), 4));
	int from_to[] = { 0,0,1,1,2,2,3,3 };

	cv::Mat inMat[] = { src,alpha }; 

	cv::mixChannels(inMat, 2, &m, 1, from_to, 4);
	//flip(m, m, 0);
}

inline void readImgfromEXR2(cv::Mat &m, std::string filename)
{
	m = cv::imread(filename, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
}

//inline void readImgfromPNG(cv::Mat &m, std::string filename)
//{
//	cv::Mat src1 = cv::imread(filename);
//	cv::Mat src = cv::Mat(src.size(), CV_32FC4);
//	//cv::Mat alpha = cv::Mat(src.size(), CV_MAKE_TYPE(src.type(), 1), cv::Scalar(1.0));
//}

inline void readImgfromXML(cv::Mat &m, std::string cap, std::string filename)
{
	cv::Mat src;
	cv::FileStorage fs(filename, cv::FileStorage::READ);
	fs[cap] >> src;
	fs.release();

	cv::Mat alpha = cv::Mat(src.size(), CV_MAKE_TYPE(src.type(), 1), cv::Scalar(1.0));
	m = cv::Mat(src.size(), CV_MAKE_TYPE(src.type(), 4));
	int from_to[] = { 0,0,1,1,2,2,3,3 };

	cv::Mat inMat[] = { src,alpha };

	cv::mixChannels(inMat, 2, &m, 1, from_to, 4);


	//flip(m, m, 0);
	//normalize(m, m, 1, 0, CV_MINMAX);
}

inline void BindEXRTextureID(GLuint &texid, GLint width, GLint height, float *ptr)
{
	if(!texid)
		glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_BGRA_EXT, GL_FLOAT, ptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

panoramaScene::panoramaScene()
{
}


panoramaScene::~panoramaScene()
{
}

panoramaScene::panoramaScene(std::string panoPath, std::string panoNormalPath, std::string panoPosPath, bool isdataset)
{
	isDataSet = isdataset;
	readInPano(panoPath, panoNormalPath, panoPosPath);
	genDepthMipMapCPU();
}

void panoramaScene::readInPano(std::string panoPath, std::string panoNormalPath, std::string panoPosPath)
{
	if (isDataSet) {
		//readImgfromPNG(panoData, panoPath);
	}
	else {
		readImgfromEXR(panoData, panoPath);
		readImgfromEXR(panoPos, panoPosPath);
		readImgfromEXR(panoNormal, panoNormalPath);

		panoWidth = panoData.cols;
		panoHeight = panoData.rows;

		BindEXRTextureID(panoTexId, panoWidth, panoHeight, (float*)panoData.data);
		BindEXRTextureID(panoPosTexId, panoWidth, panoHeight, (float*)panoPos.data);
		BindEXRTextureID(panoNormalTexId, panoWidth, panoHeight, (float*)panoNormal.data);
	}
}

void panoramaScene::genDepthMipMapCPU()
{
	max_level = log(panoWidth) / log(2);
	std::vector<cv::Mat> mm;
	int width = panoWidth, height = panoHeight;
	for (int i = 0; i < max_level; i++) {
		mm.push_back(cv::Mat(height, width, CV_32FC1, cv::Scalar(0)));
		width /= 2;
		height /= 2;
	}
	for (int i = 0; i < max_level; i++)
	{
		for (int x = 0; x < mm[i].cols; x++)
		{
			for (int y = 0; y < mm[i].rows; y++)
			{
				if (i == 0) {
					cv::Vec4f temp = panoPos.at<cv::Vec4f>(y, x);
					float len = glm::length(glm::vec3(temp[0], temp[1], temp[2]));
					mm[i].at<float>(y, x) = len;// cv::Vec4f(len, len, len, 1);
				}
				else
				{
					float v0 = mm[i - 1].at<float>(y * 2, x * 2);
					float v = MIN(v0, mm[i - 1].at<float>(y * 2 + 1, x * 2));
					v = MIN(v, mm[i - 1].at<float>(y * 2, x * 2 + 1));
					float len = MIN(v, mm[i - 1].at<float>(y * 2 + 1, x * 2 + 1));
					mm[i].at<float>(y, x) = len;// cv::Vec4f(len, len, len, 1);
				}
			}
		}
	}

	min_depth = MIN(mm[max_level - 1].at<float>(0, 0), mm[max_level - 1].at<float>(0, 1));//mm[max_level - 1].at<float>(0, 0);// 

	glGenTextures(1, &mipmapTex);
	glBindTexture(GL_TEXTURE_2D, mipmapTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, panoWidth, panoHeight, 0, GL_RED, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_level);

	for (int i = 0; i < max_level; i++) {
		glTexImage2D(GL_TEXTURE_2D, i, GL_R16F, mm[i].cols, mm[i].rows, 0, GL_RED, GL_FLOAT, (float*)mm[i].data);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void panoramaScene::updatePano(int num)
{
	std::string panoPath = "E:/wj/code/panoramic ray tracing/Real-world video/videoColor/B/";
	std::string panoNormalPath = "E:/wj/code/panoramic ray tracing/Real-world video/videoNormal/B/";
	std::string panoPosPath = "E:/wj/code/panoramic ray tracing/Real-world video/videoPos/B/";

	readImgfromEXR(panoData, panoPath + std::to_string(num) + ".exr");
	readImgfromEXR(panoPos, panoPosPath + std::to_string(num) + ".exr");
	readImgfromEXR(panoNormal, panoNormalPath + std::to_string(num) + ".exr");

	panoWidth = panoData.cols;
	panoHeight = panoData.rows;

	BindEXRTextureID(panoTexId, panoWidth, panoHeight, (float*)panoData.data);
	BindEXRTextureID(panoPosTexId, panoWidth, panoHeight, (float*)panoPos.data);
	BindEXRTextureID(panoNormalTexId, panoWidth, panoHeight, (float*)panoNormal.data);


	genDepthMipMapCPU();
}