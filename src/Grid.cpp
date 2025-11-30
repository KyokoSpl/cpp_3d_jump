#include "Grid.h"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/gl.h>

Grid::Grid(float aCellNum, float aCellSize) 
    : cellNum(aCellNum), cellSize(aCellSize) {
}

void Grid::update() {
    float size = cellNum * cellSize;
    float halfSize = size / 2.0f;

    // Draw grid lines
    glColor3f(0.78f, 0.78f, 0.78f);
    glBegin(GL_LINES);
    
    for (int i = 0; i <= cellNum; i++) {
        float x = -halfSize + i * cellSize;
        glVertex3f(x, 0.0f, -halfSize);
        glVertex3f(x, 0.0f, halfSize);

        float z = -halfSize + i * cellSize;
        glVertex3f(-halfSize, 0.0f, z);
        glVertex3f(halfSize, 0.0f, z);
    }
    
    glEnd();
    
    // Draw border walls to show the edge
    glColor3f(1.0f, 0.2f, 0.2f);
    glLineWidth(3.0f);
    float wallHeight = 5.0f;
    
    glBegin(GL_LINES);
    
    // North wall
    glVertex3f(-halfSize, 0.0f, -halfSize);
    glVertex3f(-halfSize, wallHeight, -halfSize);
    glVertex3f(halfSize, 0.0f, -halfSize);
    glVertex3f(halfSize, wallHeight, -halfSize);
    glVertex3f(-halfSize, wallHeight, -halfSize);
    glVertex3f(halfSize, wallHeight, -halfSize);
    
    // South wall
    glVertex3f(-halfSize, 0.0f, halfSize);
    glVertex3f(-halfSize, wallHeight, halfSize);
    glVertex3f(halfSize, 0.0f, halfSize);
    glVertex3f(halfSize, wallHeight, halfSize);
    glVertex3f(-halfSize, wallHeight, halfSize);
    glVertex3f(halfSize, wallHeight, halfSize);
    
    // West wall
    glVertex3f(-halfSize, 0.0f, -halfSize);
    glVertex3f(-halfSize, wallHeight, -halfSize);
    glVertex3f(-halfSize, 0.0f, halfSize);
    glVertex3f(-halfSize, wallHeight, halfSize);
    glVertex3f(-halfSize, wallHeight, -halfSize);
    glVertex3f(-halfSize, wallHeight, halfSize);
    
    // East wall
    glVertex3f(halfSize, 0.0f, -halfSize);
    glVertex3f(halfSize, wallHeight, -halfSize);
    glVertex3f(halfSize, 0.0f, halfSize);
    glVertex3f(halfSize, wallHeight, halfSize);
    glVertex3f(halfSize, wallHeight, -halfSize);
    glVertex3f(halfSize, wallHeight, halfSize);
    
    glEnd();
    glLineWidth(1.0f);
}

bool Grid::isOutOfBounds(float x, float z) const {
    float size = cellNum * cellSize;
    float halfSize = size / 2.0f;
    
    return (x < -halfSize || x > halfSize || z < -halfSize || z > halfSize);
}
