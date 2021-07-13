#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <vector>
#include <map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <common/texture.hpp>
#include <common/objloader.hpp>
#include <string>
#include <map>


#pragma once
class Object
{
public:

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > texcoords;
	std::vector< glm::vec3 > normals;

	std::vector<unsigned int> vertexIndices;
	std::vector<unsigned int> uvIndices;
	std::vector<unsigned int> normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;
	
	
	GLuint Texture;
	GLuint TextureID;
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint VertexArrayID;

	bool hasTexture;

	GLfloat scale;
	GLfloat centerx, centery, centerz;

	Object();
	~Object();

	void loadFromOBJFile(const char* filename);
	void loadFromPLYFile(const char* filename);
	void loadFromOFFFile(const char* filename);
	void loadTexture(const char* filename);
	void drawObject(GLuint program);
	void fixOrientation(std::vector<glm::vec3>& selected);
	void computeCenterScale();
	void releaseObject();

};

#endif
