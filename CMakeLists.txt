# CMake build file for 3dfsb
# Use:
# cmake . ; make
# ...to build 3dfsb.
# More ideas to improve this file: http://sourceforge.net/p/hugin/hugin/ci/default/tree/CMakeLists.txt
cmake_minimum_required(VERSION 2.8)

project(3dfsb C)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

#
# Check build type and set options accordingly
#
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# allow user to specify lib suffix
set(LIB_SUFFIX "" CACHE STRING "Define suffix of lib directory name (32/64)" )

IF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  MESSAGE("Building in Debug mode, resulting programs will be SLOW")
  ADD_DEFINITIONS(-DDEBUG)
  IF(UNIX)
    ADD_DEFINITIONS(-Wall)
  ENDIF(UNIX)
ENDIF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT)
find_package(GStreamer REQUIRED)
find_package(LibMagic REQUIRED)
find_package(ImageMagick COMPONENTS mogrify REQUIRED)
find_package(LibM REQUIRED)
find_package(GLIB COMPONENTS gobject REQUIRED)

include_directories(${SDL_INCLUDE_DIR}
                    ${GSTREAMER_INCLUDE_DIRS}
                   )

add_custom_command(OUTPUT images
                   COMMAND mogrify -format xpm *
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/images
                  )

add_library(str_replace str_replace.c)
add_library(media media.c)

add_executable(3dfsb 3dfsb.c)
add_dependencies(3dfsb images)
target_link_libraries(3dfsb str_replace media ${SDL_LIBRARY} ${SDL_IMAGE_LIBRARIES} ${OPENGL_LIBRARIES} ${GSTREAMER_LIBRARIES} ${GLUT_glut_LIBRARY} ${LibMagic_LIBRARY} ${LibM_LIBRARY} ${GLIB_GOBJECT_LIBRARIES} ${GLIB_LIBRARIES})
