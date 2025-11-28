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
    // Extended parkour course along the EDGE of the grid (negative Z side)
    // Grid is 800x800 (-400 to +400), parkour runs along Z = -300 to -350
    
    float courseZ = -320;  // Center line of the parkour course
    
    // ==================== SECTION 1: Starting Area ====================
    // Starting platform at the edge
    obstacles.push_back(Box(-350, -10, courseZ, 100, 10, 80, 0.5f, 0.5f, 0.5f));
    
    // ==================== SECTION 2: Basic Jumps ====================
    // Low barriers to jump over - progressively higher
    obstacles.push_back(Box(-220, 0, courseZ, 30, 25, 60, 0.8f, 0.3f, 0.3f));
    obstacles.push_back(Box(-150, 0, courseZ, 30, 30, 60, 0.8f, 0.3f, 0.3f));
    obstacles.push_back(Box(-80, 0, courseZ, 30, 35, 60, 0.8f, 0.3f, 0.3f));
    obstacles.push_back(Box(-10, 0, courseZ, 30, 40, 60, 0.8f, 0.3f, 0.3f));
    
    // ==================== SECTION 3: Crouch Tunnel ====================
    // Low tunnel - must crouch to pass
    obstacles.push_back(Box(80, 55, courseZ, 100, 30, 80, 0.6f, 0.8f, 0.3f));
    
    // ==================== SECTION 4: Zigzag Walls ====================
    // Walls to weave through
    obstacles.push_back(Box(200, 0, courseZ - 35, 25, 70, 40, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(260, 0, courseZ + 35, 25, 70, 40, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(320, 0, courseZ - 35, 25, 70, 40, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(380, 0, courseZ + 35, 25, 70, 40, 0.3f, 0.6f, 0.8f));
    
    // ==================== SECTION 5: Platform Jumps ====================
    // Elevated platforms to jump across
    obstacles.push_back(Box(480, 0, courseZ - 30, 50, 40, 50, 0.9f, 0.5f, 0.2f));
    obstacles.push_back(Box(560, 0, courseZ + 30, 50, 50, 50, 0.9f, 0.5f, 0.2f));
    obstacles.push_back(Box(640, 0, courseZ - 30, 50, 60, 50, 0.9f, 0.5f, 0.2f));
    obstacles.push_back(Box(720, 0, courseZ, 50, 70, 50, 0.9f, 0.5f, 0.2f));
    
    // ==================== SECTION 6: Double Crouch ====================
    // Two crouch sections back to back
    obstacles.push_back(Box(820, 55, courseZ - 25, 80, 30, 50, 0.6f, 0.8f, 0.3f));
    obstacles.push_back(Box(920, 55, courseZ + 25, 80, 30, 50, 0.6f, 0.8f, 0.3f));
    
    // ==================== SECTION 7: Narrow Corridor ====================
    // Tight passage with walls on both sides
    obstacles.push_back(Box(1050, 0, courseZ - 50, 120, 90, 25, 0.7f, 0.4f, 0.9f));
    obstacles.push_back(Box(1050, 0, courseZ + 50, 120, 90, 25, 0.7f, 0.4f, 0.9f));
    
    // ==================== SECTION 8: Staircase Up ====================
    // Rising platforms like stairs
    obstacles.push_back(Box(1180, 0, courseZ, 40, 20, 60, 0.4f, 0.7f, 0.7f));
    obstacles.push_back(Box(1240, 0, courseZ, 40, 40, 60, 0.4f, 0.7f, 0.7f));
    obstacles.push_back(Box(1300, 0, courseZ, 40, 60, 60, 0.4f, 0.7f, 0.7f));
    obstacles.push_back(Box(1360, 0, courseZ, 40, 80, 60, 0.4f, 0.7f, 0.7f));
    
    // ==================== SECTION 9: High Platform Run ====================
    // Long elevated platform
    obstacles.push_back(Box(1480, 0, courseZ, 200, 80, 70, 0.5f, 0.3f, 0.7f));
    
    // ==================== SECTION 10: Jump Down + Obstacles ====================
    // Landing zone after drop
    obstacles.push_back(Box(1650, -10, courseZ, 80, 10, 80, 0.5f, 0.5f, 0.5f));
    
    // More jump obstacles
    obstacles.push_back(Box(1750, 0, courseZ, 30, 45, 60, 0.8f, 0.3f, 0.3f));
    obstacles.push_back(Box(1830, 0, courseZ, 30, 50, 60, 0.8f, 0.3f, 0.3f));
    obstacles.push_back(Box(1910, 0, courseZ, 30, 55, 60, 0.8f, 0.3f, 0.3f));
    
    // ==================== SECTION 11: Crouch + Jump Combo ====================
    // Alternating crouch and jump sections
    obstacles.push_back(Box(2020, 55, courseZ, 60, 30, 70, 0.6f, 0.8f, 0.3f));  // Crouch
    obstacles.push_back(Box(2110, 0, courseZ, 30, 40, 60, 0.8f, 0.3f, 0.3f));   // Jump
    obstacles.push_back(Box(2180, 55, courseZ, 60, 30, 70, 0.6f, 0.8f, 0.3f));  // Crouch
    obstacles.push_back(Box(2270, 0, courseZ, 30, 45, 60, 0.8f, 0.3f, 0.3f));   // Jump
    
    // ==================== SECTION 12: Final Gauntlet ====================
    // Tight zigzag with crouch ceiling
    obstacles.push_back(Box(2380, 0, courseZ - 40, 20, 80, 30, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(2380, 55, courseZ + 10, 60, 30, 60, 0.6f, 0.8f, 0.3f));
    obstacles.push_back(Box(2460, 0, courseZ + 40, 20, 80, 30, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(2460, 55, courseZ - 10, 60, 30, 60, 0.6f, 0.8f, 0.3f));
    
    // ==================== FINISH ====================
    // Victory platform
    goalBox = Box(2600, -10, courseZ, 150, 10, 100, 0.2f, 0.9f, 0.2f);
    obstacles.push_back(goalBox);
}

bool ObstacleCourse::isOnGoal(float x, float y, float z) {
    float halfW = goalBox.width / 2;
    float halfD = goalBox.depth / 2;
    float topY = goalBox.y + goalBox.height;
    
    // Check if player is standing on top of the goal platform
    return (x >= goalBox.x - halfW && x <= goalBox.x + halfW &&
            z >= goalBox.z - halfD && z <= goalBox.z + halfD &&
            y >= topY && y <= topY + 50);  // Within 50 units above the goal
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
    float standingMargin = 10.0f;  // Extra margin for standing on edges
    
    for (const auto& box : obstacles) {
        float halfW = box.width / 2 + standingMargin;
        float halfD = box.depth / 2 + standingMargin;
        
        // Check if player is within the XZ bounds of this box (with margin)
        if (x >= box.x - halfW && x <= box.x + halfW &&
            z >= box.z - halfD && z <= box.z + halfD) {
            
            float topY = box.y + box.height;
            
            // Only count this as floor if player is above it (not inside/below)
            if (currentY >= topY && topY > maxFloor) {
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
