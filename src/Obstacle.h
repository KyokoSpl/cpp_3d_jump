#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <vector>

enum class BoxType {
    NORMAL,
    CHECKPOINT,
    DEATH
};

struct Box {
    float x, y, z;        // Position
    float width, height, depth;  // Dimensions
    float r, g, b;        // Color
    BoxType type;         // Type of box
    
    Box() : x(0), y(0), z(0), width(0), height(0), depth(0), r(0), g(0), b(0), type(BoxType::NORMAL) {}
    
    Box(float x, float y, float z, float w, float h, float d, float r = 0.8f, float g = 0.3f, float b = 0.3f, BoxType t = BoxType::NORMAL)
        : x(x), y(y), z(z), width(w), height(h), depth(d), r(r), g(g), b(b), type(t) {}
    
    bool checkCollision(float px, float py, float pz, float radius) const;
};

class ObstacleCourse {
private:
    std::vector<Box> obstacles;
    std::vector<Box> checkpoints;
    std::vector<Box> deathZones;
    Box goalBox;  // The finish platform
    float glowPhase;  // For animated glow effect
    
public:
    ObstacleCourse();
    void render(float deltaTime);
    bool checkCollision(float x, float y, float z, float radius);
    float getFloorHeight(float x, float z, float currentY);
    bool isOnGoal(float x, float y, float z);  // Check if player reached the goal
    int isOnCheckpoint(float x, float y, float z);  // Returns checkpoint index or -1
    bool isOnDeathZone(float x, float y, float z);  // Check if player is on death plate
    void getCheckpointPosition(int index, float& outX, float& outY, float& outZ);
    void drawBox(const Box& box);
    void drawSpikes(const Box& box);
    void drawGlowingBox(const Box& box, float glow);
};

#endif // OBSTACLE_H
