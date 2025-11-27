cmake_minimum_required(VERSION 3.10)
project(ProcessingTo3D)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Add executable
add_executable(processing3d
    main.cpp
    Grid.cpp
    UserInput.cpp
    Obstacle.cpp
)

# Include directories
target_include_directories(processing3d PRIVATE
    ${OPENGL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(processing3d
    ${OPENGL_LIBRARIES}
    glfw
    m
)

# Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(processing3d PRIVATE -Wall -Wextra)
endif()
