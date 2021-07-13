set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(SRCS ${SRCS} ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_widgets.cpp 
        ${IMGUI_DIR}/imgui_impl_glfw.cpp ${IMGUI_DIR}/imgui_impl_opengl3.cpp ${IMGUI_DIR}/ImGuiFileBrowser.cpp ${IMGUI_DIR}/imgui_tables.cpp)

include_directories(${IMGUI_DIR})
