#ifndef OBJLOADER_H
#define OBJLOADER_H

bool loadOBJ(
	const char * path, 
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs, 
	std::vector<glm::vec3> & out_normals,
	std::vector<glm::vec3>& temp_vertices,
	std::vector<glm::vec3>& temp_normals,
	std::vector<unsigned int>& vertexIndices,
	std::vector<unsigned int>& normalIndice
);

bool loadPLY(
	const char* path,
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs, 
	std::vector<glm::vec3> & out_normals,
	std::vector<glm::vec3>& temp_vertices,
	std::vector<glm::vec3>& temp_normals,
	std::vector<unsigned int>& vertexIndices,
	std::vector<unsigned int>& normalIndice
);

void saveOBJ(const char* sourcePath, const char* savePath, 
	std::vector<glm::vec3>& temp_vertices,
	std::vector<glm::vec3>& temp_normals
);

void saveOFF(const char* savePath, std::vector<glm::vec3>& vertices, 
									std::vector<unsigned int>& indices);
bool loadOFF(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
);

bool loadOFF2(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec3>& out_normals,
	std::vector<unsigned int>& out_indices
);

bool loadAssImp(
	const char * path, 
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals
);

#endif