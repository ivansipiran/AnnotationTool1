set(COMMON_DIR ${CMAKE_SOURCE_DIR}/common)
set (SRCS ${SRCS} ${COMMON_DIR}/objloader.cpp ${COMMON_DIR}/quaternion_utils.cpp ${COMMON_DIR}/shader.cpp 
            ${COMMON_DIR}/tangentspace.cpp ${COMMON_DIR}/text2D.cpp ${COMMON_DIR}/texture.cpp ${COMMON_DIR}/vboindexer.cpp)

include_directories(${COMMON_DIR})