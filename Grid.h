#ifndef GRID_H
#define GRID_H

class Grid {
private:
    float cellNum;
    float cellSize;

public:
    Grid(float aCellNum, float aCellSize);
    void update();
    bool isOutOfBounds(float x, float z) const;
};

#endif // GRID_H
