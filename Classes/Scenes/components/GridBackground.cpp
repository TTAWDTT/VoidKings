/**
 * @file GridBackground.cpp
 * @brief 网格背景绘制实现
 */

#include "GridBackground.h"
#include <algorithm>

// ===================================================
// 创建与初始化
// ===================================================
GridBackground* GridBackground::create(int gridWidth, int gridHeight, float cellSize) {
    GridBackground* ret = new (std::nothrow) GridBackground();
    if (ret && ret->init(gridWidth, gridHeight, cellSize)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool GridBackground::init(int gridWidth, int gridHeight, float cellSize) {
    if (!Node::init()) {
        return false;
    }

    _gridWidth = gridWidth;
    _gridHeight = gridHeight;
    _cellSize = cellSize;

    float totalWidth = _gridWidth * _cellSize;
    float totalHeight = _gridHeight * _cellSize;

    // 创建底色背景
    _bgColor = LayerColor::create(GridBackgroundConfig::BG_COLOR, totalWidth, totalHeight);
    this->addChild(_bgColor, -3);

    // 草地贴图作为底层纹理
    _tileBackground = Sprite::create("grass/grass_0000.png");
    if (_tileBackground) {
        auto texture = _tileBackground->getTexture();
        if (texture) {
            Texture2D::TexParams params = { GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT };
            texture->setTexParameters(params);
        }
        _tileBackground->setAnchorPoint(Vec2::ZERO);
        _tileBackground->setPosition(Vec2::ZERO);
        _tileBackground->setTextureRect(Rect(0, 0, totalWidth, totalHeight));
        _tileBackground->setOpacity(200);
        this->addChild(_tileBackground, -2);
    }

    // 创建网格线绘制节点
    _gridLines = DrawNode::create();
    this->addChild(_gridLines, -1);

    // 绘制网格
    drawDashedGrid();

    CCLOG("[网格背景] 初始化完成: %dx%d, 格子尺寸: %.1f", gridWidth, gridHeight, cellSize);

    return true;
}

// ===================================================
// 绘制虚线网格
// ===================================================
void GridBackground::drawDashedGrid() {
    if (!_gridLines) {
        return;
    }

    _gridLines->clear();

    float dashLength = GridBackgroundConfig::DASH_LENGTH;
    float gapLength = GridBackgroundConfig::GAP_LENGTH;
    Color4F lineColor = GridBackgroundConfig::LINE_COLOR;

    float totalWidth = _gridWidth * _cellSize;
    float totalHeight = _gridHeight * _cellSize;

    // 绘制竖线
    for (int x = 0; x <= _gridWidth; ++x) {
        float xPos = x * _cellSize;
        float y = 0;
        while (y < totalHeight) {
            float endY = std::min(y + dashLength, totalHeight);
            _gridLines->drawLine(
                Vec2(xPos, y),
                Vec2(xPos, endY),
                lineColor
            );
            y += dashLength + gapLength;
        }
    }

    // 绘制横线
    for (int y = 0; y <= _gridHeight; ++y) {
        float yPos = y * _cellSize;
        float x = 0;
        while (x < totalWidth) {
            float endX = std::min(x + dashLength, totalWidth);
            _gridLines->drawLine(
                Vec2(x, yPos),
                Vec2(endX, yPos),
                lineColor
            );
            x += dashLength + gapLength;
        }
    }
}

// ===================================================
// 重新绘制
// ===================================================
void GridBackground::redraw() {
    drawDashedGrid();
}
