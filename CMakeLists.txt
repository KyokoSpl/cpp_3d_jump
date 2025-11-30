cmake_minimum_required(VERSION 3.10)
project(ProcessingTo3D)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for clangd/IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

# Source files in src/ directory
set(SOURCES
    src/main.cpp
    src/Grid.cpp
    src/UserInput.cpp
    src/Obstacle.cpp
    src/Projectile.cpp
    src/menus/Menu.cpp
    src/menus/MenuAudio.cpp
    src/menus/MenuRender.cpp
    src/menus/MenuInput.cpp
    src/menus/Settings.cpp
    src/menus/Leaderboard.cpp
)

# Header files in src/ directory
set(HEADERS
    src/Grid.h
    src/UserInput.h
    src/Obstacle.h
    src/Projectile.h
    src/miniaudio.h
    src/menus/Menu.h
    src/menus/MenuAudio.h
    src/menus/Settings.h
    src/menus/Leaderboard.h
)

# Add executable
add_executable(processing3d ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(processing3d PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${OPENGL_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(processing3d
    ${OPENGL_LIBRARIES}
    glfw
    ${FREETYPE_LIBRARIES}
    m
    pthread
    dl
)

# Copy asset folder to build directory
file(COPY ${CMAKE_SOURCE_DIR}/asset DESTINATION ${CMAKE_BINARY_DIR})

# Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(processing3d PRIVATE -Wall -Wextra)
endif()
