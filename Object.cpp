#include <string>
#include <iostream>
#include "Object.h"

Object::Object()
{
	hasTexture = false;
}

Object::~Object()
{
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);
}

void Object::releaseObject() {
	//Delete GPU buffers
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	//Clear containers: vertices, texcoords, normals, temp_vertices, temp_normals, vertexIndices,normalIndices
	vertices.clear();
	texcoords.clear();
	normals.clear();
	temp_vertices.clear();
	temp_normals.clear();
	temp_uvs.clear();
	vertexIndices.clear();
	normalIndices.clear();
	uvIndices.clear();

}

void Object::computeCenterScale() {
	float xmin, ymin, zmin;
	float xmax, ymax, zmax;

	xmin = ymin = zmin = 1.0E30F;
	xmax = ymax = zmax = -1.0E30F;

	double cx = 0.0;
	double cy = 0.0;
	double cz = 0.0;

	for (int i = 0; i < temp_vertices.size(); i++) {
		glm::vec3 vertex = temp_vertices[i];
		if (vertex.x < xmin)	xmin = vertex.x;
		else if (vertex.x > xmax)	xmax = vertex.x;
		if (vertex.y < ymin)	ymin = vertex.y;
		else if (vertex.y > ymax)	ymax = vertex.y;
		if (vertex.z < zmin)	zmin = vertex.z;
		else if (vertex.z > zmax)	zmax = vertex.z;

		cx += vertex.x;
		cy += vertex.y;
		cz += vertex.z;
	}

	cx /= temp_vertices.size();
	cy /= temp_vertices.size();
	cz /= temp_vertices.size();

	centerx = (GLfloat)cx;
	centery = (GLfloat)cy;
	centerz = (GLfloat)cz;

	float diag = sqrt((xmax - xmin) * (xmax - xmin) + (ymax - ymin) * (ymax - ymin) + (zmax - zmin) * (zmax - zmin));
	printf("Diag: %f\n", diag);
	scale = 2.0 / diag;
}

void Object::loadFromOBJFile(const char* filename)
{
	loadOBJ(filename, vertices, texcoords, normals, temp_vertices, temp_normals, vertexIndices,normalIndices);
	computeCenterScale();
	
	glGenVertexArrays(1, &VertexArrayID); 
    glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
		
	glVertexAttribPointer(
		0,        // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,        // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0,        // stride
		(void*)0 // array buffer offset
	);
	glEnableVertexAttribArray(0);

	if (hasTexture) {
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), &texcoords[0], GL_STATIC_DRAW);

		glVertexAttribPointer(
			1,        // attribute. No particular reason for 1, but must match the layout in the shader.
			2,        // size : U+V => 2
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0 // array buffer offset
		);
		glEnableVertexAttribArray(1);
	}
}

void Object::loadFromPLYFile(const char* filename)
{
	loadPLY(filename, vertices, texcoords, normals, temp_vertices, temp_normals, vertexIndices,normalIndices);
	computeCenterScale();
	
	glGenVertexArrays(1, &VertexArrayID); 
    glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
		
	glVertexAttribPointer(
		0,        // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,        // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0,        // stride
		(void*)0 // array buffer offset
	);
	glEnableVertexAttribArray(0);

	if (hasTexture) {
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), &texcoords[0], GL_STATIC_DRAW);

		glVertexAttribPointer(
			1,        // attribute. No particular reason for 1, but must match the layout in the shader.
			2,        // size : U+V => 2
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0 // array buffer offset
		);
		glEnableVertexAttribArray(1);
	}
}

void Object::loadFromOFFFile(const char* filename)
{
	loadOFF(filename, vertices, texcoords, normals);
	
	/*for(int i=0; i < vertices.size(); i++){
		std::cout << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << std::endl;
 	}*/

	glGenVertexArrays(1, &VertexArrayID); 
    glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(
		0,        // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,        // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0,        // stride
		(void*)0 // array buffer offset
	);
	glEnableVertexAttribArray(0);

	if (hasTexture) {
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), &texcoords[0], GL_STATIC_DRAW);

		glVertexAttribPointer(
			1,        // attribute. No particular reason for 1, but must match the layout in the shader.
			2,        // size : U+V => 2
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0 // array buffer offset
		);
		glEnableVertexAttribArray(1);
	}
}

void Object::loadTexture(const char* filename)
{
	Texture = loadGeneralTexture(filename);
	std::cout << "Texture:" << Texture << std::endl;
	hasTexture = true;
}

void Object::drawObject(GLuint program)
{
	if (hasTexture) {
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		TextureID = glGetUniformLocation(program, "myTextureSampler");
		glUniform1i(TextureID, 0);
	}
	glBindVertexArray(VertexArrayID);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 3 indices starting at 0 -> 1 triangle

}

void Object::fixOrientation(std::vector<glm::vec3>& selected)
{
	printf("Scale before: %f %f %f\n", centerx, centery, centerz);
	
	glm::vec3 baricentro = (selected[0] + selected[1] + selected[2]) / 3.0f;

	//Calcular el nuevo sistema de coordenadas
	glm::vec3 newX = glm::normalize(selected[0] - baricentro); //Este ser� el nuevo X
	glm::vec3 vec2 = glm::normalize(selected[1] - baricentro);

	glm::vec3 newY = glm::normalize(glm::cross(newX, vec2));
	glm::vec3 newZ = glm::normalize(glm::cross(newX, newY));

	//Construir la matriz de transformaci�n para normales
	glm::mat3 transNormal;
	glm::mat3 trans;
	trans = glm::mat3(glm::vec3(newX.x, newY.x, newZ.x), glm::vec3(newX.y, newY.y, newZ.y), glm::vec3(newX.z, newY.z, newZ.z));
	transNormal = glm::transpose(glm::inverse(trans));

	double cx = 0.0;
	double cy = 0.0;
	double cz = 0.0;
	//Transformar los puntos y normales
	for (int i = 0; i < temp_vertices.size(); i++) {
		glm::vec3 vertex =trans*(temp_vertices[i]-glm::vec3(-centerx, -centery, -centerz));
		temp_vertices[i].x = vertex.x;
		temp_vertices[i].y = vertex.y;
		temp_vertices[i].z = vertex.z;

		cx += vertex.x;
		cy += vertex.y;
		cz += vertex.z;
		

		glm::vec3 normal = transNormal * temp_normals[i];
		temp_normals[i].x = normal.x;
		temp_normals[i].y = normal.y;
		temp_normals[i].z = normal.z;
	}

	
	cx /= temp_vertices.size();
	cy /= temp_vertices.size();
	cz /= temp_vertices.size();
	
	printf("Scale Iter: %f %f %f\n", cx, cy, cz);

	for (int i = 0; i < temp_vertices.size(); i++) {
		temp_vertices[i].x = temp_vertices[i].x - (GLfloat)cx;
		temp_vertices[i].y = temp_vertices[i].y - (GLfloat)cy;
		temp_vertices[i].z = temp_vertices[i].z - (GLfloat)cz;
	}

	//Necesitamos recomponer el buffer de vertices y normales
	vertices.clear();
	normals.clear();

	for (int i = 0; i < vertexIndices.size(); i++) {
		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		vertices.push_back(vertex);
		normals.push_back(normal);
	}

	computeCenterScale();
	printf("Scale after: %f %f %f\n", centerx, centery, centerz);

	//Administrar el buffer del GPU
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), &vertices[0]);
}
