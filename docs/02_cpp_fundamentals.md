# 2. C++ Fundamentals Used

This document explains the C++ features and patterns used throughout the project.

## Classes and Objects

### What is a Class?

A **class** is a blueprint for creating objects. It bundles data (variables) and functions (methods) together.

```cpp
// From UserInput.h
class UserInput {
private:
    float playerX, playerY, playerZ;  // Data (member variables)
    float yVel;                        // Vertical velocity
    
public:
    void jump();                       // Methods (member functions)
    void move(bool forward, bool backward, bool left, bool right, ...);
};
```

### Private vs Public

- **private**: Only accessible inside the class
- **public**: Accessible from anywhere

```cpp
class UserInput {
private:
    float playerX;  // Only UserInput methods can access this
    
public:
    float getPlayerX() const { return playerX; }  // Anyone can call this
};

// Usage:
UserInput player;
// player.playerX = 5;        // ERROR! playerX is private
float x = player.getPlayerX(); // OK! getPlayerX is public
```

This is called **encapsulation** - hiding internal details and providing a clean interface.

### Constructor

A **constructor** is a special method that runs when you create an object:

```cpp
// From UserInput.cpp
UserInput::UserInput() {
    // This runs when you write: UserInput player;
    playerX = -350;
    playerY = 100;
    playerZ = -320;
    yVel = 0;
    gravity = -0.8f;
    // ... more initialization
}
```

### The `const` Keyword

`const` means "this won't change":

```cpp
// Method won't modify the object
float getPlayerX() const { return playerX; }

// Parameter won't be modified
void render(const Box& box);  // box is read-only

// Variable is constant
const float PI = 3.14159f;
```

## Structs

A **struct** is like a simple class where everything is public by default:

```cpp
// From Obstacle.h
struct Box {
    float x, y, z;           // Position (center)
    float width, height, depth;
    float r, g, b;           // Color (red, green, blue)
    BoxType type;
    
    // Constructor with default parameters
    Box(float x, float y, float z, float w, float h, float d,
        float r = 0.5f, float g = 0.5f, float b = 0.5f,
        BoxType type = BoxType::NORMAL);
};
```

### Default Parameters

```cpp
Box(float x, float y, float z, float w, float h, float d,
    float r = 0.5f, float g = 0.5f, float b = 0.5f);

// You can create boxes with or without specifying color:
Box box1(0, 0, 0, 10, 10, 10);           // Uses default gray (0.5, 0.5, 0.5)
Box box2(0, 0, 0, 10, 10, 10, 1, 0, 0);  // Red (1, 0, 0)
```

## Enums (Enumerations)

Enums create named constants:

```cpp
// From Obstacle.h
enum class BoxType {
    NORMAL,
    CHECKPOINT,
    DEATH
};

// Usage:
BoxType type = BoxType::CHECKPOINT;
if (type == BoxType::DEATH) {
    // Player hit a death zone
}
```

The `enum class` (scoped enum) is safer than plain `enum` because it requires the type name prefix.

## Vectors (Dynamic Arrays)

`std::vector` is a resizable array:

```cpp
#include <vector>

// From Obstacle.h
std::vector<Box> obstacles;      // Dynamic list of Box objects
std::vector<Box> checkpoints;
std::vector<Box> deathZones;

// Adding elements
obstacles.push_back(Box(0, 0, 0, 100, 10, 80));

// Accessing elements
Box& first = obstacles[0];

// Iterating
for (const auto& box : obstacles) {
    drawBox(box);
}

// Size
size_t count = obstacles.size();
```

### Range-based For Loop

```cpp
// Old style:
for (size_t i = 0; i < obstacles.size(); i++) {
    drawBox(obstacles[i]);
}

// Modern style (cleaner):
for (const auto& box : obstacles) {
    drawBox(box);
}
```

- `const` - we're not modifying the box
- `auto` - compiler figures out the type (Box)
- `&` - reference, avoids copying

## Pointers

A **pointer** stores a memory address:

```cpp
// From main.cpp
UserInput* userInput = nullptr;  // Pointer, initially null

// Create object on heap (dynamic allocation)
userInput = new UserInput();

// Access members with ->
userInput->jump();
userInput->getPlayerX();

// Don't forget to delete!
delete userInput;
```

### Why Use Pointers?

1. **Lifetime control**: Object lives until you `delete` it
2. **Sharing**: Multiple parts of code can access the same object
3. **Polymorphism**: Using base class pointers for derived objects

### nullptr

Always initialize pointers to `nullptr` (null pointer):

```cpp
UserInput* player = nullptr;  // Safe default

if (player != nullptr) {
    player->jump();  // Only call if pointer is valid
}
```

## References

A **reference** is an alias for another variable:

```cpp
void getCheckpointPosition(int index, float& outX, float& outY, float& outZ) {
    // outX, outY, outZ are references - changes affect the original variables
    outX = checkpoints[index].x;
    outY = checkpoints[index].y + checkpoints[index].height + 100;
    outZ = checkpoints[index].z;
}

// Usage:
float x, y, z;
course->getCheckpointPosition(0, x, y, z);  // x, y, z are modified
```

### Pointer vs Reference

| Feature | Pointer | Reference |
|---------|---------|-----------|
| Syntax | `int* p` | `int& r` |
| Can be null | Yes | No |
| Can be reassigned | Yes | No |
| Access | `*p` or `p->` | `r` or `r.` |

## Static Variables

`static` variables retain their value between function calls:

```cpp
void UserInput::update(...) {
    static int frameCount = 0;  // Only initialized once!
    
    frameCount++;
    if (frameCount % 60 == 0) {
        // Print debug info every 60 frames
        std::cout << "PlayerY: " << playerY << std::endl;
    }
}
```

## Header Guards

Prevent including the same header twice:

```cpp
// UserInput.h
#ifndef USERINPUT_H    // If not defined
#define USERINPUT_H    // Define it

class UserInput {
    // ... class definition
};

#endif // USERINPUT_H  // End of guard
```

Without guards, including the same header twice would cause "redefinition" errors.

## Forward Declarations

Declare a class exists without including its header:

```cpp
// In UserInput.h
class ObstacleCourse;  // Forward declaration

class UserInput {
    void update(..., ObstacleCourse* course, ...);  // Can use pointer/reference
};
```

This:
1. Reduces compile time (less headers to process)
2. Avoids circular dependencies (A includes B, B includes A)

## Type Casting

Convert between types:

```cpp
// C++ style casts (preferred)
float f = 3.14f;
int i = static_cast<int>(f);  // i = 3

// C style cast (works but less safe)
int j = (int)f;  // Same result

// In the code:
float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
```

## The `std::` Namespace

Standard library classes are in the `std` namespace:

```cpp
#include <string>
#include <vector>
#include <map>
#include <algorithm>

std::string text = "Hello";
std::vector<int> numbers;
std::map<char, int> charMap;
std::max(a, b);  // Return larger of a, b
```

You can use `using namespace std;` to avoid typing `std::`, but it's not recommended in headers.

## Next Steps

Continue to [OpenGL Basics](./03_opengl_basics.md) to learn about the graphics rendering.
