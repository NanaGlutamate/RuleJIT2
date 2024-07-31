cmake_minimum_required(VERSION 3.28)

set(PROJ_NAME Render)
project(${PROJ_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(AFTER ${PROJECT_SOURCE_DIR}/src)
# include_directories(AFTER ${PROJECT_SOURCE_DIR}/include/imgui)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_definitions(-D__PROJECT_ROOT_PATH="${PROJECT_SOURCE_DIR}")
add_definitions(-DEIGEN_MPL2_ONLY)
# add_definitions(-DIMGUI_IMPL_OPENGL_ES3)

if(MSVC)
  string(APPEND CMAKE_CXX_FLAGS " /permissive- /Zc:__cplusplus /utf-8 ")
endif()

# if(MSVC)
#   string(APPEND CMAKE_CXX_FLAGS " /MT ")
# else()
#   message(FATAL_ERROR "Only MSVC is supported")
# endif()

add_subdirectory(src)
add_subdirectory(test)