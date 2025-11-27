#ifndef USERINPUT_H
#define USERINPUT_H

struct Vector3 {
    float x, y, z;
    
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    void normalize();
    Vector3 cross(const Vector3& other) const;
    Vector3 copy() const;
};

class ObstacleCourse;  // Forward declaration

class UserInput {
private:
    float playerX, playerY, playerZ;  // Player position
    float spawnX, spawnY, spawnZ;     // Respawn position
    float pitch;
    float yaw;
    float SPEED;                    // Movement per step
    float cameraDistance;           // Distance from player for 3rd person
    
    float yVel;
    float gravity;
    float jumpForce;
    bool grounded;
    float playerHeight;
    float crouchHeight;
    float normalHeight;
    bool isCrouching;
    float collisionRadius;
    float deathY;                   // Y level where player respawns

    Vector3 getViewVector();
    void drawStickFigure();
    void respawn();

public:
    UserInput();
    void rotate(float dx, float dy);
    void move(bool forward, bool backward, bool left, bool right, ObstacleCourse* course);
    void update(int windowWidth, int windowHeight, ObstacleCourse* course, class Grid* grid);
    void render();
    void jump();
    void setCrouch(bool crouch);
    float getPlayerX() const { return playerX; }
    float getPlayerY() const { return playerY; }
    float getPlayerZ() const { return playerZ; }
};

#endif // USERINPUT_H
