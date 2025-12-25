// GridMap.h
#ifndef __GRID_MAP_H__
#define __GRID_MAP_H__

#include "cocos2d.h"
#include <vector>

class Building;

enum class CellType {
    EMPTY = 0,
    OCCUPIED = 1,
    FORBIDDEN = 2
};

class GridMap : public cocos2d::Node {
public:
    static GridMap* create(int width, int height, float cellSize);
    virtual bool init(int width, int height, float cellSize);

    // Core methods
    bool canPlaceBuilding(int x, int y, int width, int height);
    void occupyCell(int x, int y, int width, int height, cocos2d::Node* building);
    void freeCell(int x, int y, int width, int height);
    cocos2d::Vec2 gridToWorld(int x, int y);
    cocos2d::Vec2 worldToGrid(cocos2d::Vec2 pos);

    // Debug rendering
    void showGrid(bool show);

    int getGridWidth() const { return _gridWidth; }
    int getGridHeight() const { return _gridHeight; }
    float getCellSize() const { return _cellSize; }

private:
    int _gridWidth;
    int _gridHeight;
    float _cellSize;
    std::vector<std::vector<CellType>> _cells;
    std::vector<std::vector<cocos2d::Node*>> _buildings;
    cocos2d::DrawNode* _gridLines;

    void initCells();
    void drawGridLines();
};

#endif // __GRID_MAP_H__
