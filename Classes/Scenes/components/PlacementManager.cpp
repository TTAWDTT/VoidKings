/**
 * @file PlacementManager.cpp
 * @brief 建筑放置管理器实现
 */

#include "PlacementManager.h"
#include <algorithm>

 // ===================================================
 // 创建与初始化
 // ===================================================

PlacementManager* PlacementManager::create(
    GridMap* gridMap,
    const std::function<void(const BuildingOption&, int, int)>& onPlacementConfirmed,
    const std::function<void()>& onPlacementCancelled
) {
    PlacementManager* ret = new (std::nothrow) PlacementManager();
    if (ret && ret->init(gridMap, onPlacementConfirmed, onPlacementCancelled)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool PlacementManager::init(
    GridMap* gridMap,
    const std::function<void(const BuildingOption&, int, int)>& onPlacementConfirmed,
    const std::function<void()>& onPlacementCancelled
) {
    if (!Node::init()) {
        return false;
    }

    _gridMap = gridMap;
    _onPlacementConfirmed = onPlacementConfirmed;
    _onPlacementCancelled = onPlacementCancelled;

    // 创建全地图格子显示节点
    _gridDisplayNode = DrawNode::create();
    this->addChild(_gridDisplayNode, 0);
    _gridDisplayNode->setVisible(false);

    // 创建放置位置格子显示节点
    _placementGridNode = DrawNode::create();
    this->addChild(_placementGridNode, 1);
    _placementGridNode->setVisible(false);

    CCLOG("[放置管理器] 初始化完成");

    return true;
}

// ===================================================
// 开始放置建筑
// ===================================================
void PlacementManager::startPlacement(const BuildingOption& option) {
    // 清理之前的预览
    cleanupPreview();

    // 更新状态
    _state.isPlacing = true;
    _state.buildingType = option.type;
    _state.gridWidth = option.gridWidth;
    _state.gridHeight = option.gridHeight;
    _state.spritePath = option.spritePath;
    _state.cost = option.cost;
    _currentOption = option;

    // 创建预览精灵
    _previewSprite = Sprite::create(option.spritePath);
    if (_previewSprite) {
        _previewSprite->setOpacity(PlacementConfig::PREVIEW_OPACITY);
        this->addChild(_previewSprite, 10);
    }

    if (_previewSprite && _gridMap) {
        float cellSize = _gridMap->getCellSize();
        Size spriteSize = _previewSprite->getContentSize();
        if (spriteSize.width > 0 && spriteSize.height > 0) {
            float targetWidth = _state.gridWidth * cellSize * 0.85f;
            float targetHeight = _state.gridHeight * cellSize * 0.85f;
            float scaleX = targetWidth / spriteSize.width;
            float scaleY = targetHeight / spriteSize.height;
            float scale = std::min(scaleX, scaleY);
            if (scale > 0.0f) {
                _previewSprite->setScale(scale);
            }
        }
        float baseScale = _previewSprite->getScale();
        _previewSprite->runAction(RepeatForever::create(Sequence::create(
            ScaleTo::create(0.6f, baseScale * 1.02f),
            ScaleTo::create(0.6f, baseScale),
            nullptr)));
    }

    setupPreviewInfo();

    // 绘制全地图格子状态
    drawAllGridCells();

    // 显示网格辅助线
    if (_gridMap) {
        _gridMap->showGrid(true);
    }

    CCLOG("[放置管理器] 开始放置建筑: %s (尺寸: %dx%d)",
        option.name.c_str(), option.gridWidth, option.gridHeight);
}

// ===================================================
// 绘制全地图格子状态（建造模式时显示）
// ===================================================
void PlacementManager::drawAllGridCells() {
    if (!_gridMap || !_gridDisplayNode) return;

    _gridDisplayNode->clear();
    _gridDisplayNode->setVisible(true);

    float cellSize = _gridMap->getCellSize();
    int gridWidth = _gridMap->getGridWidth();
    int gridHeight = _gridMap->getGridHeight();

    // 遍历所有格子，根据状态绘制不同颜色
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            Vec2 bottomLeft(x * cellSize, y * cellSize);
            Vec2 topRight((x + 1) * cellSize, (y + 1) * cellSize);

            // 检查格子是否可以放置建筑（单格检测）
            bool canPlace = _gridMap->canPlaceBuilding(x, y, 1, 1);

            Color4F fillColor;
            if (canPlace) {
                fillColor = PlacementConfig::GRID_COLOR_EMPTY;  // 空闲：淡绿色
            }
            else {
                // 检查是否是边界禁止区域
                if (x < 2 || x >= gridWidth - 2 || y < 2 || y >= gridHeight - 2) {
                    fillColor = PlacementConfig::GRID_COLOR_FORBIDDEN;  // 禁止区域：深灰色
                }
                else {
                    fillColor = PlacementConfig::GRID_COLOR_OCCUPIED;   // 已占用：灰色
                }
            }

            // 绘制填充矩形
            _gridDisplayNode->drawSolidRect(bottomLeft, topRight, fillColor);
        }
    }

    CCLOG("[放置管理器] 绘制全地图格子状态完成");
}

// ===================================================
// 隐藏格子状态显示
// ===================================================
void PlacementManager::hideGridDisplay() {
    if (_gridDisplayNode) {
        _gridDisplayNode->setVisible(false);
    }
    if (_placementGridNode) {
        _placementGridNode->setVisible(false);
    }
}

// ===================================================
// 更新预览位置
// ===================================================
void PlacementManager::updatePreviewPosition(const Vec2& worldPos) {
    if (!_state.isPlacing || !_previewSprite || !_gridMap) return;

    // 转换为网格坐标
    Vec2 gridPos = _gridMap->worldToGrid(worldPos);
    int gridX = (int)gridPos.x;
    int gridY = (int)gridPos.y;

    _currentGridX = gridX;
    _currentGridY = gridY;

    // 计算建筑中心位置
    Vec2 centerPos = calculateBuildingCenterPosition(gridX, gridY, _state.gridWidth, _state.gridHeight);
    _previewSprite->setPosition(centerPos);

    // 检查是否可以放置
    bool canPlace = _gridMap->canPlaceBuilding(gridX, gridY, _state.gridWidth, _state.gridHeight);

    // 更新当前放置位置的格子颜色显示
    updatePlacementGridDisplay(gridX, gridY, canPlace);
    updatePreviewInfo(canPlace);
    updatePreviewInfoPosition(centerPos);

    // 更新预览精灵颜色
    if (canPlace) {
        _previewSprite->setColor(Color3B::WHITE);
    }
    else {
        _previewSprite->setColor(Color3B::GRAY);
    }
}

// ===================================================
// 更新当前放置位置的格子颜色显示
// ===================================================
void PlacementManager::updatePlacementGridDisplay(int gridX, int gridY, bool canPlace) {
    if (!_placementGridNode || !_gridMap) return;

    _placementGridNode->clear();
    _placementGridNode->setVisible(true);

    float cellSize = _gridMap->getCellSize();

    // 根据可放置状态选择颜色
    Color4F fillColor = canPlace ?
        PlacementConfig::GRID_COLOR_CAN_PLACE :
        PlacementConfig::GRID_COLOR_CANNOT_PLACE;

    Color4F borderColor = canPlace ?
        PlacementConfig::GRID_BORDER_CAN_PLACE :
        PlacementConfig::GRID_BORDER_CANNOT_PLACE;

    // 绘制建筑占用的每个格子
    for (int dy = 0; dy < _state.gridHeight; ++dy) {
        for (int dx = 0; dx < _state.gridWidth; ++dx) {
            int cellX = gridX + dx;
            int cellY = gridY + dy;

            Vec2 bottomLeft(cellX * cellSize, cellY * cellSize);
            Vec2 topRight((cellX + 1) * cellSize, (cellY + 1) * cellSize);

            // 绘制填充矩形
            _placementGridNode->drawSolidRect(bottomLeft, topRight, fillColor);

            // 绘制边框
            Vec2 vertices[] = {
                bottomLeft,
                Vec2(topRight.x, bottomLeft.y),
                topRight,
                Vec2(bottomLeft.x, topRight.y)
            };
            _placementGridNode->drawPolygon(vertices, 4, Color4F(0, 0, 0, 0), 1.0f, borderColor);
        }
    }
}

// ===================================================
// 预览信息面板
// ===================================================
void PlacementManager::setupPreviewInfo() {
    if (_previewInfoPanel) {
        _previewInfoPanel->removeFromParent();
        _previewInfoPanel = nullptr;
        _previewInfoLabel = nullptr;
    }

    _previewInfoPanel = Node::create();
    _previewInfoPanel->setAnchorPoint(Vec2(0.5f, 0.5f));
    _previewInfoPanel->setIgnoreAnchorPointForPosition(false);
    _previewInfoPanel->setVisible(false);
    this->addChild(_previewInfoPanel, 12);

    auto bg = LayerColor::create(Color4B(20, 20, 20, 200), 170.0f, 52.0f);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    bg->setName("previewInfoBg");
    _previewInfoPanel->addChild(bg);

    _previewInfoLabel = Label::createWithTTF("", "fonts/ScienceGothic.ttf", 14);
    if (!_previewInfoLabel) {
        _previewInfoLabel = Label::createWithSystemFont("", "Arial", 14);
    }
    _previewInfoLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    _previewInfoLabel->setAlignment(TextHAlignment::CENTER);
    _previewInfoLabel->setTextColor(Color4B::WHITE);
    _previewInfoPanel->addChild(_previewInfoLabel, 1);
}

void PlacementManager::updatePreviewInfo(bool canPlace) {
    if (!_previewInfoPanel || !_previewInfoLabel) {
        return;
    }

    const char* stateText = canPlace ? "Place" : "Blocked";
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s  %dG\n%dx%d  %s",
        _currentOption.name.c_str(),
        _currentOption.cost,
        _currentOption.gridWidth,
        _currentOption.gridHeight,
        stateText);
    _previewInfoLabel->setString(buffer);

    auto bg = dynamic_cast<LayerColor*>(_previewInfoPanel->getChildByName("previewInfoBg"));
    if (bg) {
        Color4B bgColor = canPlace ? Color4B(30, 60, 30, 200) : Color4B(70, 30, 30, 200);
        bg->setColor(Color3B(bgColor.r, bgColor.g, bgColor.b));
        bg->setOpacity(bgColor.a);

        Size textSize = _previewInfoLabel->getContentSize();
        float width = std::max(170.0f, textSize.width + 20.0f);
        float height = std::max(52.0f, textSize.height + 14.0f);
        bg->setContentSize(Size(width, height));
    }

    _previewInfoPanel->setVisible(true);
}


void PlacementManager::updatePreviewInfoPosition(const Vec2& centerPos) {
    if (!_previewInfoPanel || !_gridMap) {
        return;
    }

    auto bg = dynamic_cast<LayerColor*>(_previewInfoPanel->getChildByName("previewInfoBg"));
    Size panelSize = bg ? bg->getContentSize() : Size(170.0f, 52.0f);

    float cellSize = _gridMap->getCellSize();
    float offsetY = _state.gridHeight * cellSize * 0.5f + 26.0f;
    Vec2 desired = centerPos + Vec2(0.0f, offsetY);

    float maxX = _gridMap->getGridWidth() * cellSize - panelSize.width * 0.5f - 6.0f;
    float minX = panelSize.width * 0.5f + 6.0f;
    float maxY = _gridMap->getGridHeight() * cellSize - panelSize.height * 0.5f - 6.0f;
    float minY = panelSize.height * 0.5f + 6.0f;

    float clampedX = std::max(minX, std::min(maxX, desired.x));
    float clampedY = std::max(minY, std::min(maxY, desired.y));

    _previewInfoPanel->setPosition(Vec2(clampedX, clampedY));
}


// ===================================================
// 尝试确认放置
// ===================================================
bool PlacementManager::tryConfirmPlacement(const Vec2& worldPos) {
    if (!_state.isPlacing || !_gridMap) {
        return false;
    }

    // 转换为网格坐标
    Vec2 gridPos = _gridMap->worldToGrid(worldPos);
    int gridX = (int)gridPos.x;
    int gridY = (int)gridPos.y;

    // 检查是否可以放置
    if (_gridMap->canPlaceBuilding(gridX, gridY, _state.gridWidth, _state.gridHeight)) {
        CCLOG("[放置管理器] 确认放置建筑: %s 在位置 (%d, %d)",
            _currentOption.name.c_str(), gridX, gridY);

        // 触发回调
        if (_onPlacementConfirmed) {
            _onPlacementConfirmed(_currentOption, gridX, gridY);
        }

        // 清理放置状态
        cleanupPreview();
        _state.isPlacing = false;

        // 隐藏网格辅助线和格子显示
        if (_gridMap) {
            _gridMap->showGrid(false);
        }
        hideGridDisplay();

        return true;
    }
    else {
        CCLOG("[放置管理器] 无法放置建筑，位置不可用");
        return false;
    }
}

// ===================================================
// 取消放置
// ===================================================
void PlacementManager::cancelPlacement() {
    if (!_state.isPlacing) return;

    CCLOG("[放置管理器] 取消放置建筑");

    cleanupPreview();
    _state.isPlacing = false;

    // 隐藏网格辅助线和格子显示
    if (_gridMap) {
        _gridMap->showGrid(false);
    }
    hideGridDisplay();

    // 触发回调
    if (_onPlacementCancelled) {
        _onPlacementCancelled();
    }
}

// ===================================================
// 计算建筑中心位置
// ===================================================
Vec2 PlacementManager::calculateBuildingCenterPosition(int gridX, int gridY, int width, int height) {
    if (!_gridMap) return Vec2::ZERO;

    float cellSize = _gridMap->getCellSize();
    float centerX = (gridX + width * 0.5f) * cellSize;
    float centerY = (gridY + height * 0.5f) * cellSize;
    return Vec2(centerX, centerY);
}

// ===================================================
// 清理预览资源
// ===================================================
void PlacementManager::cleanupPreview() {
    if (_previewSprite) {
        _previewSprite->removeFromParent();
        _previewSprite = nullptr;
    }
    if (_previewInfoPanel) {
        _previewInfoPanel->removeFromParent();
        _previewInfoPanel = nullptr;
        _previewInfoLabel = nullptr;
    }
}
