cmake_minimum_required(VERSION 3.5)

# Project Name
PROJECT(generator)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_executable(${PROJECT_NAME} generator.cpp)

# Math library (needed for sin, cos, etc. on Linux)
if (UNIX)
    target_link_libraries(${PROJECT_NAME} m)
endif()
