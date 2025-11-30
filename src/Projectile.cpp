#include "Projectile.h"
#include <GL/gl.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ProjectileManager::ProjectileManager(float gridSize) {
    gridHalfSize = gridSize / 2.0f;
    
    // Initialize random seed
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(nullptr)));
        seeded = true;
    }
    
    // Arrow parameters
    arrowLength = 60.0f;
    arrowRadius = 8.0f;
    
    // Pre-allocate some arrows
    arrows.reserve(50);
    
    // Parkour course is at Z = -320
    float courseZ = -320.0f;
    float launcherZ = 100.0f;  // Launchers positioned in front (positive Z)
    
    // ============ SET UP ARROW LAUNCHERS AT SPECIFIC OBSTACLES ============
    
    // Section 2: Basic Jumps - launcher shooting low (need to jump)
    // At X = -150 (second jump barrier)
    launchers.push_back(ArrowLauncher(-150, 30, launcherZ, 30.0f, 2.0f, 500.0f));
    
    // Section 4: Zigzag Walls - launcher shooting mid height
    // At X = 260 (middle of zigzag)
    launchers.push_back(ArrowLauncher(260, 50, launcherZ, 50.0f, 1.8f, 550.0f));
    
    // Section 5: Platform Jumps - launcher shooting at jump height
    // At X = 560
    launchers.push_back(ArrowLauncher(560, 60, launcherZ, 60.0f, 1.5f, 600.0f));
    
    // Section 7: Narrow Corridor - launcher shooting head height (crouch!)
    // At X = 1050
    launchers.push_back(ArrowLauncher(1050, 90, launcherZ, 90.0f, 1.2f, 650.0f));
    
    // Section 8: Staircase - launcher at multiple heights
    // At X = 1300
    launchers.push_back(ArrowLauncher(1300, 40, launcherZ, 40.0f, 2.5f, 500.0f));  // Low
    launchers.push_back(ArrowLauncher(1300, 100, launcherZ, 100.0f, 2.5f, 500.0f)); // High
    
    // Section 9: High Platform Run - fast launcher
    // At X = 1480
    launchers.push_back(ArrowLauncher(1480, 95, launcherZ, 95.0f, 1.0f, 700.0f));
    
    // Section 10: After drop - low launcher
    // At X = 1750
    launchers.push_back(ArrowLauncher(1750, 35, launcherZ, 35.0f, 1.5f, 600.0f));
    
    // Section 11: Crouch/Jump Combo - alternating heights
    // At X = 2110
    launchers.push_back(ArrowLauncher(2110, 30, launcherZ, 30.0f, 2.0f, 550.0f));  // Low (jump)
    launchers.push_back(ArrowLauncher(2180, 90, launcherZ, 90.0f, 2.0f, 550.0f));  // High (crouch)
    
    // Section 12: Final Gauntlet - multiple fast launchers
    // At X = 2400-2500
    launchers.push_back(ArrowLauncher(2420, 40, launcherZ, 40.0f, 0.8f, 750.0f));
    launchers.push_back(ArrowLauncher(2500, 95, launcherZ, 95.0f, 0.8f, 750.0f));
}

float ProjectileManager::randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

void ProjectileManager::spawnArrowFromLauncher(ArrowLauncher& launcher) {
    Arrow arrow;
    
    arrow.x = launcher.x;
    arrow.z = launcher.z;
    arrow.y = launcher.targetHeight;
    arrow.height = launcher.targetHeight;
    arrow.speed = launcher.arrowSpeed;
    arrow.active = true;
    
    arrows.push_back(arrow);
}

void ProjectileManager::update(float deltaTime) {
    // Update each launcher's timer and spawn arrows
    for (auto& launcher : launchers) {
        launcher.timer += deltaTime;
        if (launcher.timer >= launcher.fireInterval) {
            spawnArrowFromLauncher(launcher);
            launcher.timer = 0.0f;
        }
    }
    
    // Update all arrows
    for (auto& arrow : arrows) {
        if (!arrow.active) continue;
        
        // Move arrow forward (negative Z direction towards parkour)
        arrow.z -= arrow.speed * deltaTime;
        
        // Deactivate if past the parkour area (Z < -400)
        if (arrow.z < -450) {
            arrow.active = false;
        }
    }
    
    // Remove inactive arrows periodically to prevent memory buildup
    static int cleanupCounter = 0;
    if (++cleanupCounter > 120) {  // Every ~2 seconds at 60fps
        arrows.erase(
            std::remove_if(arrows.begin(), arrows.end(), 
                [](const Arrow& a) { return !a.active; }),
            arrows.end()
        );
        cleanupCounter = 0;
    }
}

void ProjectileManager::drawLauncher(const ArrowLauncher& launcher) {
    glPushMatrix();
    glTranslatef(launcher.x, launcher.y, launcher.z);
    
    // Draw launcher body (box shape)
    float size = 25.0f;
    float depth = 40.0f;
    
    // Main body color - dark metallic
    glColor3f(0.3f, 0.3f, 0.35f);
    
    glBegin(GL_QUADS);
    // Front face
    glVertex3f(-size, -size, depth/2);
    glVertex3f(size, -size, depth/2);
    glVertex3f(size, size, depth/2);
    glVertex3f(-size, size, depth/2);
    
    // Back face
    glVertex3f(size, -size, -depth/2);
    glVertex3f(-size, -size, -depth/2);
    glVertex3f(-size, size, -depth/2);
    glVertex3f(size, size, -depth/2);
    
    // Top face
    glVertex3f(-size, size, depth/2);
    glVertex3f(size, size, depth/2);
    glVertex3f(size, size, -depth/2);
    glVertex3f(-size, size, -depth/2);
    
    // Bottom face
    glVertex3f(-size, -size, -depth/2);
    glVertex3f(size, -size, -depth/2);
    glVertex3f(size, -size, depth/2);
    glVertex3f(-size, -size, depth/2);
    
    // Right face
    glVertex3f(size, -size, depth/2);
    glVertex3f(size, -size, -depth/2);
    glVertex3f(size, size, -depth/2);
    glVertex3f(size, size, depth/2);
    
    // Left face
    glVertex3f(-size, -size, -depth/2);
    glVertex3f(-size, -size, depth/2);
    glVertex3f(-size, size, depth/2);
    glVertex3f(-size, size, -depth/2);
    glEnd();
    
    // Draw barrel (front opening) - red warning color
    glColor3f(0.8f, 0.2f, 0.2f);
    float barrelSize = size * 0.6f;
    
    glBegin(GL_QUADS);
    // Front barrel opening
    glVertex3f(-barrelSize, -barrelSize, depth/2 + 1);
    glVertex3f(barrelSize, -barrelSize, depth/2 + 1);
    glVertex3f(barrelSize, barrelSize, depth/2 + 1);
    glVertex3f(-barrelSize, barrelSize, depth/2 + 1);
    glEnd();
    
    // Draw warning stripes
    glColor3f(1.0f, 0.8f, 0.0f);  // Yellow warning
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    // Diagonal stripes on front
    for (int i = -2; i <= 2; i++) {
        float offset = i * 10.0f;
        glVertex3f(-size + offset, -size, depth/2 + 2);
        glVertex3f(size + offset, size, depth/2 + 2);
    }
    glEnd();
    
    // Draw aiming laser/indicator line towards parkour
    glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(0, 0, depth/2);
    glVertex3f(0, 0, -300);  // Line pointing towards parkour
    glEnd();
    
    glPopMatrix();
}

void ProjectileManager::drawArrow(const Arrow& arrow) {
    glPushMatrix();
    glTranslatef(arrow.x, arrow.y, arrow.z);
    
    // Rotate to point in -Z direction (towards player)
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    
    // Arrow color - red/orange
    glColor3f(1.0f, 0.3f, 0.1f);
    
    // Draw arrow shaft (cylinder approximation with lines)
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    
    // Main shaft
    glVertex3f(-arrowLength * 0.7f, 0, 0);
    glVertex3f(arrowLength * 0.3f, 0, 0);
    
    // Shaft thickness (cross pattern)
    float shaftRadius = arrowRadius * 0.3f;
    glVertex3f(-arrowLength * 0.7f, -shaftRadius, 0);
    glVertex3f(arrowLength * 0.1f, -shaftRadius, 0);
    glVertex3f(-arrowLength * 0.7f, shaftRadius, 0);
    glVertex3f(arrowLength * 0.1f, shaftRadius, 0);
    glVertex3f(-arrowLength * 0.7f, 0, -shaftRadius);
    glVertex3f(arrowLength * 0.1f, 0, -shaftRadius);
    glVertex3f(-arrowLength * 0.7f, 0, shaftRadius);
    glVertex3f(arrowLength * 0.1f, 0, shaftRadius);
    
    glEnd();
    
    // Draw arrowhead (pyramid/cone)
    glColor3f(0.8f, 0.8f, 0.8f);  // Silver tip
    glBegin(GL_TRIANGLES);
    
    float tipX = arrowLength * 0.3f;
    float baseX = arrowLength * 0.1f;
    float headSize = arrowRadius;
    
    // Top face
    glVertex3f(tipX, 0, 0);
    glVertex3f(baseX, headSize, 0);
    glVertex3f(baseX, 0, headSize);
    
    // Bottom face
    glVertex3f(tipX, 0, 0);
    glVertex3f(baseX, 0, headSize);
    glVertex3f(baseX, -headSize, 0);
    
    // Left face
    glVertex3f(tipX, 0, 0);
    glVertex3f(baseX, -headSize, 0);
    glVertex3f(baseX, 0, -headSize);
    
    // Right face
    glVertex3f(tipX, 0, 0);
    glVertex3f(baseX, 0, -headSize);
    glVertex3f(baseX, headSize, 0);
    
    glEnd();
    
    // Draw fletching (feathers at back)
    glColor3f(0.6f, 0.2f, 0.2f);  // Dark red feathers
    glBegin(GL_TRIANGLES);
    
    float backX = -arrowLength * 0.7f;
    float midX = -arrowLength * 0.5f;
    float featherSize = arrowRadius * 0.8f;
    
    // Top feather
    glVertex3f(backX, 0, 0);
    glVertex3f(midX, 0, 0);
    glVertex3f(midX, featherSize, 0);
    
    // Bottom feather
    glVertex3f(backX, 0, 0);
    glVertex3f(midX, 0, 0);
    glVertex3f(midX, -featherSize, 0);
    
    // Side feathers
    glVertex3f(backX, 0, 0);
    glVertex3f(midX, 0, 0);
    glVertex3f(midX, 0, featherSize);
    
    glVertex3f(backX, 0, 0);
    glVertex3f(midX, 0, 0);
    glVertex3f(midX, 0, -featherSize);
    
    glEnd();
    
    glPopMatrix();
}

void ProjectileManager::render() {
    // Draw all launchers first
    for (const auto& launcher : launchers) {
        drawLauncher(launcher);
    }
    
    // Draw all active arrows
    for (const auto& arrow : arrows) {
        if (arrow.active) {
            drawArrow(arrow);
        }
    }
}

bool ProjectileManager::checkPlayerCollision(float playerX, float playerY, float playerZ,
                                              float playerRadius, float playerHeight, bool isCrouching) {
    // Player hitbox: cylinder from (playerY - playerHeight) to playerY
    float playerBottom = playerY - playerHeight;
    float playerTop = playerY;
    
    // If crouching, reduce effective height
    if (isCrouching) {
        playerTop = playerBottom + playerHeight * 0.5f;
    }
    
    for (const auto& arrow : arrows) {
        if (!arrow.active) continue;
        
        // Check if arrow is within X range of player
        float dx = arrow.x - playerX;
        if (std::abs(dx) > playerRadius + arrowRadius) continue;
        
        // Check if arrow is within Z range of player (arrow is long)
        float arrowFront = arrow.z - arrowLength * 0.3f;
        float arrowBack = arrow.z + arrowLength * 0.7f;
        
        if (playerZ - playerRadius > arrowBack || playerZ + playerRadius < arrowFront) continue;
        
        // Check height collision
        float arrowBottom = arrow.y - arrowRadius;
        float arrowTop = arrow.y + arrowRadius;
        
        // Check if arrow height overlaps with player height
        if (arrowBottom > playerTop || arrowTop < playerBottom) continue;
        
        // Collision detected!
        return true;
    }
    
    return false;
}

void ProjectileManager::setDifficulty(float speedMultiplier, float spawnRateMultiplier) {
    // Adjust all launcher parameters based on base values
    // We need to store base values, so let's recalculate from default
    float baseIntervals[] = {2.0f, 1.8f, 1.5f, 1.2f, 2.5f, 2.5f, 1.0f, 1.5f, 2.0f, 2.0f, 0.8f, 0.8f};
    float baseSpeeds[] = {500.0f, 550.0f, 600.0f, 650.0f, 500.0f, 500.0f, 700.0f, 600.0f, 550.0f, 550.0f, 750.0f, 750.0f};
    
    for (size_t i = 0; i < launchers.size() && i < 12; i++) {
        // Scale fire interval (lower = faster)
        launchers[i].fireInterval = baseIntervals[i] / spawnRateMultiplier;
        if (launchers[i].fireInterval < 0.3f) launchers[i].fireInterval = 0.3f;
        
        // Scale arrow speed
        launchers[i].arrowSpeed = baseSpeeds[i] * speedMultiplier;
    }
}

void ProjectileManager::reset() {
    arrows.clear();
    
    // Reset all launcher timers
    for (auto& launcher : launchers) {
        launcher.timer = 0.0f;
    }
}

int ProjectileManager::getActiveArrowCount() const {
    int count = 0;
    for (const auto& arrow : arrows) {
        if (arrow.active) count++;
    }
    return count;
}
