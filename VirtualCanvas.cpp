#include "VirtualCanvas.h"
#include <iostream>
#include <png.h>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

void VirtualCanvas::write_png(std::string filename, unsigned int width, unsigned int height, GLubyte* pixels) {
    FILE* fp = NULL;

    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;

    std::cout << "Filename: " + filename << std::endl;
    fp = fopen(filename.c_str(), "wb");
    if (fp == NULL) {
        printf("No se puede abrir archivo png %s\n", filename.c_str());
        exit(0);
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL) {
        printf("No se puede crear memoria para png\n");
        exit(0);
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        printf("No se puede crear memoria para info png\n");
        exit(0);
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("Error creando png\n");
        exit(0);
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    row = (png_bytep)malloc(3 * width * height * sizeof(png_byte));

    for (int y = 0; y < height; y++) {
        memcpy(row, pixels + (height - y - 1) * width * 3, sizeof(png_byte) * 3 * width);
        png_write_row(png_ptr, row);
    }

    png_write_end(png_ptr, NULL);

    fclose(fp);
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    free(row);
}

VirtualCanvas::VirtualCanvas(unsigned int width, unsigned int height) {
    canvasWidth = width;
    canvasHeight = height;

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &textureFB);

    glBindTexture(GL_TEXTURE_2D, textureFB);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, canvasWidth, canvasHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureFB, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, canvasWidth, canvasHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR Framebuffer" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VirtualCanvas::render(Object& shape, glm::mat4& matrix, GLuint& programID, float maxRadius) {

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, canvasWidth, canvasHeight);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programID);

    glm::mat4 projVista = glm::perspective(45.0f, GLfloat(canvasWidth) / canvasHeight, 0.1f, 100.0f);
    float distY = 1.5 * maxRadius / glm::tan(glm::radians(45.0f));
    float distX = 1.5 * maxRadius / (glm::tan(glm::radians(45.0f) * (GLfloat(canvasWidth) / canvasHeight)));

    if (distX > distY)
        distY = distX;

    glm::vec3 pos = glm::vec3(0.0f, 0.0f, distY);
    glm::vec3 lookAtPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Camera matrix
    glm::mat4 viewVista = glm::lookAt(pos, lookAtPos, up);
    glm::mat4 mvpVista = projVista * viewVista * matrix;

    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(mvpVista));

    shape.drawObject(programID);

    GLubyte* pixelsVista = (unsigned char*)malloc(sizeof(unsigned char) * 3 * canvasWidth * canvasHeight);
    glReadPixels(0, 0, canvasWidth, canvasHeight, GL_RGB, GL_UNSIGNED_BYTE, pixelsVista);
    
    std::string tempName = std::tmpnam(nullptr);
    outputFilename = std::string("temp/") + fs::path(tempName).filename().string() + ".png";
    std::cout << outputFilename << std::endl;
    write_png(outputFilename, canvasWidth, canvasHeight, pixelsVista);
    free(pixelsVista);
    pixelsVista = NULL;
}