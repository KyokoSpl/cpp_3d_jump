#ifndef USERINPUT_H
#define USERINPUT_H

#include <string>

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
    
    // Wall running
    bool isWallRunning;
    float wallRunTimer;             // How long player has been wall running
    float maxWallRunTime;           // Maximum wall run duration
    int wallRunSide;                // -1 = left wall, 1 = right wall, 0 = none
    bool wallRunKeyHeld;            // Is E key being held
    
    // Timer and stats
    float timer;                    // Current run time in seconds
    bool timerRunning;              // Is timer currently running
    bool timerFinished;             // Has the timer been stopped by reaching goal
    int deathCount;                 // Number of deaths this session
    
    // Checkpoint system
    int lastCheckpoint;             // Last checkpoint reached (-1 = none)
    float checkpointPopupTimer;     // Timer for showing checkpoint popup
    std::string checkpointMessage;  // Message to show

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
    void crouchJump();              // Jump while crouching (lower but faster)
    void setCrouch(bool crouch);
    void setWallRunKey(bool held);  // Set wall run key state
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
    void respawn(ObstacleCourse* course = nullptr);  // Respawn and increment death count
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
    float getCheckpointPopupTimer() const { return checkpointPopupTimer; }
    const std::string& getCheckpointMessage() const { return checkpointMessage; }
    bool getIsWallRunning() const { return isWallRunning; }
};

#endif // USERINPUT_H
