#pragma once
#ifndef __PATTERN_H_
#define __PATTERN_H_

#include <GL/glew.h> 
#include <string>

class Pattern
{
public:
	std::string filename;
	unsigned int patternWidth;
	unsigned int patternHeight;

	GLuint textureId;
	
	Pattern() { textureId = -1; patternWidth = 0; patternHeight = 0; }
	Pattern(std::string filename);
	~Pattern();

	GLuint getTextureId() { return textureId; }
	unsigned int getPatternWidth() { return patternWidth; }
	unsigned int getPatternHeight() { return patternHeight; }
	bool loadTextureFromFile(const char* filename);
	
};

#endif

