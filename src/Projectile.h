#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <vector>

struct Arrow {
    float x, y, z;          // Position
    float speed;            // Movement speed
    float height;           // Height level (for collision)
    bool active;            // Is this arrow still flying?
    
    Arrow() : x(0), y(0), z(0), speed(0), height(30), active(false) {}
};

// Arrow launcher at specific locations
struct ArrowLauncher {
    float x, y, z;          // Position of launcher
    float targetHeight;     // Height at which arrows fire (low, mid, high)
    float fireInterval;     // Time between shots
    float timer;            // Current timer
    float arrowSpeed;       // Speed of arrows from this launcher
    
    ArrowLauncher(float px, float py, float pz, float height, float interval, float speed)
        : x(px), y(py), z(pz), targetHeight(height), fireInterval(interval), 
          timer(0), arrowSpeed(speed) {}
};

class ProjectileManager {
private:
    std::vector<Arrow> arrows;
    std::vector<ArrowLauncher> launchers;
    
    float gridHalfSize;         // Half the grid size
    
    // Arrow parameters
    float arrowLength;
    float arrowRadius;
    
    // Random number generation
    float randomFloat(float min, float max);
    
    void spawnArrowFromLauncher(ArrowLauncher& launcher);
    void drawArrow(const Arrow& arrow);
    void drawLauncher(const ArrowLauncher& launcher);

public:
    ProjectileManager(float gridSize);
    
    void update(float deltaTime);
    void render();
    
    // Check collision with player
    // Returns true if player is hit
    bool checkPlayerCollision(float playerX, float playerY, float playerZ, 
                              float playerRadius, float playerHeight, bool isCrouching);
    
    // Difficulty scaling
    void setDifficulty(float speedMultiplier, float spawnRateMultiplier);
    
    // Reset all projectiles
    void reset();
    
    // Get arrow count for debugging
    int getActiveArrowCount() const;
};

#endif // PROJECTILE_H
