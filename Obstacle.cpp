#include "Obstacle.h"
#include <GL/gl.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

ObstacleCourse::ObstacleCourse() : glowPhase(0.0f) {
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
    
    // ==================== DEATH ZONE 1 ====================
    deathZones.push_back(Box(40, -5, courseZ, 35, 5, 60, 0.4f, 0.4f, 0.4f, BoxType::DEATH));
    
    // ==================== SECTION 3: Crouch Tunnel ====================
    // Low tunnel - must crouch to pass
    obstacles.push_back(Box(80, 55, courseZ, 100, 30, 80, 0.6f, 0.8f, 0.3f));
    
    // ==================== SECTION 4: Zigzag Walls ====================
    // Walls to weave through
    obstacles.push_back(Box(200, 0, courseZ - 35, 25, 70, 40, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(260, 0, courseZ + 35, 25, 70, 40, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(320, 0, courseZ - 35, 25, 70, 40, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(380, 0, courseZ + 35, 25, 70, 40, 0.3f, 0.6f, 0.8f));
    
    // ==================== DEATH ZONE 2 ====================
    deathZones.push_back(Box(430, -5, courseZ, 35, 5, 60, 0.4f, 0.4f, 0.4f, BoxType::DEATH));
    
    // ==================== SECTION 5: Platform Jumps ====================
    // Elevated platforms to jump across
    obstacles.push_back(Box(480, 0, courseZ - 30, 50, 40, 50, 0.9f, 0.5f, 0.2f));
    obstacles.push_back(Box(560, 0, courseZ + 30, 50, 50, 50, 0.9f, 0.5f, 0.2f));
    obstacles.push_back(Box(640, 0, courseZ - 30, 50, 60, 50, 0.9f, 0.5f, 0.2f));
    obstacles.push_back(Box(720, 0, courseZ, 50, 70, 50, 0.9f, 0.5f, 0.2f));
    
    // ==================== CHECKPOINT 1 (after platform jumps) ====================
    checkpoints.push_back(Box(800, 0, courseZ, 50, 5, 50, 0.2f, 0.9f, 0.3f, BoxType::CHECKPOINT));
    
    // ==================== SECTION 6: Double Crouch ====================
    // Two crouch sections back to back
    obstacles.push_back(Box(870, 55, courseZ - 25, 80, 30, 50, 0.6f, 0.8f, 0.3f));
    obstacles.push_back(Box(970, 55, courseZ + 25, 80, 30, 50, 0.6f, 0.8f, 0.3f));
    
    // ==================== DEATH ZONE 3 ====================
    deathZones.push_back(Box(1030, -5, courseZ, 35, 5, 60, 0.4f, 0.4f, 0.4f, BoxType::DEATH));
    
    // ==================== SECTION 7: Narrow Corridor ====================
    // Tight passage with walls on both sides
    obstacles.push_back(Box(1100, 0, courseZ - 50, 120, 90, 25, 0.7f, 0.4f, 0.9f));
    obstacles.push_back(Box(1100, 0, courseZ + 50, 120, 90, 25, 0.7f, 0.4f, 0.9f));
    
    // ==================== SECTION 8: Staircase Up ====================
    // Rising platforms like stairs
    obstacles.push_back(Box(1230, 0, courseZ, 40, 20, 60, 0.4f, 0.7f, 0.7f));
    obstacles.push_back(Box(1290, 0, courseZ, 40, 40, 60, 0.4f, 0.7f, 0.7f));
    obstacles.push_back(Box(1350, 0, courseZ, 40, 60, 60, 0.4f, 0.7f, 0.7f));
    obstacles.push_back(Box(1410, 0, courseZ, 40, 80, 60, 0.4f, 0.7f, 0.7f));
    
    // ==================== SECTION 9: High Platform Run ====================
    // Long elevated platform
    obstacles.push_back(Box(1530, 0, courseZ, 200, 80, 70, 0.5f, 0.3f, 0.7f));
    
    // ==================== CHECKPOINT 2 (halfway point, on high platform) ====================
    checkpoints.push_back(Box(1600, 80, courseZ, 50, 5, 50, 0.2f, 0.9f, 0.3f, BoxType::CHECKPOINT));
    
    // ==================== SECTION 10: Jump Down + Obstacles ====================
    // Landing zone after drop
    obstacles.push_back(Box(1700, -10, courseZ, 80, 10, 80, 0.5f, 0.5f, 0.5f));
    
    // ==================== DEATH ZONE 4 ====================
    deathZones.push_back(Box(1760, -5, courseZ, 35, 5, 60, 0.4f, 0.4f, 0.4f, BoxType::DEATH));
    
    // More jump obstacles
    obstacles.push_back(Box(1800, 0, courseZ, 30, 45, 60, 0.8f, 0.3f, 0.3f));
    obstacles.push_back(Box(1880, 0, courseZ, 30, 50, 60, 0.8f, 0.3f, 0.3f));
    obstacles.push_back(Box(1960, 0, courseZ, 30, 55, 60, 0.8f, 0.3f, 0.3f));
    
    // ==================== SECTION 11: Crouch + Jump Combo ====================
    // Alternating crouch and jump sections
    obstacles.push_back(Box(2070, 55, courseZ, 60, 30, 70, 0.6f, 0.8f, 0.3f));  // Crouch
    obstacles.push_back(Box(2160, 0, courseZ, 30, 40, 60, 0.8f, 0.3f, 0.3f));   // Jump
    obstacles.push_back(Box(2230, 55, courseZ, 60, 30, 70, 0.6f, 0.8f, 0.3f));  // Crouch
    obstacles.push_back(Box(2320, 0, courseZ, 30, 45, 60, 0.8f, 0.3f, 0.3f));   // Jump
    
    // ==================== SECTION 12: Final Gauntlet ====================
    // Tight zigzag with crouch ceiling
    obstacles.push_back(Box(2430, 0, courseZ - 40, 20, 80, 30, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(2430, 55, courseZ + 10, 60, 30, 60, 0.6f, 0.8f, 0.3f));
    obstacles.push_back(Box(2510, 0, courseZ + 40, 20, 80, 30, 0.3f, 0.6f, 0.8f));
    obstacles.push_back(Box(2510, 55, courseZ - 10, 60, 30, 60, 0.6f, 0.8f, 0.3f));
    
    // ==================== SECTION 13: Wall Run Section ====================
    // Tall walls on the sides for wall running - gap too wide to jump normally
    obstacles.push_back(Box(2620, 0, courseZ - 50, 15, 150, 120, 0.2f, 0.5f, 0.9f));  // Left wall
    obstacles.push_back(Box(2620, 0, courseZ + 50, 15, 150, 120, 0.2f, 0.5f, 0.9f));  // Right wall
    // Landing platform after wall run
    obstacles.push_back(Box(2720, 0, courseZ, 60, 40, 60, 0.8f, 0.6f, 0.3f));
    
    // ==================== DEATH ZONE 5 (below wall run gap) ====================
    deathZones.push_back(Box(2670, -15, courseZ, 60, 10, 60, 0.4f, 0.4f, 0.4f, BoxType::DEATH));
    
    // ==================== SECTION 14: Crouch Jump Section ====================
    // Low ceiling + gap - requires crouch jump
    obstacles.push_back(Box(2800, 0, courseZ, 80, 30, 80, 0.5f, 0.5f, 0.5f));  // Platform
    obstacles.push_back(Box(2800, 75, courseZ, 100, 20, 100, 0.6f, 0.3f, 0.3f));  // Low ceiling
    obstacles.push_back(Box(2920, 0, courseZ, 60, 30, 60, 0.5f, 0.5f, 0.5f));  // Landing
    
    // ==================== SECTION 15: Extended Wall Run Gauntlet ====================
    // Multiple wall run sections in sequence
    obstacles.push_back(Box(3020, 0, courseZ - 55, 20, 180, 100, 0.3f, 0.4f, 0.8f));  // Wall 1
    obstacles.push_back(Box(3150, 0, courseZ + 55, 20, 180, 100, 0.3f, 0.4f, 0.8f));  // Wall 2
    obstacles.push_back(Box(3280, 0, courseZ - 55, 20, 180, 100, 0.3f, 0.4f, 0.8f));  // Wall 3
    obstacles.push_back(Box(3350, 0, courseZ, 60, 50, 60, 0.8f, 0.6f, 0.3f));  // Landing
    
    // Death zones under wall run gaps
    deathZones.push_back(Box(3085, -15, courseZ, 50, 10, 60, 0.4f, 0.4f, 0.4f, BoxType::DEATH));
    deathZones.push_back(Box(3215, -15, courseZ, 50, 10, 60, 0.4f, 0.4f, 0.4f, BoxType::DEATH));
    
    // ==================== SECTION 16: Crouch Tunnel Gauntlet ====================
    // Long crouch section with turns
    obstacles.push_back(Box(3450, 55, courseZ, 150, 30, 60, 0.6f, 0.8f, 0.3f));  // Low ceiling straight
    obstacles.push_back(Box(3620, 55, courseZ - 30, 100, 30, 60, 0.6f, 0.8f, 0.3f));  // Turn left
    obstacles.push_back(Box(3720, 55, courseZ - 60, 80, 30, 60, 0.6f, 0.8f, 0.3f));  // Continue
    
    // ==================== SECTION 17: Mixed Challenge ====================
    // Combines crouch, jump, and walls
    obstacles.push_back(Box(3850, 0, courseZ - 40, 30, 50, 60, 0.8f, 0.3f, 0.3f));  // Jump barrier
    obstacles.push_back(Box(3930, 55, courseZ, 60, 30, 80, 0.6f, 0.8f, 0.3f));  // Crouch
    obstacles.push_back(Box(4010, 0, courseZ + 40, 30, 55, 60, 0.8f, 0.3f, 0.3f));  // Jump barrier
    obstacles.push_back(Box(4100, 0, courseZ - 60, 15, 160, 100, 0.2f, 0.5f, 0.9f));  // Wall run wall
    obstacles.push_back(Box(4100, 0, courseZ + 60, 15, 160, 100, 0.2f, 0.5f, 0.9f));  // Wall run wall
    obstacles.push_back(Box(4200, 0, courseZ, 60, 45, 60, 0.8f, 0.6f, 0.3f));  // Landing
    
    // Death zone
    deathZones.push_back(Box(4100, -15, courseZ, 60, 10, 80, 0.4f, 0.4f, 0.4f, BoxType::DEATH));
    
    // ==================== FINISH ====================
    // Victory platform
    goalBox = Box(4350, -10, courseZ, 150, 10, 100, 0.2f, 0.9f, 0.2f);
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

int ObstacleCourse::isOnCheckpoint(float x, float y, float z) {
    for (size_t i = 0; i < checkpoints.size(); i++) {
        const Box& cp = checkpoints[i];
        float halfW = cp.width / 2;
        float halfD = cp.depth / 2;
        float topY = cp.y + cp.height;
        
        // Player Y includes playerHeight (100), so check a larger range
        if (x >= cp.x - halfW && x <= cp.x + halfW &&
            z >= cp.z - halfD && z <= cp.z + halfD &&
            y >= topY && y <= topY + 150) {
            return (int)i;
        }
    }
    return -1;
}

bool ObstacleCourse::isOnDeathZone(float x, float y, float z) {
    for (const auto& dz : deathZones) {
        float halfW = dz.width / 2;
        float halfD = dz.depth / 2;
        float topY = dz.y + dz.height;
        
        // Player Y includes playerHeight (100), so check a larger range
        if (x >= dz.x - halfW && x <= dz.x + halfW &&
            z >= dz.z - halfD && z <= dz.z + halfD &&
            y >= topY && y <= topY + 150) {
            return true;
        }
    }
    return false;
}

void ObstacleCourse::getCheckpointPosition(int index, float& outX, float& outY, float& outZ) {
    if (index >= 0 && index < (int)checkpoints.size()) {
        outX = checkpoints[index].x;
        // Player Y is head position = floor + playerHeight (100)
        // Checkpoint surface is at checkpoint.y + checkpoint.height
        outY = checkpoints[index].y + checkpoints[index].height + 100;
        outZ = checkpoints[index].z;
    }
}

void ObstacleCourse::render(float deltaTime) {
    // Update glow animation
    glowPhase += deltaTime * 3.0f;
    if (glowPhase > 2.0f * M_PI) glowPhase -= 2.0f * M_PI;
    float glow = 0.5f + 0.5f * std::sin(glowPhase);
    
    // Render normal obstacles
    for (const auto& box : obstacles) {
        drawBox(box);
    }
    
    // Render checkpoints with glow effect
    for (const auto& cp : checkpoints) {
        drawGlowingBox(cp, glow);
    }
    
    // Render death zones with spikes
    for (const auto& dz : deathZones) {
        drawBox(dz);
        drawSpikes(dz);
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
    
    // Check normal obstacles
    for (const auto& box : obstacles) {
        float halfW = box.width / 2 + standingMargin;
        float halfD = box.depth / 2 + standingMargin;
        
        if (x >= box.x - halfW && x <= box.x + halfW &&
            z >= box.z - halfD && z <= box.z + halfD) {
            
            float topY = box.y + box.height;
            
            if (currentY >= topY && topY > maxFloor) {
                maxFloor = topY;
            }
        }
    }
    
    // Check checkpoints
    for (const auto& cp : checkpoints) {
        float halfW = cp.width / 2 + standingMargin;
        float halfD = cp.depth / 2 + standingMargin;
        
        if (x >= cp.x - halfW && x <= cp.x + halfW &&
            z >= cp.z - halfD && z <= cp.z + halfD) {
            
            float topY = cp.y + cp.height;
            
            if (currentY >= topY && topY > maxFloor) {
                maxFloor = topY;
            }
        }
    }
    
    // Check death zones (they also have floor)
    for (const auto& dz : deathZones) {
        float halfW = dz.width / 2 + standingMargin;
        float halfD = dz.depth / 2 + standingMargin;
        
        if (x >= dz.x - halfW && x <= dz.x + halfW &&
            z >= dz.z - halfD && z <= dz.z + halfD) {
            
            float topY = dz.y + dz.height;
            
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

void ObstacleCourse::drawSpikes(const Box& box) {
    float x1 = box.x - box.width / 2;
    float x2 = box.x + box.width / 2;
    float topY = box.y + box.height;
    float z1 = box.z - box.depth / 2;
    float z2 = box.z + box.depth / 2;
    
    // Spike parameters
    float spikeHeight = 12.0f;
    float spikeSpacing = 10.0f;
    
    // Dark metal color for spikes
    glColor3f(0.25f, 0.25f, 0.28f);
    
    // Draw spikes in a grid pattern
    for (float sx = x1 + spikeSpacing / 2; sx < x2; sx += spikeSpacing) {
        for (float sz = z1 + spikeSpacing / 2; sz < z2; sz += spikeSpacing) {
            float baseSize = 3.5f;
            
            glBegin(GL_TRIANGLES);
            
            // Front face
            glVertex3f(sx, topY + spikeHeight, sz);
            glVertex3f(sx - baseSize, topY, sz + baseSize);
            glVertex3f(sx + baseSize, topY, sz + baseSize);
            
            // Right face
            glVertex3f(sx, topY + spikeHeight, sz);
            glVertex3f(sx + baseSize, topY, sz + baseSize);
            glVertex3f(sx + baseSize, topY, sz - baseSize);
            
            // Back face
            glVertex3f(sx, topY + spikeHeight, sz);
            glVertex3f(sx + baseSize, topY, sz - baseSize);
            glVertex3f(sx - baseSize, topY, sz - baseSize);
            
            // Left face
            glVertex3f(sx, topY + spikeHeight, sz);
            glVertex3f(sx - baseSize, topY, sz - baseSize);
            glVertex3f(sx - baseSize, topY, sz + baseSize);
            
            glEnd();
        }
    }
}

void ObstacleCourse::drawGlowingBox(const Box& box, float glow) {
    // Main box with pulsing brightness
    float brightness = 0.6f + 0.4f * glow;
    glColor3f(box.r * brightness, box.g * brightness, box.b * brightness);
    
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
    
    // Top face (brighter)
    glColor3f(box.r * brightness * 1.2f, box.g * brightness * 1.2f, box.b * brightness * 1.2f);
    glVertex3f(x1, y2, z2);
    glVertex3f(x2, y2, z2);
    glVertex3f(x2, y2, z1);
    glVertex3f(x1, y2, z1);
    
    // Bottom face
    glColor3f(box.r * brightness, box.g * brightness, box.b * brightness);
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
    
    // Draw glow border/ring on top
    glColor3f(0.3f + 0.7f * glow, 1.0f, 0.4f + 0.3f * glow);
    float borderY = y2 + 0.5f;
    float borderInset = 2.0f;
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(x1 + borderInset, borderY, z1 + borderInset);
    glVertex3f(x2 - borderInset, borderY, z1 + borderInset);
    glVertex3f(x2 - borderInset, borderY, z2 - borderInset);
    glVertex3f(x1 + borderInset, borderY, z2 - borderInset);
    glEnd();
    glLineWidth(1.0f);
}
