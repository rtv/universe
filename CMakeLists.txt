cmake_minimum_required(VERSION 2.6)
project(universe)

set(universe_HEADERS src/universe.h)
set(universe_SOURCES src/universe.cc src/controller.cc)

list (APPEND REQ_LIBS "")

include (FindGLUT)
if (GLUT_FOUND)
  message (STATUS "Found GLUT in ${GLUT_INCLUDE_DIR}")
  message (STATUS "  GLUT_LIBRARIES    ${GLUT_LIBRARIES}")
  message (STATUS "  GLUT_glut_LIBRARY ${GLUT_glut_LIBRARY}")
  include_directories(${GLUT_INCLUDE_DIRS})
  list(APPEND REQ_LIBS ${GLUT_glut_LIBRARY})

  if (${GLUT_Xmu_LIBRARY} MATCHES NOTFOUND)
    message (STATUS "  GLUT_Xmu_LIBRARY  not found")
  else ($GLUT_Xmu_LIBRARY MATCHES NOTFOUND)
    message (STATUS "  GLUT_Xmu_LIBRARY  ${GLUT_Xmu_LIBRARY}")
    list(APPEND REQ_LIBS ${GLUT_Xmu_LIBRARY})
  endif (${GLUT_Xmu_LIBRARY} MATCHES NOTFOUND)
  if (${GLUT_Xi_LIBRARY} MATCHES NOTFOUND)
    message (STATUS "  GLUT_Xi_LIBRARY   not found")
  else (${GLUT_Xi_LIBRARY} MATCHES NOTFOUND)
    message (STATUS "  GLUT_Xi_LIBRARY   ${GLUT_Xi_LIBRARY}")
    list(APPEND REQ_LIBS ${GLUT_Xi_LIBRARY})
  endif (${GLUT_Xi_LIBRARY} MATCHES NOTFOUND)
else (GLUT_FOUND)
  message (FATAL_ERROR "GLUT not found")
endif (GLUT_FOUND)

find_package(OpenGL REQUIRED)
if (NOT ${OPENGL_GLU_FOUND} STREQUAL "YES")
	message( FATAL_ERROR "GLU not found" )
endif (NOT ${OPENGL_GLU_FOUND} STREQUAL "YES")

set(CMAKE_CXX_FLAGS "-g -Wall -O3")

message (STATUS "Found OpenGL in ${OPENGL_INCLUDE_DIR}")
message (STATUS "  OPENGL_LIBRARIES   ${OPENGL_LIBRARIES}")
message (STATUS "  OPENGL_gl_LIBRARY  ${OPENGL_gl_LIBRARY}")
message (STATUS "  OPENGL_glu_LIBRARY ${OPENGL_glu_LIBRARY}")
include_directories(${OpenGL_INCLUDE_DIRS})
list(APPEND REQ_LIBS ${OPENGL_LIBRARIES})

add_executable(universe 
    ${universe_HEADERS}
    ${universe_SOURCES}
)

link_directories(${GLUT_LIBRARY_DIRS} ${OpenGL_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS} ${OpenGL_DEFINITIONS})

target_link_libraries(universe  ${REQ_LIBS})
