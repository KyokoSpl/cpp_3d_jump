#include "UserInput.h"
#include "Obstacle.h"
#include "Grid.h"
#ifdef _WIN32
#include <windows.h>
#endif
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
    // Starting/spawn position - at the start of the parkour course
    spawnX = -350;
    spawnY = 100;
    spawnZ = -320;  // Parkour course Z position
    
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
    coyoteTimer = 0.0f;
    normalHeight = 100;
    crouchHeight = 50;
    playerHeight = normalHeight;
    isCrouching = false;
    collisionRadius = 20;
    deathY = -100;  // Fall below this and respawn
    devMode = false;  // Set via setDevMode()
    renderDistance = 3000.0f;  // Default render distance
    sensitivity = 0.003f;  // Default mouse sensitivity
    fov = 60.0f;  // Default field of view
    
    // Wall running
    isWallRunning = false;
    wallRunTimer = 0.0f;
    maxWallRunTime = 1.5f;  // Can wall run for 1.5 seconds
    wallRunSide = 0;
    wallRunKeyHeld = false;
    
    // Timer and stats
    timer = 0.0f;
    timerRunning = false;
    timerFinished = false;
    deathCount = 0;
    
    // Checkpoint system
    lastCheckpoint = -1;
    checkpointPopupTimer = 0.0f;
    checkpointMessage = "";
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
    yaw += dx * sensitivity;
    pitch -= dy * sensitivity;  // Inverted: pull down to look down, pull up to look up
    
    // Constrain pitch to prevent flipping
    pitch = std::max(-static_cast<float>(M_PI)/2.0f + 0.01f, 
                     std::min(static_cast<float>(M_PI)/2.0f - 0.01f, pitch));
}

void UserInput::move(bool forward, bool backward, bool left, bool right, ObstacleCourse* course, float deltaTime) {
    // Get view direction
    Vector3 v = getViewVector();
    
    // Forward vector (without Y → no climbing)
    Vector3 forwardV(v.x, 0, v.z);
    forwardV.normalize();
    
    // Right vector = cross product (v × Y-axis)
    Vector3 rightV = forwardV.cross(Vector3(0, 1, 0));
    rightV.normalize();
    
    // Normalize speed to 60 FPS base (deltaTime * 60 = 1.0 at 60 FPS)
    float frameSpeed = SPEED * deltaTime * 60.0f;
    
    float moveX = 0;
    float moveZ = 0;
    
    if (forward) {
        moveX += forwardV.x * frameSpeed;
        moveZ += forwardV.z * frameSpeed;
    }
    if (backward) {
        moveX -= forwardV.x * frameSpeed;
        moveZ -= forwardV.z * frameSpeed;
    }
    if (right) {
        moveX += rightV.x * frameSpeed;
        moveZ += rightV.z * frameSpeed;
    }
    if (left) {
        moveX -= rightV.x * frameSpeed;
        moveZ -= rightV.z * frameSpeed;
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

void UserInput::update(int windowWidth, int windowHeight, ObstacleCourse* course, Grid* grid, float deltaTime) {
    // Update timer if running
    if (timerRunning) {
        timer += deltaTime;
    }
    
    // Normalize to 60 FPS base
    float timeScale = deltaTime * 60.0f;
    
    // Smoothly adjust height when crouching
    float targetHeight = isCrouching ? crouchHeight : normalHeight;
    float lerpFactor = 1.0f - std::pow(1.0f - 0.2f, timeScale);  // Frame-rate independent lerp
    playerHeight += (targetHeight - playerHeight) * lerpFactor;
    
    // Wall running logic
    if (wallRunKeyHeld && !grounded && course) {
        // Check for walls on left and right
        Vector3 v = getViewVector();
        Vector3 forwardV(v.x, 0, v.z);
        forwardV.normalize();
        Vector3 rightV = forwardV.cross(Vector3(0, 1, 0));
        rightV.normalize();
        
        float wallCheckDist = collisionRadius + 15.0f;
        bool leftWall = course->checkCollision(playerX - rightV.x * wallCheckDist, playerY, 
                                               playerZ - rightV.z * wallCheckDist, 5.0f);
        bool rightWall = course->checkCollision(playerX + rightV.x * wallCheckDist, playerY,
                                                playerZ + rightV.z * wallCheckDist, 5.0f);
        
        if ((leftWall || rightWall) && wallRunTimer < maxWallRunTime && yVel <= 0) {
            if (!isWallRunning) {
                isWallRunning = true;
                wallRunSide = rightWall ? 1 : -1;
            }
            wallRunTimer += deltaTime;
            
            // Slow descent while wall running
            yVel = -2.0f;
            
            // Move forward along wall
            float wallRunSpeed = SPEED * 1.2f * timeScale;
            playerX += forwardV.x * wallRunSpeed;
            playerZ += forwardV.z * wallRunSpeed;
        } else {
            isWallRunning = false;
        }
    } else {
        isWallRunning = false;
    }
    
    // Reset wall run when grounded
    if (grounded) {
        wallRunTimer = 0.0f;
        isWallRunning = false;
    }
    
    // Apply gravity (scaled by delta time) - reduced during wall run
    if (isWallRunning) {
        yVel = std::max(yVel, -2.0f);  // Limit fall speed during wall run
    } else {
        yVel += gravity * timeScale;
    }
    
    // Try to move vertically (velocity also scaled by delta time)
    float newY = playerY + yVel * timeScale;
    
    // Check if we're standing on an obstacle
    float obstacleFloorY = 0;
    if (course) {
        obstacleFloorY = course->getFloorHeight(playerX, playerZ, playerY);
    }
    
    // Determine floor height
    float floorY = -1000.0f;  // Default: no floor (will fall forever)
    
    // In dev mode, add a floor at Y=0 everywhere so player doesn't fall forever
    if (devMode) {
        floorY = 0.0f;
    }
    
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
        if (!grounded) {
            land();  // Reset double jump when landing
        }
        coyoteTimer = 0.1f;  // Reset coyote time when on ground
    } else {
        // Coyote time: allow jumping briefly after leaving edge
        if (coyoteTimer > 0) {
            coyoteTimer -= deltaTime;
            grounded = true;  // Still considered grounded during coyote time
        } else {
            grounded = false;
        }
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
    // Skip respawn in dev mode
    if (!devMode && (playerY < deathY || (offGrid && playerY < spawnY - 10))) {
        std::cout << "RESPAWNING!" << std::endl;
        respawn(course);
    }
    
    // Check for death zone (spike plates)
    if (!devMode && course && course->isOnDeathZone(playerX, playerY, playerZ)) {
        std::cout << "HIT DEATH ZONE!" << std::endl;
        respawn(course);
    }
    
    // Check for checkpoint
    if (course) {
        int checkpoint = course->isOnCheckpoint(playerX, playerY, playerZ);
        if (checkpoint != -1 && checkpoint > lastCheckpoint) {
            lastCheckpoint = checkpoint;
            checkpointPopupTimer = 2.0f;  // Show popup for 2 seconds
            checkpointMessage = "Checkpoint " + std::to_string(checkpoint + 1) + " Reached!";
            std::cout << checkpointMessage << std::endl;
        }
    }
    
    // Update checkpoint popup timer
    if (checkpointPopupTimer > 0) {
        checkpointPopupTimer -= deltaTime;
    }
    
    // Set up camera - first person if zoomed in close, otherwise third person
    Vector3 viewDir = getViewVector();
    
    float cameraX, cameraY, cameraZ;
    float lookAtX, lookAtY, lookAtZ;
    
    // First person threshold - switch at distance < 20
    bool firstPerson = cameraDistance < 20.0f;
    
    if (firstPerson) {
        // First person: camera at player eye level
        cameraX = playerX;
        cameraY = playerY + playerHeight * 0.4f;  // Eye level (slightly below top of head)
        cameraZ = playerZ;
        
        // Look forward in view direction
        lookAtX = cameraX + viewDir.x * 100.0f;
        lookAtY = cameraY + viewDir.y * 100.0f;
        lookAtZ = cameraZ + viewDir.z * 100.0f;
    } else {
        // Third person: camera behind and above player
        cameraX = playerX - viewDir.x * cameraDistance;
        cameraY = playerY + playerHeight * 0.5f - viewDir.y * cameraDistance;
        cameraZ = playerZ - viewDir.z * cameraDistance;
        
        // Look at player center
        lookAtX = playerX;
        lookAtY = playerY + playerHeight * 0.5f;
        lookAtZ = playerZ;
    }
    
    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    float fovRad = fov * M_PI / 180.0f;
    float nearPlane = 0.1f;
    float farPlane = renderDistance;
    
    float f = 1.0f / std::tan(fovRad / 2.0f);
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
    // Wall jump - jump off wall while wall running
    if (isWallRunning) {
        // Jump away from wall
        Vector3 v = getViewVector();
        Vector3 forwardV(v.x, 0, v.z);
        forwardV.normalize();
        Vector3 rightV = forwardV.cross(Vector3(0, 1, 0));
        rightV.normalize();
        
        // Push away from wall and up
        yVel = jumpForce * 0.9f;
        isWallRunning = false;
        wallRunTimer = 0.0f;
        grounded = false;
        return;
    }
    
    // Normal jump
    if (grounded && !isCrouching) {
        yVel = jumpForce;
        grounded = false;
    // double jump
    } 
    if (!grounded && remainingJumps > 0) {
        yVel = jumpForce; // Slightly reduced height for double jump
        remainingJumps--;
    }
}
void UserInput::land() {
    remainingJumps = maxJumps;
    grounded = true;
}

void UserInput::crouchJump() {
    // Crouch jump - lower height but can be done while crouching
    if (grounded && isCrouching) {
        yVel = jumpForce * 0.6f;  // 60% of normal jump height
        grounded = false;
    }
}

void UserInput::setCrouch(bool crouch) {
    isCrouching = crouch;
}

void UserInput::setWallRunKey(bool held) {
    wallRunKeyHeld = held;
    if (!held) {
        isWallRunning = false;
        wallRunTimer = 0.0f;
    }
}

void UserInput::adjustCameraDistance(float delta) {
    cameraDistance -= delta * 10.0f;  // Scroll up = zoom in, scroll down = zoom out
    // Clamp camera distance - allow 0 for first person
    if (cameraDistance < 0.0f) cameraDistance = 0.0f;
    if (cameraDistance > 400.0f) cameraDistance = 400.0f;
}

void UserInput::setPhysics(float speed, float grav, float jump) {
    SPEED = speed;
    gravity = grav;
    jumpForce = jump;
}

void UserInput::resetPosition() {
    playerX = spawnX;
    playerY = spawnY;
    playerZ = spawnZ;
    yVel = 0;
    grounded = false;
}

void UserInput::respawn(ObstacleCourse* course) {
    // Respawn at last checkpoint if available
    if (lastCheckpoint >= 0 && course) {
        course->getCheckpointPosition(lastCheckpoint, playerX, playerY, playerZ);
    } else {
        playerX = spawnX;
        playerY = spawnY;
        playerZ = spawnZ;
    }
    yVel = 0;
    grounded = false;
    deathCount++;  // Increment death counter
}

void UserInput::toggleTimer() {
    if (!timerFinished) {  // Can only toggle if not finished
        timerRunning = !timerRunning;
    }
}

void UserInput::stopTimer() {
    timerRunning = false;
    timerFinished = true;
}

void UserInput::resetStats() {
    timer = 0.0f;
    timerRunning = false;
    timerFinished = false;
    deathCount = 0;
    lastCheckpoint = -1;
    checkpointPopupTimer = 0.0f;
    checkpointMessage = "";
}

void UserInput::render() {
    // Always draw shadow circle (visible in both first and third person)
    drawShadow();
    
    // Only draw stick figure in third person mode
    if (cameraDistance >= 20.0f) {
        drawStickFigure();
    }
}

// Helper function to draw a smooth 3D capsule/limb (cylinder with rounded ends)
static void drawLimb(float x1, float y1, float z1, float x2, float y2, float z2, float radius, int segments = 12) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    float length = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (length < 0.001f) return;
    
    // Normalize direction
    dx /= length; dy /= length; dz /= length;
    
    // Find perpendicular vectors
    float px, py, pz;
    if (std::abs(dy) < 0.9f) {
        px = -dz; py = 0; pz = dx;
    } else {
        px = 1; py = 0; pz = 0;
    }
    float pl = std::sqrt(px*px + py*py + pz*pz);
    px /= pl; py /= pl; pz /= pl;
    
    // Second perpendicular
    float qx = dy * pz - dz * py;
    float qy = dz * px - dx * pz;
    float qz = dx * py - dy * px;
    
    // Draw cylinder body
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = (i * 2.0f * M_PI) / segments;
        float c = std::cos(angle);
        float s = std::sin(angle);
        
        float nx = px * c + qx * s;
        float ny = py * c + qy * s;
        float nz = pz * c + qz * s;
        
        glNormal3f(nx, ny, nz);
        glVertex3f(x1 + nx * radius, y1 + ny * radius, z1 + nz * radius);
        glVertex3f(x2 + nx * radius, y2 + ny * radius, z2 + nz * radius);
    }
    glEnd();
    
    // Draw hemispherical caps for smooth ends
    int capSegs = 6;
    // Cap at start (x1, y1, z1)
    for (int i = 0; i < capSegs; i++) {
        float lat0 = M_PI * 0.5f * (float)i / capSegs;
        float lat1 = M_PI * 0.5f * (float)(i + 1) / capSegs;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; j++) {
            float lng = 2 * M_PI * (float)j / segments;
            float cx = std::cos(lng);
            float cz = std::sin(lng);
            
            for (int k = 0; k < 2; k++) {
                float lat = (k == 0) ? lat0 : lat1;
                float r = std::cos(lat) * radius;
                float offset = -std::sin(lat) * radius;
                
                float nx = px * cx * std::cos(lat) + qx * cz * std::cos(lat) - dx * std::sin(lat);
                float ny = py * cx * std::cos(lat) + qy * cz * std::cos(lat) - dy * std::sin(lat);
                float nz = pz * cx * std::cos(lat) + qz * cz * std::cos(lat) - dz * std::sin(lat);
                
                glNormal3f(nx, ny, nz);
                glVertex3f(x1 + (px * cx + qx * cz) * r + dx * offset,
                          y1 + (py * cx + qy * cz) * r + dy * offset,
                          z1 + (pz * cx + qz * cz) * r + dz * offset);
            }
        }
        glEnd();
    }
    
    // Cap at end (x2, y2, z2)
    for (int i = 0; i < capSegs; i++) {
        float lat0 = M_PI * 0.5f * (float)i / capSegs;
        float lat1 = M_PI * 0.5f * (float)(i + 1) / capSegs;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; j++) {
            float lng = 2 * M_PI * (float)j / segments;
            float cx = std::cos(lng);
            float cz = std::sin(lng);
            
            for (int k = 0; k < 2; k++) {
                float lat = (k == 0) ? lat0 : lat1;
                float r = std::cos(lat) * radius;
                float offset = std::sin(lat) * radius;
                
                float nx = px * cx * std::cos(lat) + qx * cz * std::cos(lat) + dx * std::sin(lat);
                float ny = py * cx * std::cos(lat) + qy * cz * std::cos(lat) + dy * std::sin(lat);
                float nz = pz * cx * std::cos(lat) + qz * cz * std::cos(lat) + dz * std::sin(lat);
                
                glNormal3f(nx, ny, nz);
                glVertex3f(x2 + (px * cx + qx * cz) * r + dx * offset,
                          y2 + (py * cx + qy * cz) * r + dy * offset,
                          z2 + (pz * cx + qz * cz) * r + dz * offset);
            }
        }
        glEnd();
    }
}

// Helper function to draw a smooth 3D sphere
static void drawSphere(float x, float y, float z, float radius, int segments = 12) {
    for (int i = 0; i < segments; i++) {
        float lat0 = M_PI * (-0.5f + (float)i / segments);
        float lat1 = M_PI * (-0.5f + (float)(i + 1) / segments);
        float y0 = std::sin(lat0);
        float y1 = std::sin(lat1);
        float r0 = std::cos(lat0);
        float r1 = std::cos(lat1);
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; j++) {
            float lng = 2 * M_PI * (float)j / segments;
            float cx = std::cos(lng);
            float cz = std::sin(lng);
            
            glNormal3f(cx * r0, y0, cz * r0);
            glVertex3f(x + radius * cx * r0, y + radius * y0, z + radius * cz * r0);
            glNormal3f(cx * r1, y1, cz * r1);
            glVertex3f(x + radius * cx * r1, y + radius * y1, z + radius * cz * r1);
        }
        glEnd();
    }
}

// Draw smooth tapered torso with rounded top
static void drawTorso(float x, float bottomY, float topY, float bottomRadius, float topRadius, int segments = 12) {
    float height = topY - bottomY;
    int rings = 8;
    
    for (int i = 0; i < rings; i++) {
        float t0 = (float)i / rings;
        float t1 = (float)(i + 1) / rings;
        float y0 = bottomY + height * t0;
        float y1 = bottomY + height * t1;
        float r0 = bottomRadius + (topRadius - bottomRadius) * t0;
        float r1 = bottomRadius + (topRadius - bottomRadius) * t1;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; j++) {
            float angle = 2 * M_PI * (float)j / segments;
            float cx = std::cos(angle);
            float cz = std::sin(angle);
            
            glNormal3f(cx, 0.1f, cz);
            glVertex3f(x + cx * r0, y0, cz * r0);
            glVertex3f(x + cx * r1, y1, cz * r1);
        }
        glEnd();
    }
    
    // Rounded dome cap on top
    int capSegs = 6;
    float capHeight = topRadius * 0.4f; // How much the dome rises
    for (int i = 0; i < capSegs; i++) {
        float lat0 = M_PI * 0.5f * (float)i / capSegs;
        float lat1 = M_PI * 0.5f * (float)(i + 1) / capSegs;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; j++) {
            float angle = 2 * M_PI * (float)j / segments;
            float cx = std::cos(angle);
            float cz = std::sin(angle);
            
            for (int k = 0; k < 2; k++) {
                float lat = (k == 0) ? lat0 : lat1;
                float r = std::cos(lat) * topRadius;
                float yOffset = std::sin(lat) * capHeight;
                
                glNormal3f(cx * std::cos(lat), std::sin(lat), cz * std::cos(lat));
                glVertex3f(x + cx * r, topY + yOffset, cz * r);
            }
        }
        glEnd();
    }
}

void UserInput::drawStickFigure() {
    glPushMatrix();
    
    // Position at player location
    glTranslatef(playerX, playerY - playerHeight, playerZ);
    
    // Rotate to face camera direction
    float faceAngle = std::atan2(-getViewVector().x, -getViewVector().z) * 180.0f / M_PI;
    glRotatef(faceAngle, 0, 1, 0);
    
    // Enable smooth shading and lighting
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Soft lighting
    float lightPos[] = {50.0f, 150.0f, 100.0f, 0.0f};
    float lightAmb[] = {0.4f, 0.4f, 0.4f, 1.0f};
    float lightDif[] = {0.6f, 0.6f, 0.6f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);
    
    // Body proportions
    float scale = playerHeight / 70.0f;
    float legLength = 32.0f * scale;
    float torsoLength = 28.0f * scale;
    float headRadius = 7.0f * scale;
    float torsoRadiusBottom = 7.0f * scale;
    float torsoRadiusTop = 8.0f * scale;
    float legRadius = 3.5f * scale;
    float armRadius = 2.8f * scale;
    float armLength = 26.0f * scale;
    float shoulderWidth = 9.0f * scale;
    float hipWidth = 4.0f * scale;
    
    // Single smooth color - warm gray
    glColor3f(0.75f, 0.72f, 0.70f);
    
    // ===== LEGS (smooth, continuous) =====
    float footY = 2.0f * scale;
    float kneeY = legLength * 0.45f;
    float hipY = legLength;
    
    // Left leg - thigh and calf as one smooth limb each
    drawLimb(-hipWidth, footY, 2.0f * scale, -hipWidth * 0.8f, kneeY, 0, legRadius);
    drawLimb(-hipWidth * 0.8f, kneeY, 0, -hipWidth * 0.5f, hipY, 0, legRadius * 1.1f);
    
    // Right leg
    drawLimb(hipWidth, footY, 2.0f * scale, hipWidth * 0.8f, kneeY, 0, legRadius);
    drawLimb(hipWidth * 0.8f, kneeY, 0, hipWidth * 0.5f, hipY, 0, legRadius * 1.1f);
    
    // Feet (rounded)
    drawSphere(-hipWidth, footY, 3.0f * scale, legRadius * 1.3f, 8);
    drawSphere(hipWidth, footY, 3.0f * scale, legRadius * 1.3f, 8);
    
    // ===== TORSO (smooth tapered) =====
    float torsoBottom = hipY - 2.0f * scale;
    float torsoTop = hipY + torsoLength;
    drawTorso(0, torsoBottom, torsoTop, torsoRadiusBottom, torsoRadiusTop);
    
    // Hip area - smooth sphere to blend legs into torso
    drawSphere(0, torsoBottom + 2.0f * scale, 0, torsoRadiusBottom * 1.1f, 10);
    
    // ===== ARMS (hanging naturally at sides) =====
    float shoulderY = torsoTop - 4.0f * scale;
    float elbowY = shoulderY - armLength * 0.5f;
    float handY = shoulderY - armLength * 0.95f;
    
    // Left arm - slight natural bend
    drawLimb(-shoulderWidth, shoulderY, 0, 
             -shoulderWidth - 2.0f * scale, elbowY, 3.0f * scale, armRadius);
    drawLimb(-shoulderWidth - 2.0f * scale, elbowY, 3.0f * scale,
             -shoulderWidth - 1.0f * scale, handY, 5.0f * scale, armRadius * 0.9f);
    
    // Right arm
    drawLimb(shoulderWidth, shoulderY, 0,
             shoulderWidth + 2.0f * scale, elbowY, 3.0f * scale, armRadius);
    drawLimb(shoulderWidth + 2.0f * scale, elbowY, 3.0f * scale,
             shoulderWidth + 1.0f * scale, handY, 5.0f * scale, armRadius * 0.9f);
    
    // Hands (smooth spheres)
    drawSphere(-shoulderWidth - 1.0f * scale, handY, 5.0f * scale, armRadius * 1.4f, 8);
    drawSphere(shoulderWidth + 1.0f * scale, handY, 5.0f * scale, armRadius * 1.4f, 8);
    
    // Shoulder spheres - larger and positioned to bridge arm and torso
    float shoulderSphereRadius = armRadius * 2.2f;
    drawSphere(-shoulderWidth + 2.0f * scale, shoulderY + 0.5f * scale, 0, shoulderSphereRadius, 10);
    drawSphere(shoulderWidth - 2.0f * scale, shoulderY + 0.5f * scale, 0, shoulderSphereRadius, 10);
    
    // ===== NECK & HEAD =====
    float neckY = torsoTop;
    float neckTopY = torsoTop + 5.0f * scale;
    drawLimb(0, neckY, 0, 0, neckTopY, 0, armRadius * 1.0f);
    
    // Head (smooth sphere)
    float headY = neckTopY + headRadius * 0.7f;
    drawSphere(0, headY, 0, headRadius, 16);
    
    // Disable lighting
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    
    glPopMatrix();
}

void UserInput::drawShadow() {
    glPushMatrix();
    
    // Position shadow at player's feet, raised above ground to avoid z-fighting
    glTranslatef(playerX, playerY - playerHeight + 2.0f, playerZ);
    
    // Rotate to lay flat on ground (rotate around X axis)
    glRotatef(90.0f, 1, 0, 0);
    
    // Disable depth test to ensure shadow always renders on top of ground
    glDisable(GL_DEPTH_TEST);
    
    // Enable blending for semi-transparent shadow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Outer shadow - collision radius (lighter)
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, 0);  // Center
    for (int i = 0; i <= 32; i++) {
        float angle = (i * 2.0f * M_PI) / 32.0f;
        glVertex3f(std::cos(angle) * collisionRadius, std::sin(angle) * collisionRadius, 0);
    }
    glEnd();
    
    // Inner circle - actual standing point (darker, smaller)
    float standingRadius = 5.0f;  // Small radius where player actually stands
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, 0.1f);  // Slightly above to render on top
    for (int i = 0; i <= 32; i++) {
        float angle = (i * 2.0f * M_PI) / 32.0f;
        glVertex3f(std::cos(angle) * standingRadius, std::sin(angle) * standingRadius, 0.1f);
    }
    glEnd();
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    glPopMatrix();
}
