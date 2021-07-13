#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include "ImGuiFileBrowser.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <experimental/filesystem>
#include <malloc.h>


#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <common/shader.hpp>
#include "ArcBall.h"

#include "Object.h"
#include "VirtualCanvas.h"
#include "DataDocument.h"
#include "Pattern.h"
#include <coldetimpl.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image_aug.h>



namespace fs = std::experimental::filesystem;

void window_size_callback(GLFWwindow *window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int Window_width = 1920;
unsigned int Window_height = 1001;

unsigned int vista_width = 256;
unsigned int vista_height = 256;

glm::mat3 LastRot;
glm::mat3 ThisRot;
ArcBallT ArcBall;
bool isClicked;
glm::vec2 MousePt;
glm::mat4 Model;
glm::mat4 View;
glm::mat4 Projection;

bool vistaTaken = false;
bool show_open_dialog = false;
bool show_message_dialog = false;
bool modal_dialog = false;

std::vector<glm::vec3> vertSelection;
DataDocument dataApp;

GLubyte* pixels = NULL;

static imgui_addons::ImGuiFileBrowser file_dialog;

glm::vec3 unproject(glm::vec3 point, glm::mat4 modelview, glm::mat4 proj, int x, int y, int width, int height) {
    glm::mat4 inverse = glm::inverse(proj * modelview);

    glm::vec4 tmp(point, 1.0f);
    tmp.x = (tmp.x - float(x)) / float(width);
    tmp.y = (tmp.y - float(y)) / float(height);
    tmp = tmp * 2.0f - glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    glm::vec4 obj = inverse * tmp;
    if (abs(obj.w) <= 0.00001f) {
        obj.w = 1.0f;
    }

    obj = obj / obj.w;

    return glm::vec3(obj);
}

void OpenModalWindow(std::string message) {
    ImGui::PushID(2);
    ImGui::OpenPopup("Message");

    ImVec2 center(ImGui::GetIO().DisplaySize.x*0.5f, ImGui::GetIO().DisplaySize.y*0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Message", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(message.c_str());
        ImGui::Separator();
        if (ImGui::Button("OK"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::PopID();

}

int main(int argc, char* argv[])
{
    fs::create_directory("temp");
    
    isClicked = false;
    ThisRot = glm::mat3(1.0f);
    ArcBall.setBounds((GLfloat)Window_width, (GLfloat)Window_height);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow *window = glfwCreateWindow(Window_width, Window_height, "Peruvian Dataset Annotation Tool", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, window_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    glewInit();

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    const char* glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();

    
    bool ImGui = true;
    bool show_demo_window = true;
    
    VirtualCanvas canvas(256, 256);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");
    GLuint programSEL = LoadShaders("Selection.vertexshader","Selection.fragmentshader");
    
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
       
    Model = glm::mat4(1.0f);

    dataApp.init();

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();

        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open")) { show_open_dialog = true; }
                if (ImGui::MenuItem("Quit")) {}
                
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties");
        
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2,2));
        if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable)) {
            if (dataApp.openedObject) {
                ImGui::PushID(1);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                bool node_open = ImGui::TreeNode(dataApp.nameObject.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("");

                if (node_open) {
                    ImGui::PushID(0);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
                    ImGui::TreeNodeEx("Field", flags, "Num. Vertices:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    std::string value = std::to_string(dataApp.shape.temp_vertices.size());
                    ImGui::Text(value.c_str());
                    ImGui::NextColumn();
                    ImGui::PopID();

                    ImGui::PushID(1);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    ImGui::TreeNodeEx("Field", flags, "Num. Triangles:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    value = std::to_string(dataApp.shape.vertexIndices.size() / 3);
                    ImGui::Text(value.c_str());
                    ImGui::NextColumn();
                    ImGui::PopID();

                    ImGui::TreePop();
                }
                
                ImGui::PopID();
                
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::End();

        ImGui::Begin("Panel", &ImGui, ImGuiWindowFlags_MenuBar);
        
        if (ImGui::Button("Clean selection")) {
            vertSelection.clear();
        }

        if (ImGui::Button("Take pattern")) {
            if (dataApp.openedObject && vertSelection.size() > 4) {
                //Compute the surface centroid
                glm::vec3 centroid(0.0f);

                for (int i = 0; i < vertSelection.size(); i++) {
                    centroid = centroid + vertSelection[i];
                }

                glm::vec3 dir(-centroid.x, -centroid.y, -centroid.z);
                dir = glm::normalize(dir);

                float origin[3] = { centroid.x * 2.0, centroid.y * 2.0, centroid.z * 2.0 };
                float direction[3] = { dir.x, dir.y, dir.z };

                bool flag = dataApp.modelCollision->rayCollision(origin, direction, true, 0.0, 3.4e+38F);

                if (flag) {
                    float point[3];
                    dataApp.modelCollision->getCollisionPoint(point, true);
                    //vertSelection.push_back(glm::vec3(point[0], point[1], point[2]));
                    centroid.x = point[0];
                    centroid.y = point[1];
                    centroid.z = point[2];
                }

                //Calcular angulo de rotacion en eje Y
                float angle = 0.0;
                if (centroid.x < 0.0 && centroid.z > 0.0) {
                    angle = atan(fabs(centroid.x) / fabs(centroid.z));
                }

                if (centroid.x > 0.0 && centroid.z > 0.0) {
                    angle = -atan(fabs(centroid.x) / fabs(centroid.z));
                }

                if (centroid.x > 0.0 && centroid.z < 0.0) {
                    angle = -(3.14159265 / 2.0 + atan(fabs(centroid.z) / fabs(centroid.x)));
                }

                if (centroid.x < 0.0 && centroid.z < 0.0) {
                    angle = 3.14159265 / 2.0 + atan(fabs(centroid.z) / fabs(centroid.x));
                }

                glm::mat4 scal = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(-centroid.x, -centroid.y, -centroid.z));
                glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

                glm::mat4 modelVista = scal * rot * trans;

                //Find maximum radius
                float maxRadius = 0.0;
                for (int i = 0; i < vertSelection.size(); i++) {
                    glm::vec4 result = modelVista * glm::vec4(vertSelection[i], 1.0f);

                    if (fabs(result.x) > maxRadius) {
                        maxRadius = fabs(result.x);
                    }

                    if (fabs(result.y) > maxRadius) {
                        maxRadius = fabs(result.y);
                    }
                }

                canvas.render(dataApp.shape, modelVista, programID, maxRadius);

                Pattern auxPattern(canvas.getFilename());
                dataApp.patterns.push_back(auxPattern);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                vistaTaken = true;
                vertSelection.clear();
            
            }
        }

        if (ImGui::Button("Export patterns")) {
            dataApp.exportPatterns();
        }

        if (show_open_dialog) {
            ImGui::OpenPopup("Open File");
            show_open_dialog = false;
        }

        if (file_dialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(600, 300), ".obj,.OBJ")) {
            if (dataApp.openedObject)
                dataApp.releaseObject();
            dataApp.loadShape(file_dialog.selected_path);
        }

        ImGui::End();

        if (vistaTaken) {
            int mark = -1;
            ImGui::Begin("Patterns");
            for (int i = 0; i < dataApp.patterns.size(); i++) {
                ImGui::Image((void*)(intptr_t)dataApp.patterns[i].getTextureId(), ImVec2(dataApp.patterns[i].getPatternWidth(), dataApp.patterns[i].getPatternHeight()));
                ImGui::SameLine();
                ImGui::PushID(i);
                if (ImGui::Button("Delete")) { if(mark==-1) mark = i;}
                ImGui::PopID();
                
            }
            ImGui::End();

            if (mark != -1) {
                std::cout << "Erase:" << mark << std::endl;
                dataApp.patterns.erase(dataApp.patterns.begin() + mark);
            }

            if (dataApp.patterns.size() == 0)
                vistaTaken = false;
        }
        

        glViewport(0, 0, Window_width, Window_height);
            glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (dataApp.openedObject) {
            glm::mat4 scal = glm::scale(glm::mat4(1.0f), glm::vec3(dataApp.shape.scale, dataApp.shape.scale, dataApp.shape.scale));
            glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(-dataApp.shape.centerx, -dataApp.shape.centery, -dataApp.shape.centerz));

            Projection = glm::perspective(45.0f, GLfloat(Window_width) / Window_height, 0.1f, 100.0f);
            float distY = 2.0 / glm::tan(glm::radians(45.0f));
            float distX = 2.0 / (glm::tan(glm::radians(45.0f) * (GLfloat(Window_width) / Window_height)));

            if (distX > distY)
                distY = distX;
            glm::vec3 pos = glm::vec3(0.0f, 0.0f, distY);
            glm::vec3 lookAtPos = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);


            // Camera matrix
            View = glm::lookAt(pos, lookAtPos, up);
            glm::mat4 model = Model * scal * trans;
            glm::mat4 MVP = Projection * View * model;

            glUseProgram(programID);
            GLuint MatrixID = glGetUniformLocation(programID, "MVP");
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(MVP));

            dataApp.shape.drawObject(programID);

            glUseProgram(programSEL);
            MatrixID = glGetUniformLocation(programSEL, "MVP");
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(MVP));


            for (int i = 0; i < vertSelection.size(); i++) {
                glm::mat4 NewT = glm::translate(glm::mat4(1.0f), vertSelection[i]) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
                glm::mat4 NewMVP = MVP * NewT;
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(NewMVP));
                dataApp.selectionShape.drawObject(programSEL);
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        
    }
    
    glDeleteProgram(programID);
    glDeleteProgram(programSEL);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    
    fs::path tempDir("temp");
    fs::remove_all(tempDir);
    return 0;
}

void window_size_callback(GLFWwindow *window, int width, int height)
{
    std::cout << height << "---" << width << std::endl;
    Window_height = height;
    Window_width = width;
    glViewport(0, 0, width, height);
    ArcBall.setBounds((GLfloat)width, (GLfloat)height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;
    if (dataApp.openedObject) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            isClicked = true;
            LastRot = ThisRot;
            double xpos, ypos;
            Point2fT auxPoint;
            glfwGetCursorPos(window, &xpos, &ypos);
            auxPoint.s.X = (GLfloat)xpos;
            auxPoint.s.Y = (GLfloat)ypos;
            ArcBall.click(&auxPoint);
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            glm::mat4 scal = glm::scale(glm::mat4(1.0f), glm::vec3(dataApp.shape.scale, dataApp.shape.scale, dataApp.shape.scale));
            glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(-dataApp.shape.centerx, -dataApp.shape.centery, -dataApp.shape.centerz));

            glm::mat4 modelview = View * Model * scal * trans;
            glm::vec3 point1(xpos, Window_height - ypos, 0.0);
            glm::vec3 point2(xpos, Window_height - ypos, 1.0);

            glm::vec3 up1 = unproject(point1, modelview, Projection, 0, 0, Window_width, Window_height);
            glm::vec3 up2 = unproject(point2, modelview, Projection, 0, 0, Window_width, Window_height);

            glm::vec3 dir(up2.x - up1.x, up2.y - up1.y, up2.z - up1.z);
            dir = glm::normalize(dir);

            float origin[3] = { up1.x, up1.y, up1.z };
            float direction[3] = { dir.x, dir.y, dir.z };

            bool flag = dataApp.modelCollision->rayCollision(origin, direction, true, 0.0, 3.4e+38F);

            if (flag) {
                float point[3];
                dataApp.modelCollision->getCollisionPoint(point, true);
                vertSelection.push_back(glm::vec3(point[0], point[1], point[2]));
            }
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && isClicked) {
            isClicked = false;
            std::cout << "Release button" << std::endl;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    if (dataApp.openedObject) {
        if (yoffset > 0) {
            dataApp.shape.scale *= 1.05;
        }
        else {
            dataApp.shape.scale *= 0.95;
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    if (dataApp.openedObject) {
        if (isClicked) {
            Quat4fT ThisQuat;
            Point2fT auxPoint;
            auxPoint.s.X = (GLfloat)xpos;
            auxPoint.s.Y = (GLfloat)ypos;
            ArcBall.drag(&auxPoint, &ThisQuat);

            Matrix3fT mThisRot;
            memcpy(mThisRot.M, glm::value_ptr(ThisRot), sizeof(float) * 9);

            Matrix3fT mLastRot;
            memcpy(mLastRot.M, glm::value_ptr(LastRot), sizeof(float) * 9);
            Matrix4fT mTransform;
            memcpy(mTransform.M, glm::value_ptr(Model), sizeof(float) * 16);

            Matrix3fSetRotationFromQuat4f(&mThisRot, &ThisQuat);
            Matrix3fMulMatrix3f(&mThisRot, &mLastRot);
            Matrix4fSetRotationFromMatrix3f(&mTransform, &mThisRot);

            memcpy(glm::value_ptr(ThisRot), mThisRot.M, sizeof(float) * 9);
            memcpy(glm::value_ptr(LastRot), mLastRot.M, sizeof(float) * 9);
            memcpy(glm::value_ptr(Model), mTransform.M, sizeof(float) * 16);
            
        }
    }
}
