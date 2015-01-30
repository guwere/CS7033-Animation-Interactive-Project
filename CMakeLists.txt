cmake_minimum_required(VERSION 3.0.1)

#project basic setup stuff---------------------------
PROJECT (InteractiveProject)
SET (APP_NAME "InteractiveProject")
SET (APP_VERSION_MAJOR 1)
SET (APP_VERSION_MINOR 0)
SET (APP_VERSION_PATCH 0)
SET (APP_VERSION_STRING "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_PATCH}-${APP_VERSION_TYPE}")
SET (APP_AUTHORS "Antonio Nikolov")

#location of 1st/3rd party c++ libraries
SET(DEVLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cpplib")
#various app folders
SET(APP_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
SET(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test")
SET(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
SET(BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
SET(BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")

FILE(GLOB_RECURSE APP_SRC_FILES "${APP_SRC_DIR}/*.hpp"
							    "${APP_SRC_DIR}/*.h"
							    "${APP_SRC_DIR}/*.c"
							    "${APP_SRC_DIR}/*.cpp"
							    "${APP_SRC_DIR}/*.cxx")

#IDE setup stuff-------------------------------------
#Visual studio folder filter
SOURCE_GROUP(${APP_NAME} FILES ${APP_SRC_FILES})
#output directories
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}")
# SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
# SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")
# SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
# SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")


#program install-------------------------------------
add_executable(${APP_NAME}	${APP_SRC_FILES})
SET_PROPERTY( TARGET ${APP_NAME} PROPERTY DEBUG_POSTFIX "_debug" )

#1st party dependencies------------------------------
#luapath
add_subdirectory(${DEVLIB_DIR}/luapath "luapath")
include_directories(${LUAPATH_INCLUDE_DIR})
target_link_libraries(${APP_NAME} ${LUAPATH_LIBRARIES})
#logger
add_subdirectory(${DEVLIB_DIR}/logger/logger "logger")
include_directories(${LOGGER_INCLUDE_DIR})
target_link_libraries(${APP_NAME} ${LOGGER_LIBRARIES})

#3rd party dependencies------------------------------
#PLEASE INSTALL LIBRARIES BEFOREHAND as they would add substantial build time otherwise
#opengl
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} )
target_link_libraries(${APP_NAME} ${OPENGL_LIBRARIES} )

#glfw
include_directories("${DEVLIB_DIR}/glfw-3.0.4/include")
target_link_libraries(${APP_NAME} ${DEVLIB_DIR}/glfw-3.0.4/build/src/Release/glfw3dll.lib)

# ADD STRAIGHT TO PROJECT
# SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" )
# SET(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
# SET(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
# SET(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
# add_subdirectory("${DEVLIB_DIR}/glfw-3.0.4" "glfw")
# include_directories("${DEVLIB_DIR}/glfw-3.0.4/include")
# target_link_libraries(${APP_NAME} glfw ${GLFW_LIBRARIES})

#glm
include_directories( "${DEVLIB_DIR}/glm" )

#glew
include_directories( "${DEVLIB_DIR}/glew-1.11.0/include" ) 
target_link_libraries(${APP_NAME} ${DEVLIB_DIR}/glew-1.11.0/build/Release/libglew_shared.lib )
# ADD STRAIGHT TO PROJECT
# add_subdirectory("${DEVLIB_DIR}/glew-1.11.0" "glew")
# include_directories( "${DEVLIB_DIR}/glew-1.11.0/include" ) 
# target_link_libraries(${APP_NAME} libglew_shared )

#assimp
include_directories( "${DEVLIB_DIR}/assimp-3.1.1/include" )
target_link_libraries(${APP_NAME} ${DEVLIB_DIR}/assimp-3.1.1/build/code/Release/assimp.lib )

#SOIL
include_directories( "${DEVLIB_DIR}/SOIL/src" )
target_link_libraries(${APP_NAME} ${DEVLIB_DIR}/SOIL/projects/VC9/Release/SOIL.lib )




