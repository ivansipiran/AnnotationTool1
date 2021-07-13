#pragma once
#ifndef __VIRTUALCANVAS_H_
#include <GL/glew.h> 
#include "Object.h"

class VirtualCanvas
{
public:
	GLuint textureFB;
	unsigned int framebuffer;
	unsigned int rbo;

	unsigned int canvasWidth;
	unsigned int canvasHeight;
	std::string outputFilename;

	VirtualCanvas();
	VirtualCanvas(unsigned int width, unsigned int height);

	void render(Object& shape, glm::mat4& matrix, GLuint& programID, float maxRadius);
	std::string getFilename() { return outputFilename; }

	static void write_png(std::string filename, unsigned int width, unsigned int height, GLubyte* pixels);
};

#endif
