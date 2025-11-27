#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <vector>

struct Box {
    float x, y, z;        // Position
    float width, height, depth;  // Dimensions
    float r, g, b;        // Color
    
    Box(float x, float y, float z, float w, float h, float d, float r = 0.8f, float g = 0.3f, float b = 0.3f)
        : x(x), y(y), z(z), width(w), height(h), depth(d), r(r), g(g), b(b) {}
    
    bool checkCollision(float px, float py, float pz, float radius) const;
};

class ObstacleCourse {
private:
    std::vector<Box> obstacles;
    
public:
    ObstacleCourse();
    void render();
    bool checkCollision(float x, float y, float z, float radius);
    float getFloorHeight(float x, float z, float currentY);
    void drawBox(const Box& box);
};

#endif // OBSTACLE_H
