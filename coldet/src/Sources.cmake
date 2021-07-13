set(COLDET_DIR ${CMAKE_SOURCE_DIR}/coldet)
set(SRCS ${SRCS} ${COLDET_DIR}/src/box.cpp ${COLDET_DIR}/src/box_bld.cpp ${COLDET_DIR}/src/cdmath3d.cpp ${COLDET_DIR}/src/coldet.cpp ${COLDET_DIR}/src/coldet_bld.cpp 
        ${COLDET_DIR}/src/multiobject.cpp ${COLDET_DIR}/src/mytritri.cpp ${COLDET_DIR}/src/sysdep.cpp ${COLDET_DIR}/src/tritri.c)

include_directories(${COLDET_DIR}/src/)