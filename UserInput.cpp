#include "UserInput.h"
#include "Obstacle.h"
#include "Grid.h"
#include <GL/gl.h>
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Vector3 methods
void Vector3::normalize() {
    float length = std::sqrt(x * x + y * y + z * z);
    if (length > 0) {
        x /= length;
        y /= length;
        z /= length;
    }
}

Vector3 Vector3::cross(const Vector3& other) const {
    return Vector3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

Vector3 Vector3::copy() const {
    return Vector3(x, y, z);
}

// UserInput methods
UserInput::UserInput() {
    // Starting/spawn position
    spawnX = 0;
    spawnY = 100;
    spawnZ = 0;
    
    playerX = spawnX;
    playerY = spawnY;
    playerZ = spawnZ;
    
    pitch = 0.3f;  // Look down slightly at player
    yaw = 0;
    SPEED = 5;
    cameraDistance = 150;
    
    yVel = 0;
    gravity = -0.8f;
    jumpForce = 15;
    grounded = false;
    normalHeight = 100;
    crouchHeight = 50;
    playerHeight = normalHeight;
    isCrouching = false;
    collisionRadius = 20;
    deathY = -100;  // Fall below this and respawn
}

Vector3 UserInput::getViewVector() {
    Vector3 v;
    
    // Yaw: Rotation around Y-axis
    v.x = std::cos(yaw) * std::cos(pitch);
    v.y = std::sin(pitch);
    v.z = std::sin(yaw) * std::cos(pitch);
    
    return v;
}

void UserInput::rotate(float dx, float dy) {
    float sensitivity = 0.003f;
    
    yaw += dx * sensitivity;
    pitch += dy * sensitivity;
    
    // Constrain pitch to prevent flipping
    pitch = std::max(-static_cast<float>(M_PI)/2.0f + 0.01f, 
                     std::min(static_cast<float>(M_PI)/2.0f - 0.01f, pitch));
}

void UserInput::move(bool forward, bool backward, bool left, bool right, ObstacleCourse* course) {
    // Get view direction
    Vector3 v = getViewVector();
    
    // Forward vector (without Y → no climbing)
    Vector3 forwardV(v.x, 0, v.z);
    forwardV.normalize();
    
    // Right vector = cross product (v × Y-axis)
    Vector3 rightV = forwardV.cross(Vector3(0, 1, 0));
    rightV.normalize();
    
    float moveX = 0;
    float moveZ = 0;
    
    if (forward) {
        moveX += forwardV.x * SPEED;
        moveZ += forwardV.z * SPEED;
    }
    if (backward) {
        moveX -= forwardV.x * SPEED;
        moveZ -= forwardV.z * SPEED;
    }
    if (right) {
        moveX += rightV.x * SPEED;
        moveZ += rightV.z * SPEED;
    }
    if (left) {
        moveX -= rightV.x * SPEED;
        moveZ -= rightV.z * SPEED;
    }
    
    // Try X movement
    float newX = playerX + moveX;
    if (!course || !course->checkCollision(newX, playerY, playerZ, collisionRadius)) {
        playerX = newX;
    }
    
    // Try Z movement  
    float newZ = playerZ + moveZ;
    if (!course || !course->checkCollision(playerX, playerY, newZ, collisionRadius)) {
        playerZ = newZ;
    }
}

void UserInput::update(int windowWidth, int windowHeight, ObstacleCourse* course, Grid* grid) {
    // Smoothly adjust height when crouching
    float targetHeight = isCrouching ? crouchHeight : normalHeight;
    playerHeight += (targetHeight - playerHeight) * 0.2f;
    
    // Apply gravity
    yVel += gravity;
    
    // Try to move vertically
    float newY = playerY + yVel;
    
    // Check if we're standing on an obstacle
    float obstacleFloorY = 0;
    if (course) {
        obstacleFloorY = course->getFloorHeight(playerX, playerZ, playerY);
    }
    
    // Determine floor height
    float floorY = -1000.0f;  // Default: no floor (will fall forever)
    
    // Check if player is within grid bounds
    bool onGrid = grid && !grid->isOutOfBounds(playerX, playerZ);
    
    if (onGrid) {
        // On grid: use ground level (Y=0) or obstacle top, whichever is higher
        floorY = std::max(0.0f, obstacleFloorY);
    } else if (obstacleFloorY > 0) {
        // Off grid but on an obstacle platform
        floorY = obstacleFloorY;
    }
    // else: off grid and no obstacle = floorY stays at -1000 (no floor)
    
    // Check if hitting ceiling
    if (yVel > 0 && course && course->checkCollision(playerX, playerY + playerHeight + 5, playerZ, collisionRadius)) {
        yVel = 0;
        newY = playerY;
    }
    
    // Update Y position
    playerY = newY;
    
    // Ground/obstacle collision
    if (playerY <= floorY + playerHeight) {
        playerY = floorY + playerHeight;
        yVel = 0;
        grounded = true;
    } else {
        grounded = false;
    }
    
    // Check if fallen off the map
    bool offGrid = grid && grid->isOutOfBounds(playerX, playerZ);
    
    // Debug output
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {  // Print every 60 frames
        std::cout << "PlayerY: " << playerY << " FloorY: " << floorY 
                  << " OffGrid: " << offGrid << " Grounded: " << grounded << std::endl;
    }
    
    // Respawn if: fell below death zone OR (off grid AND below spawn height)
    if (playerY < deathY || (offGrid && playerY < spawnY - 10)) {
        std::cout << "RESPAWNING!" << std::endl;
        respawn();
    }
    
    // Set up 3rd person camera
    Vector3 viewDir = getViewVector();
    
    // Camera position behind and above player
    float cameraX = playerX - viewDir.x * cameraDistance;
    float cameraY = playerY + playerHeight * 0.5f - viewDir.y * cameraDistance;
    float cameraZ = playerZ - viewDir.z * cameraDistance;
    
    // Look at player center
    float lookAtX = playerX;
    float lookAtY = playerY + playerHeight * 0.5f;
    float lookAtZ = playerZ;
    
    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    float fov = 60.0f * M_PI / 180.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    
    float f = 1.0f / std::tan(fov / 2.0f);
    float rangeInv = 1.0f / (nearPlane - farPlane);
    
    float matrix[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (nearPlane + farPlane) * rangeInv, -1,
        0, 0, nearPlane * farPlane * rangeInv * 2.0f, 0
    };
    glMultMatrixf(matrix);
    
    // Set up modelview matrix (camera)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Look at (camera transformation)
    Vector3 forward(lookAtX - cameraX, lookAtY - cameraY, lookAtZ - cameraZ);
    forward.normalize();
    
    Vector3 up(0, 1, 0);
    Vector3 side = forward.cross(up);
    side.normalize();
    
    Vector3 upVec = side.cross(forward);
    
    float viewMatrix[16] = {
        side.x, upVec.x, -forward.x, 0,
        side.y, upVec.y, -forward.y, 0,
        side.z, upVec.z, -forward.z, 0,
        -side.x * cameraX - side.y * cameraY - side.z * cameraZ,
        -upVec.x * cameraX - upVec.y * cameraY - upVec.z * cameraZ,
        forward.x * cameraX + forward.y * cameraY + forward.z * cameraZ,
        1
    };
    glMultMatrixf(viewMatrix);
}

void UserInput::jump() {
    if (grounded && !isCrouching) {
        yVel = jumpForce;
        grounded = false;
    }
}

void UserInput::setCrouch(bool crouch) {
    isCrouching = crouch;
}

void UserInput::respawn() {
    playerX = spawnX;
    playerY = spawnY;
    playerZ = spawnZ;
    yVel = 0;
    grounded = false;
}

void UserInput::render() {
    drawStickFigure();
}

void UserInput::drawStickFigure() {
    glPushMatrix();
    
    // Position at player location - centered on ground
    glTranslatef(playerX, playerY - playerHeight, playerZ);
    
    // Billboard rotation - make stick figure face camera
    // Rotate around Y axis to face camera direction
    float billboardAngle = std::atan2(-getViewVector().x, -getViewVector().z) * 180.0f / M_PI;
    glRotatef(billboardAngle, 0, 1, 0);
    
    float bodyHeight = playerHeight * 0.6f;
    float headRadius = playerHeight * 0.1f;  // Smaller head
    float legLength = playerHeight * 0.4f;
    float armLength = playerHeight * 0.3f;   // Slimmer arms
    
    // Set color
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);  // Thinner lines
    
    glBegin(GL_LINES);
    
    // Body - starts from ground at legs
    glVertex3f(0, legLength, 0);
    glVertex3f(0, legLength + bodyHeight, 0);
    
    // Head (circle approximation)
    float headY = legLength + bodyHeight + headRadius;
    for (int i = 0; i < 16; i++) {
        float angle1 = (i * 2.0f * M_PI) / 16.0f;
        float angle2 = ((i + 1) * 2.0f * M_PI) / 16.0f;
        glVertex3f(std::cos(angle1) * headRadius, headY + std::sin(angle1) * headRadius, 0);
        glVertex3f(std::cos(angle2) * headRadius, headY + std::sin(angle2) * headRadius, 0);
    }
    
    // Arms
    float armY = legLength + bodyHeight * 0.8f;
    float armAngle = isCrouching ? 0.5f : 0.3f;
    glVertex3f(0, armY, 0);
    glVertex3f(-armLength * std::cos(armAngle), armY - armLength * std::sin(armAngle), 0);
    glVertex3f(0, armY, 0);
    glVertex3f(armLength * std::cos(armAngle), armY - armLength * std::sin(armAngle), 0);
    
    // Legs - go from leg junction to ground
    float legSpread = playerHeight * 0.08f;  // Much narrower stance
    glVertex3f(0, legLength, 0);
    glVertex3f(-legSpread, 0, 0);
    glVertex3f(0, legLength, 0);
    glVertex3f(legSpread, 0, 0);
    
    glEnd();
    
    glPopMatrix();
}
