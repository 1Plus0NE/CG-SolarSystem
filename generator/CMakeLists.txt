cmake_minimum_required(VERSION 3.10)

# Project Name
project(generator)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 11)

add_executable(generator
    generator.cpp
    box.cpp
    plane.cpp
    sphere.cpp
    cone.cpp
    cylinder.cpp
    icosphere.cpp
    torus.cpp
)

# Math library (needed for sin, cos, etc. on Linux)
if (UNIX)
    target_link_libraries(${PROJECT_NAME} m)
endif()
