#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "objloader.hpp"

bool loadOFF(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	char lineHeader[128];

	int res = fscanf(file, "%s", lineHeader);
	if (strcmp(lineHeader, "OFF")) {
		printf("Error during reading OFF format\n");
		return false;
	}

	int nv, nf, ne;

	fscanf(file, "%d %d %d\n", &nv, &nf, &ne);
	for (int i = 0; i < nv; i++) {
		glm::vec3 vertex;
		fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
		temp_vertices.push_back(vertex);
	}

	for (int i = 0; i < nf; i++) {
		unsigned int index[3];
		unsigned int count;
		fscanf(file, "%d %d %d %d\n", &count, &index[0], &index[1], &index[2]);
		vertexIndices.push_back(index[0]);
		vertexIndices.push_back(index[1]);
		vertexIndices.push_back(index[2]);
	}

	float xmin, ymin, zmin;
	float xmax, ymax, zmax;
	float cx = 0.0, cy = 0.0, cz = 0.0;

	xmin = ymin = zmin = 1.0E30F;
	xmax = ymax = zmax = -1.0E30F;

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
	
		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex];
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		
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

	cx /= vertexIndices.size();
	cy /= vertexIndices.size();
	cz /= vertexIndices.size();

	float diag = sqrt((xmax - xmin) * (xmax - xmin) + (ymax - ymin) * (ymax - ymin) + (zmax - zmin) * (zmax - zmin));
	printf("Diag: %f\n", diag);
	diag = 1.0 / diag;

	xmin = ymin = zmin = 1.0E30F;
	xmax = ymax = zmax = -1.0E30F;

	for (int i = 0; i < out_vertices.size(); i++) {
		out_vertices[i].x = (out_vertices[i].x - cx) * 1.0 * diag;
		out_vertices[i].y = (out_vertices[i].y - cy) * 1.0 * diag;
		out_vertices[i].z = (out_vertices[i].z - cz) * 1.0 * diag;

		if (out_vertices[i].x < xmin)	xmin = out_vertices[i].x;
		else if (out_vertices[i].x > xmax)	xmax = out_vertices[i].x;
		if (out_vertices[i].y < ymin)	ymin = out_vertices[i].y;
		else if (out_vertices[i].y > ymax)	ymax = out_vertices[i].y;
		if (out_vertices[i].z < zmin)	zmin = out_vertices[i].z;
		else if (out_vertices[i].z > zmax)	zmax = out_vertices[i].z;
	}

	printf("%f %f\n", xmin, xmax);
	printf("%f %f\n", ymin, ymax);
	printf("%f %f\n", zmin, zmax);

	fclose(file);
	return true;
}

//Carga un archivo OFF y computa las normales de cada vértice como el promedio de normales de las caras aledañas
bool loadOFF2(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec3>& out_normals,
	std::vector<unsigned int>& out_indices
) {
	printf("Loading OFF file %s...\n", path);

	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	char lineHeader[128];

	int res = fscanf(file, "%s", lineHeader);
	if (strcmp(lineHeader, "OFF")) {
		printf("Error during reading OFF format\n");
		return false;
	}

	int nv, nf, ne;

	fscanf(file, "%d %d %d\n", &nv, &nf, &ne);
	for (int i = 0; i < nv; i++) {
		glm::vec3 vertex;
		fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
		out_vertices.push_back(vertex);
	}

	for (int i = 0; i < nf; i++) {
		unsigned int index[3];
		unsigned int count;
		fscanf(file, "%d %d %d %d\n", &count, &index[0], &index[1], &index[2]);
		out_indices.push_back(index[0]);
		out_indices.push_back(index[1]);
		out_indices.push_back(index[2]);
	}

	printf("Read OFF!\n");
	//std::vector<glm::vec3> normalTriangle;
	std::vector<float> normalVert(3*nv);
	std::vector<int> countNormals(nv);

	//Calcular normales
	for(int i = 0; i < nf; i++){
		float nx = 0.0, ny = 0.0, nz = 0.0;
		unsigned int vert[3] = {out_indices[3*i], out_indices[3*i+1], out_indices[3*i+2]};

		unsigned int ind1 = 2;
        for(unsigned int j = 0; j < 3; j++){
            int ind2 = j;
            nx += (out_vertices[vert[ind1]].y - out_vertices[vert[ind2]].y)*(out_vertices[vert[ind1]].z + out_vertices[vert[ind2]].z);
            ny += (out_vertices[vert[ind1]].z - out_vertices[vert[ind2]].z)*(out_vertices[vert[ind1]].x + out_vertices[vert[ind2]].x);
            nz += (out_vertices[vert[ind1]].x - out_vertices[vert[ind2]].x)*(out_vertices[vert[ind1]].y + out_vertices[vert[ind2]].y);
            ind1 = ind2;
        }

        float magnitude = sqrt(nx*nx + ny*ny + nz*nz);
        if(magnitude > 1.0E-6){
            nx /= magnitude;
            ny /= magnitude;
            nz /= magnitude;
        }
        
		normalVert[3*vert[0]] += nx;	normalVert[3*vert[0]+1] += ny;	normalVert[3*vert[0]+2] += nz;	countNormals[vert[0]]++;
		normalVert[3*vert[1]] += nx;	normalVert[3*vert[1]+1] += ny;	normalVert[3*vert[1]+2] += nz;	countNormals[vert[1]]++;
		normalVert[3*vert[2]] += nx;	normalVert[3*vert[2]+1] += ny;	normalVert[3*vert[2]+2] += nz;	countNormals[vert[2]]++;
	}

	printf("Normales faces!\n");

	for(int i = 0; i < nv; i++){
		normalVert[3*i] = normalVert[3*i]/countNormals[i];
		normalVert[3*i+1] = normalVert[3*i+1]/countNormals[i];
		normalVert[3*i+2] = normalVert[3*i+2]/countNormals[i];

		float mag = sqrt(normalVert[3*i]*normalVert[3*i] + normalVert[3*i + 1]*normalVert[3*i + 1] + normalVert[3*i + 2]*normalVert[3*i + 2]);

		normalVert[3*i] /= mag;
		normalVert[3*i+1] /=mag;
		normalVert[3*i+2] /=mag;

		out_normals.push_back(glm::vec3(normalVert[3*i],normalVert[3*i+1],normalVert[3*i+2]));

	}

	printf("Normales vertex!\n");

	fclose(file);
	return true;
}

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals,
	std::vector<glm::vec3>& temp_vertices,
	std::vector<glm::vec3>& temp_normals,
	std::vector<unsigned int>& vertexIndices,
	std::vector<unsigned int>& normalIndices
){
	std::vector<unsigned int> uvIndices;

	std::vector<glm::vec2> temp_uvs;
	printf("Loading OBJ file %s...\n", path);

	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		
		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// Recorre cada indice de cada triangulo
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];
		
		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);
	}

	fclose(file);
	return true;
}

bool loadPLY(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals,
	std::vector<glm::vec3>& temp_vertices,
	std::vector<glm::vec3>& temp_normals,
	std::vector<unsigned int>& vertexIndices,
	std::vector<unsigned int>& normalIndices
){
	std::vector<unsigned int> uvIndices;

	std::vector<glm::vec2> temp_uvs;
	printf("Loading PLY file %s...\n", path);

	char stupidBuffer[1000];

	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	char lineHeader[128];
	int res = fscanf(file, "%s", lineHeader);

	printf("lineHeader: %s\n", lineHeader);

	if(strcmp(lineHeader, "ply") != 0){
		printf("Error in name format\n");
		fclose(file);
		return false;
	}

	char content[128];
	res = fscanf(file, "%s", lineHeader);
	res = fscanf(file, "%s", content);
	res = fscanf(file, "%s", stupidBuffer);

	printf("%s %s %s\n", lineHeader, content, stupidBuffer);

	if(strcmp(lineHeader, "format")==0){
		if(strcmp(content, "ascii")!=0){
			printf("Only ascii format is supported.\n");
			fclose(file);
			return false;	
		}

	}else{
		printf("Missing error\n");
		fclose(file);
		return false;
	}

	//Read all the comments
	do{
		res = fscanf(file, "%s", lineHeader);
		if(strcmp(lineHeader, "comment")==0)
			fgets(stupidBuffer, 1000, file);

	}while(strcmp(lineHeader, "comment")==0);
	
	//We should encounter the element vertex nro
	char number[128];
	res = fscanf(file, "%s", content);
	res = fscanf(file, "%s", number);

	printf("lineHeader: %s\n", lineHeader);

	int numVert = atoi(number);

	//Read all the vertex properties
	do{
		res = fscanf(file, "%s", lineHeader);
		if(strcmp(lineHeader, "property")==0)
			fgets(stupidBuffer, 1000, file);
	}while(strcmp(lineHeader, "property")==0);

	//We should encounter the element face nro
	res = fscanf(file, "%s", content);
	res = fscanf(file, "%s", number);
	printf("lineHeader: %s\n", lineHeader);

	int numTri = atoi(number);

	//Read all the faces properties
	do{
		res = fscanf(file, "%s", lineHeader);
			if(strcmp(lineHeader, "property")==0)
			fgets(stupidBuffer, 1000, file);
	}while(strcmp(lineHeader, "property")==0);

	if(strcmp(lineHeader, "end_header")!=0){
		printf("Error with end_header\n");
		fclose(file);
		return false;
	}

	printf("Vert: %d, Faces: %d\n", numVert, numTri);

	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	for(int i = 0; i < numVert; i++){
		glm::vec3 vertex;
		glm::vec3 normal;
		fscanf(file, "%f %f %f %f %f %f\n", &vertex.x, &vertex.y, &vertex.z, &normal.x, &normal.y,&normal.z);

		glm::vec4 vert = glm::vec4(vertex, 1.0f);
		//vert = rot * vert;

		temp_vertices.push_back(glm::vec3(vert.x, vert.y, vert.z));
		temp_normals.push_back(normal);
	}

	printf("Hito 1\n");
	unsigned int vertexIndex[3], uvIndex[3];

	for(int i = 0; i < numTri; i++){
		int numIndex;
		int numCoord;

		fscanf(file, "%d", &numIndex);
		if(numIndex!=3){
			printf("Only triangles are supported\n");
			fclose(file);
			return false;
		}

		fscanf(file, "%d %d %d", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
		glm::vec2 temp1, temp2, temp3;
		fscanf(file, "%d %f %f %f %f %f %f", &numCoord, &temp1.x, &temp1.y, &temp2.x, &temp2.y, &temp3.x, &temp3.y);
		temp1.y = -temp1.y;
		temp2.y = -temp2.y;
		temp3.y = -temp3.y;
		temp_uvs.push_back(temp1);
		temp_uvs.push_back(temp2);
		temp_uvs.push_back(temp3);

		vertexIndices.push_back(vertexIndex[0]);
		vertexIndices.push_back(vertexIndex[1]);
		vertexIndices.push_back(vertexIndex[2]);
		uvIndices.push_back(3*i);
		uvIndices.push_back(3*i+1);
		uvIndices.push_back(3*i+2);
		normalIndices.push_back(vertexIndex[0]);
		normalIndices.push_back(vertexIndex[1]);
		normalIndices.push_back(vertexIndex[2]);
	}
	
	printf("Hito 2\n");
	
	// Recorre cada indice de cada triangulo
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];
		
		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex ];
		glm::vec2 uv = temp_uvs[ uvIndex ];
		glm::vec3 normal = temp_normals[ normalIndex ];
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals .push_back(normal);
	}

	fclose(file);
	return true;
}

void saveOFF(const char* savePath, std::vector<glm::vec3>& vertices, 
									std::vector<unsigned int>& indices){ 
	
	std::ofstream out(savePath);

	out << "OFF" << std::endl;
	out << vertices.size() << " " << indices.size()/3 << " 0" << std::endl;

	for(int i = 0; i < vertices.size(); i++){
		out << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << std::endl;
	}

	for(int i = 0; i < indices.size()/3; i++){
		out << "3 " << indices[3*i] << " " << indices[3*i+1] << " " << indices[3*i+2] << std::endl;
	}

	out.close();
}

void saveOBJ(const char* sourcePath, const char* savePath,
	std::vector<glm::vec3>& temp_vertices,
	std::vector<glm::vec3>& temp_normals) {

	//Abrimos el archivo original 
	FILE* file = fopen(sourcePath, "r");
	std::ofstream out(savePath);

	if (file == NULL) {
		printf("No se puede leer el archivo\n");
		getchar();
		return;
	}

	while (1) {
		char lineHeader[128];

		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF) {
			break;
		}

		//Si llegamos a la parte de los vertices, salimos del bucle
		if (lineHeader[0]=='#' || lineHeader[0]=='m') {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);

			out << lineHeader << " " << stupidBuffer;
		}
		else
			break;
	}

	char stupidBuffer[1000];
	fgets(stupidBuffer, 1000, file);

	
	//Llenamos las normales y los vertices en el archivo de salida
	for (int i = 0; i < temp_vertices.size(); i++) {
		glm::vec3 vertex = temp_vertices[i];
		glm::vec3 normal = temp_normals[i];

		out << "vn " << normal.x << " " << normal.y << " " << normal.z << std::endl;
		out << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
	}

	//Copiamos el resto del archivo original al destino


	while (1) {
		char lineHeader[128];

		int res = fscanf(file, "%s", lineHeader);

		if (res == EOF) {
			break;
		}

		if (strcmp(lineHeader, "v") == 0 || strcmp(lineHeader, "vn") == 0) {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
			continue;
		}

		char stupidBuffer[1000];
		fgets(stupidBuffer, 1000, file);

		out << lineHeader << " " << stupidBuffer;
	}

	out.close();
	fclose(file);
	
}

#ifdef USE_ASSIMP // don't use this #define, it's only for me (it AssImp fails to compile on your machine, at least all the other tutorials still work)

// Include AssImp
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

bool loadAssImp(
	const char * path, 
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals
){

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, 0/*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/);
	if( !scene) {
		fprintf( stderr, importer.GetErrorString());
		getchar();
		return false;
	}
	const aiMesh* mesh = scene->mMeshes[0]; // In this simple example code we always use the 1rst mesh (in OBJ files there is often only one anyway)

	// Fill vertices positions
	vertices.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D pos = mesh->mVertices[i];
		vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
	}

	// Fill vertices texture coordinates
	uvs.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
		uvs.push_back(glm::vec2(UVW.x, UVW.y));
	}

	// Fill vertices normals
	normals.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D n = mesh->mNormals[i];
		normals.push_back(glm::vec3(n.x, n.y, n.z));
	}


	// Fill face indices
	indices.reserve(3*mesh->mNumFaces);
	for (unsigned int i=0; i<mesh->mNumFaces; i++){
		// Assume the model has only triangles.
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}
	
	// The "scene" pointer will be deleted automatically by "importer"
	return true;
}

#endif