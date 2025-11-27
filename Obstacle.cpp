#include "Obstacle.h"
#include <GL/gl.h>
#include <cmath>

bool Box::checkCollision(float px, float py, float pz, float radius) const {
    // AABB collision - check if sphere overlaps box
    float halfW = width / 2;
    float halfD = depth / 2;
    
    // Find closest point on box to sphere center
    float closestX = std::max(x - halfW, std::min(px, x + halfW));
    float closestY = std::max(y, std::min(py, y + height));
    float closestZ = std::max(z - halfD, std::min(pz, z + halfD));
    
    // Calculate distance from closest point to sphere center
    float dx = px - closestX;
    float dy = py - closestY;
    float dz = pz - closestZ;
    
    float distSquared = dx * dx + dy * dy + dz * dz;
    return distSquared < (radius * radius);
}

ObstacleCourse::ObstacleCourse() {
    // Create a jump and run course with wider surfaces
    
    // Starting platform
    obstacles.push_back(Box(-200, -10, 0, 120, 10, 120, 0.5f, 0.5f, 0.5f));
    
    // Low barriers to jump over - wider tops
    obstacles.push_back(Box(-50, 0, 0, 30, 30, 60, 0.8f, 0.3f, 0.3f));
    obstacles.push_back(Box(50, 0, 0, 30, 35, 60, 0.8f, 0.3f, 0.3f));
    obstacles.push_back(Box(150, 0, 0, 30, 40, 60, 0.8f, 0.3f, 0.3f));
    
    // Walls to go around - wider passages
    obstacles.push_back(Box(250, 0, -60, 30, 60, 50, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(250, 0, 60, 30, 60, 50, 0.3f, 0.6f, 0.8f));
    
    // Low tunnel to crouch through - wider
    obstacles.push_back(Box(350, 60, 0, 80, 20, 100, 0.6f, 0.8f, 0.3f));
    
    // More jumps - bigger landing areas
    obstacles.push_back(Box(500, 0, 30, 35, 45, 60, 0.8f, 0.3f, 0.3f));
    obstacles.push_back(Box(600, 0, -30, 35, 50, 60, 0.8f, 0.3f, 0.3f));
    
    // Narrow passage - slightly wider
    obstacles.push_back(Box(700, 0, -80, 100, 80, 30, 0.7f, 0.4f, 0.9f));
    obstacles.push_back(Box(700, 0, 80, 100, 80, 30, 0.7f, 0.4f, 0.9f));
    
    // Final platform
    obstacles.push_back(Box(850, -10, 0, 120, 10, 120, 0.3f, 0.8f, 0.3f));
}

void ObstacleCourse::render() {
    for (const auto& box : obstacles) {
        drawBox(box);
    }
}

bool ObstacleCourse::checkCollision(float x, float y, float z, float radius) {
    for (const auto& box : obstacles) {
        if (box.checkCollision(x, y, z, radius)) {
            return true;
        }
    }
    return false;
}

float ObstacleCourse::getFloorHeight(float x, float z, float currentY) {
    float maxFloor = 0.0f;
    
    for (const auto& box : obstacles) {
        float halfW = box.width / 2;
        float halfD = box.depth / 2;
        
        // Check if player is above this box
        if (x >= box.x - halfW && x <= box.x + halfW &&
            z >= box.z - halfD && z <= box.z + halfD) {
            
            float topY = box.y + box.height;
            // Only consider if we're close to the top and it's higher than current max
            if (topY > maxFloor && currentY >= topY - 5) {
                maxFloor = topY;
            }
        }
    }
    
    return maxFloor;
}

void ObstacleCourse::drawBox(const Box& box) {
    glColor3f(box.r, box.g, box.b);
    
    float x1 = box.x - box.width / 2;
    float x2 = box.x + box.width / 2;
    float y1 = box.y;
    float y2 = box.y + box.height;
    float z1 = box.z - box.depth / 2;
    float z2 = box.z + box.depth / 2;
    
    glBegin(GL_QUADS);
    
    // Front face
    glVertex3f(x1, y1, z2);
    glVertex3f(x2, y1, z2);
    glVertex3f(x2, y2, z2);
    glVertex3f(x1, y2, z2);
    
    // Back face
    glVertex3f(x2, y1, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y2, z1);
    glVertex3f(x2, y2, z1);
    
    // Top face
    glVertex3f(x1, y2, z2);
    glVertex3f(x2, y2, z2);
    glVertex3f(x2, y2, z1);
    glVertex3f(x1, y2, z1);
    
    // Bottom face
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y1, z1);
    glVertex3f(x2, y1, z2);
    glVertex3f(x1, y1, z2);
    
    // Right face
    glVertex3f(x2, y1, z2);
    glVertex3f(x2, y1, z1);
    glVertex3f(x2, y2, z1);
    glVertex3f(x2, y2, z2);
    
    // Left face
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z2);
    glVertex3f(x1, y2, z2);
    glVertex3f(x1, y2, z1);
    
    glEnd();
}
