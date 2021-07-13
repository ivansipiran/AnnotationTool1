#pragma once
#ifndef __DATA_DOCUMENT_H__

#include "Object.h"
#include "VirtualCanvas.h"
#include "Pattern.h"
#include <coldetimpl.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image_aug.h>
#include "Pattern.h"

std::string trim(std::string input);
std::string getFilenameTexture(std::string filename);
std::string getFilenameTexturePLY(std::string filename);

class DataDocument
{
public:
	std::string nameObject;
	std::string pathObject;
	std::string filenameTexturePLY;
	Object shape;
	Object selectionShape;
	COLDET::CollisionModel3DImpl* modelCollision;
	std::vector<Pattern> patterns;
	bool openedObject;

	DataDocument();
	~DataDocument();

	void loadShape(std::string filename);
	void init();
	void releaseObject();
	void exportPatterns();
};

#endif