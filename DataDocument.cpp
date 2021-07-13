#include "DataDocument.h"
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

DataDocument::DataDocument() {
   
   openedObject = false;
}

DataDocument::~DataDocument() {
    if (openedObject)
        delete modelCollision;  
}

void DataDocument::init() {
    selectionShape.loadFromOFFFile("sphere.off");
}

void DataDocument::releaseObject() {
    if (openedObject) {
        shape.releaseObject();
        delete modelCollision;
        openedObject = false;
        patterns.clear();
    }
}

void DataDocument::exportPatterns() {
    if (patterns.size() > 0) {
        //Create the directory for this object
        std::string dirName = "patterns/" + nameObject;

        fs::create_directory(dirName.c_str());

        fs::path file = pathObject + std::string("/") + nameObject + std::string(".ply");
        if(fs::exists(file)){
            fs::copy_file(file.c_str(), dirName + "/" + nameObject + ".ply");
        }

        //Try to copy the JPG file
        file = pathObject + std::string("/") + filenameTexturePLY;
        if (fs::exists(file)) {
            fs::copy_file(file.c_str(), dirName + "/" + filenameTexturePLY);
        }

        //Try to copy the PNG file
        int pos = filenameTexturePLY.find("jpg");

        std::string newFile = filenameTexturePLY.replace(pos, 3, "png");

        file = pathObject + std::string("/") + newFile;
        if (fs::exists(file)) {
            fs::copy_file(file.c_str(), dirName + "/" + newFile);
        }

        //Copy patterns
        for (int i = 0; i < patterns.size(); i++) {
            std::string patternName = dirName + "/" + nameObject + ".pat" + std::to_string(i) + ".png";
            fs::copy_file(patterns[i].filename,  patternName);
        }

    }
}

void DataDocument::loadShape(std::string filename) {

    nameObject = fs::path(filename).stem().string();
    pathObject = fs::path(filename).parent_path().string();
    std::string filenameMat = filename + ".mtl";

    std::string filenameTexture = getFilenameTexture(filenameMat);

    //filenameTexturePLY = getFilenameTexturePLY(filename);
    //std::cout << "Prueba:" << filenameTexturePLY << std::endl;
    
    if (filenameTexture.compare("Bad") == 0) {
        std::cout << "The material file does not contain a texture" << std::endl;
        return;
    }

    std::string texturePath = fs::path(filename).parent_path().append(filenameTexture).string();
    shape.loadTexture(texturePath.c_str());
    std::cout << "Texture loaded" << std::endl;
    shape.loadFromOBJFile(filename.c_str());
    //shape.loadFromPLYFile(filename.c_str());
    std::cout << "OBJ loaded" << std::endl;

    //Build collision model
    modelCollision = new COLDET::CollisionModel3DImpl(true);

    for (int i = 0; i < shape.vertices.size(); i = i + 3) {
        modelCollision->addTriangle(shape.vertices[i].x, shape.vertices[i].y, shape.vertices[i].z,
            shape.vertices[i + 1].x, shape.vertices[i + 1].y, shape.vertices[i + 1].z,
            shape.vertices[i + 2].x, shape.vertices[i + 2].y, shape.vertices[i + 2].z);
    }

    modelCollision->finalize();

    openedObject = true;
}

std::string trim(std::string input) {
    std::string ret = input;

    std::string::iterator p = ret.begin();
    while (p != ret.end() && *p == ' ') {
        p = ret.erase(p);
    }

    p = ret.end();
    while (p != ret.begin() && !isalnum(*(p - 1))) {
        p = ret.erase(p - 1);
    }

    return ret;
}

std::string getFilenameTexture(std::string filename) {
    FILE* file = fopen(filename.c_str(), "r");
    if (file == NULL) {
        printf("Material file does not exist\n");
        getchar();
        return std::string("Bad");
    }

    while (1) {

        char lineHeader[128];
        char stupidBuffer[1000];


        int res = fscanf(file, "%s", lineHeader);

        if (res == EOF)
            break;

        if (strcmp(lineHeader, "map_Kd") == 0) {
            fgets(stupidBuffer, 1000, file);
            std::string texFile = trim(std::string(stupidBuffer));
            if (texFile.find(".jpg") != std::string::npos) {
                fclose(file);
                return texFile;
            }
        }
        else {
            fgets(stupidBuffer, 1000, file);
        }
    }

    fclose(file);
    return std::string("Bad");
}

std::string getFilenameTexturePLY(std::string filename) {
    FILE* file = fopen(filename.c_str(), "r");
    if (file == NULL) {
        printf("Material file does not exist\n");
        getchar();
        return std::string("Bad");
    }

    printf("Punto 1\n");

    while (1) {

        char lineHeader[128];
        char stupidBuffer[1000];


        int res = fscanf(file, "%s", lineHeader);

        if (res == EOF)
            break;
        
        printf("Ojo -> %s\n", lineHeader);
        
        if (strcmp(lineHeader, "comment") == 0) {
            res = fscanf(file, "%s", stupidBuffer);
            std::string nameField = trim(std::string(stupidBuffer));
            if(nameField.find("TextureFile") != std::string::npos){
                res = fscanf(file, "%s", stupidBuffer);
                std::string texFile = trim(std::string(stupidBuffer));
                fclose(file);
                return texFile;
            }else{
                fgets(stupidBuffer, 1000, file);    
            }
        }
        else {
            fgets(stupidBuffer, 1000, file);
            
        }
    }

    fclose(file);
    return std::string("Bad");
}