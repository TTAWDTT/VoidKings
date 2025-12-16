// GridMap.cpp
#include "GridMap.h"

USING_NS_CC;

GridMap* GridMap::create(int width, int height, float cellSize) {
    GridMap* pRet = new(std::nothrow) GridMap();
    if (pRet && pRet->init(width, height, cellSize)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool GridMap::init(int width, int height, float cellSize) {
    if (!Node::init()) return false;

    _gridWidth = width;
    _gridHeight = height;
    _cellSize = cellSize;

    initCells();

    _gridLines = DrawNode::create();
    this->addChild(_gridLines);
    _gridLines->setVisible(false);

    return true;
}

void GridMap::initCells() {
    _cells.resize(_gridHeight);
    _buildings.resize(_gridHeight);
    for (int y = 0; y < _gridHeight; ++y) {
        _cells[y].resize(_gridWidth, CellType::EMPTY);
        _buildings[y].resize(_gridWidth, nullptr);
    }

    // Mark borders as forbidden (optional)
    for (int y = 0; y < _gridHeight; ++y) {
        for (int x = 0; x < _gridWidth; ++x) {
            if (x < 2 || x >= _gridWidth - 2 || y < 2 || y >= _gridHeight - 2) {
                _cells[y][x] = CellType::FORBIDDEN;
            }
        }
    }
}

bool GridMap::canPlaceBuilding(int x, int y, int width, int height) {
    // Check bounds
    if (x < 0 || y < 0 || x + width > _gridWidth || y + height > _gridHeight) {
        return false;
    }

    // Check all cells - 使用正确的行优先索引 [y][x]
    for (int j = y; j < y + height; ++j) {
        for (int i = x; i < x + width; ++i) {
            if (_cells[j][i] != CellType::EMPTY) {
                return false;
            }
        }
    }
    return true;
}

void GridMap::occupyCell(int x, int y, int width, int height, Node* building) {
    for (int j = y; j < y + height; ++j) {
        for (int i = x; i < x + width; ++i) {
            if (i >= 0 && i < _gridWidth && j >= 0 && j < _gridHeight) {
                _cells[j][i] = CellType::OCCUPIED;
                _buildings[j][i] = building;
            }
        }
    }
}

void GridMap::freeCell(int x, int y, int width, int height) {
    for (int j = y; j < y + height; ++j) {
        for (int i = x; i < x + width; ++i) {
            if (i >= 0 && i < _gridWidth && j >= 0 && j < _gridHeight) {
                _cells[j][i] = CellType::EMPTY;
                _buildings[j][i] = nullptr;
            }
        }
    }
}

Vec2 GridMap::gridToWorld(int x, int y) {
    return Vec2(x * _cellSize + _cellSize * 0.5f, y * _cellSize + _cellSize * 0.5f);
}

Vec2 GridMap::worldToGrid(Vec2 pos) {
    int x = (int)(pos.x / _cellSize);
    int y = (int)(pos.y / _cellSize);
    return Vec2(x, y);
}

void GridMap::showGrid(bool show) {
    if (show) {
        drawGridLines();
    }
    _gridLines->setVisible(show);
}

void GridMap::drawGridLines() {
    if (!_gridLines) return;

    _gridLines->clear();

    // Draw vertical lines
    for (int x = 0; x <= _gridWidth; ++x) {
        float xPos = x * _cellSize;
        _gridLines->drawLine(Vec2(xPos, 0), Vec2(xPos, _gridHeight * _cellSize), Color4F(0.5f, 0.5f, 0.5f, 0.5f));
    }

    // Draw horizontal lines
    for (int y = 0; y <= _gridHeight; ++y) {
        float yPos = y * _cellSize;
        _gridLines->drawLine(Vec2(0, yPos), Vec2(_gridWidth * _cellSize, yPos), Color4F(0.5f, 0.5f, 0.5f, 0.5f));
    }
}
