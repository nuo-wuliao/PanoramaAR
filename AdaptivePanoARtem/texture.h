#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <GLEW/glew.h>
#include <iostream>
#include <fstream>
#include <SOIL/SOIL.h>

class TextureHelper
{
public:
	/*
	/* 成功加载2D纹理则返回纹理对象Id 否则返回0                                                                
	*/
	static  GLuint load2DTexture(const char* filename, GLint internalFormat = GL_RGBA,
		GLenum picFormat = GL_RGBA, int loadChannels = SOIL_LOAD_RGBA)
	{
		// Step1 创建并绑定纹理对象
		GLuint textureId = 0;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		// Step2 设定wrap参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Step3 设定filter参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
			GL_LINEAR_MIPMAP_LINEAR); // 为MipMap设定filter方法
		// Step4 加载纹理
		GLubyte *imageData = NULL;
		int picWidth, picHeight;
		int channels = 0;
		imageData = SOIL_load_image(filename, &picWidth, &picHeight, &channels, loadChannels);
		if (imageData == NULL)
		{
			std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
			return 0;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, picWidth, picHeight, 
			0, picFormat, GL_UNSIGNED_BYTE, imageData);
		glGenerateMipmap(GL_TEXTURE_2D);
		// Step5 释放纹理图片资源
		SOIL_free_image_data(imageData);
		glBindTexture(GL_TEXTURE_2D, 0);
		return textureId;
	}
#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

	static GLuint loadDDS(const char * filename){

		
		/* try to open the file */
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		if (!file){
			std::cout << "Error::loadDDs, could not open:" 
				<< filename << "for read." << std::endl;
			return 0;
		}

		/* verify the type of file */
		char filecode[4];
		file.read(filecode, 4);
		if (strncmp(filecode, "DDS ", 4) != 0) {
			std::cout << "Error::loadDDs, format is not dds :"
				<< filename  << std::endl;
			file.close();
			return 0;
		}

		/* get the surface desc */
		char header[124];
		file.read(header, 124);

		unsigned int height = *(unsigned int*)&(header[8]);
		unsigned int width = *(unsigned int*)&(header[12]);
		unsigned int linearSize = *(unsigned int*)&(header[16]);
		unsigned int mipMapCount = *(unsigned int*)&(header[24]);
		unsigned int fourCC = *(unsigned int*)&(header[80]);


		char * buffer = NULL;
		unsigned int bufsize;
		/* how big is it going to be including all mipmaps? */
		bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
		buffer = new char[bufsize];
		file.read(buffer, bufsize);
		/* close the file pointer */
		file.close();

		unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
		unsigned int format;
		switch (fourCC)
		{
		case FOURCC_DXT1:
			format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			break;
		case FOURCC_DXT3:
			format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;
		case FOURCC_DXT5:
			format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		default:
			delete[] buffer;
			return 0;
		}

		// Create one OpenGL texture
		GLuint textureID;
		glGenTextures(1, &textureID);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
		unsigned int offset = 0;

		/* load the mipmaps */
		for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
		{
			unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
			glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
				0, size, buffer + offset);

			offset += size;
			width /= 2;
			height /= 2;

			// Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
			if (width < 1) width = 1;
			if (height < 1) height = 1;

		}

		delete[] buffer;

		return textureID;
	}

	//static GLuint loadMatToTexture(string filename, GLenum minFilter,
	//	GLenum magFilter, GLenum wrapFilter)
	//{
	//	cv::Mat mat = cv::imread(filename, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);

	//	// Generate a number for our textureID's unique handle
	//	GLuint textureID;
	//	glGenTextures(1, &textureID);

	//	// Bind to our texture handle
	//	glBindTexture(GL_TEXTURE_2D, textureID);

	//	// Catch silly-mistake texture interpolation method for magnification
	//	if (magFilter == GL_LINEAR_MIPMAP_LINEAR ||
	//		magFilter == GL_LINEAR_MIPMAP_NEAREST ||
	//		magFilter == GL_NEAREST_MIPMAP_LINEAR ||
	//		magFilter == GL_NEAREST_MIPMAP_NEAREST)
	//	{
	//		cout << "You can't use MIPMAPs for magnification - setting filter to GL_LINEAR" << endl;
	//		magFilter = GL_LINEAR;
	//	}

	//	// Set texture interpolation methods for minification and magnification
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	//	// Set texture clamping method
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);

	//	// Set incoming texture format to:
	//	// GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
	//	// GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
	//	// Work out other mappings as required ( there's a list in comments in main() )
	//	GLenum inputColourFormat = GL_BGR_EXT;
	//	if (mat.channels() == 1)
	//	{
	//		inputColourFormat = GL_LUMINANCE;
	//	}

	//	// Create the texture
	//	glTexImage2D(GL_TEXTURE_2D,     // Type of texture
	//		0,                 // Pyramid level (for mip-mapping) - 0 is the top level
	//		GL_RGB,            // Internal colour format to convert to
	//		mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
	//		mat.rows,          // Image height i.e. 480 for Kinect in standard mode
	//		0,                 // Border width in pixels (can either be 1 or 0)
	//		inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
	//		GL_FLOAT,			//GL_UNSIGNED_BYTE,  // Image data type
	//		mat.ptr());        // The actual image data itself

	//						   // If we're using mipmaps then generate them. Note: This requires OpenGL 3.0 or higher
	//	if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
	//		minFilter == GL_LINEAR_MIPMAP_NEAREST ||
	//		minFilter == GL_NEAREST_MIPMAP_LINEAR ||
	//		minFilter == GL_NEAREST_MIPMAP_NEAREST)
	//	{
	//		glGenerateMipmap(GL_TEXTURE_2D);
	//	}

	//	return textureID;
	//}

};

#endif