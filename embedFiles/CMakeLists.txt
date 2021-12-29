cmake_minimum_required(VERSION 3.15)
set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
# project(embed C CXX)
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(cmake/StandardProjectSettings.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

add_library(embed_project_options INTERFACE)
add_library(embed_project_warnings INTERFACE)

target_compile_features(embed_project_options INTERFACE cxx_std_17)
set_project_warnings(embed_project_warnings)
enable_sanitizers(embed_project_options)

add_subdirectory(tools)
add_subdirectory(src)
