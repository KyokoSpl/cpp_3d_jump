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
    float coyoteTimer;              // Time after leaving edge where jump is still allowed
    float playerHeight;
    float crouchHeight;
    float normalHeight;
    bool isCrouching;
    float collisionRadius;
    float deathY;                   // Y level where player respawns
    bool devMode;                   // God mode - cannot die
    float renderDistance;           // Camera far plane distance
    float sensitivity;              // Mouse sensitivity
    float fov;                      // Field of view in degrees
    
    // Timer and stats
    float timer;                    // Current run time in seconds
    bool timerRunning;              // Is timer currently running
    bool timerFinished;             // Has the timer been stopped by reaching goal
    int deathCount;                 // Number of deaths this session

    Vector3 getViewVector();
    void drawStickFigure();
    void drawShadow();

public:
    UserInput();
    void rotate(float dx, float dy);
    void move(bool forward, bool backward, bool left, bool right, ObstacleCourse* course, float deltaTime);
    void update(int windowWidth, int windowHeight, ObstacleCourse* course, class Grid* grid, float deltaTime);
    void render();
    void jump();
    void setCrouch(bool crouch);
    void adjustCameraDistance(float delta);
    void setPhysics(float speed, float grav, float jump);
    void setDevMode(bool enabled) { devMode = enabled; }
    void setRenderDistance(float dist) { renderDistance = dist; }
    void setSensitivity(float sens) { sensitivity = sens; }
    void setFOV(float f) { fov = f; }
    void toggleTimer();             // Toggle timer on/off with T key
    void stopTimer();               // Stop timer (when reaching goal)
    void resetPosition();
    void resetStats();              // Reset timer and death count
    void respawn();                 // Respawn and increment death count
    float getPlayerX() const { return playerX; }
    float getPlayerY() const { return playerY; }
    float getPlayerZ() const { return playerZ; }
    float getPlayerHeight() const { return playerHeight; }
    float getCollisionRadius() const { return collisionRadius; }
    bool getIsCrouching() const { return isCrouching; }
    float getTimer() const { return timer; }
    bool isTimerRunning() const { return timerRunning; }
    bool isTimerFinished() const { return timerFinished; }
    int getDeathCount() const { return deathCount; }
};

#endif // USERINPUT_H
