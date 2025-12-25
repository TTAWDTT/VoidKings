/**
 * @file BaseScene.cpp
 * @brief 基地场景实现文件（模块化重构版本）
 *
 * 使用模块化组件实现基地场景的各项功能：
 * - BuildShopPanel: 建筑商店面板
 * - PlacementManager: 建筑放置管理器（含格子颜色显示）
 * - BaseUIPanel: UI面板
 * - GridBackground: 网格背景
 */

#include "BaseScene.h"
#include "MainMenuScene.h"
#include "LevelSelectScene.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Buildings/StorageBuilding.h"
#include "UI/TrainPanel.h"
#include "Core/Core.h"
#include "Soldier/UnitManager.h"
#include "Utils/AudioManager.h"
#include <algorithm>
#include <cmath>

USING_NS_CC;

namespace {
constexpr float kHoverPanelPaddingX = 16.0f;
constexpr float kHoverPanelPaddingY = 14.0f;
constexpr float kHoverPanelMinWidth = 260.0f;
constexpr float kHoverPanelMinHeight = 140.0f;

struct SavedBuilding {
    BuildingOption option;
    int gridX = 0;
    int gridY = 0;
    int level = 0;
};

std::vector<SavedBuilding> s_savedBuildings;

Sprite* findBodySprite(Node* building) {
    if (!building) {
        return nullptr;
    }
    if (auto* named = dynamic_cast<Sprite*>(building->getChildByName("bodySprite"))) {
        return named;
    }
    Sprite* largest = nullptr;
    float largestArea = 0.0f;
    for (auto* child : building->getChildren()) {
        auto* sprite = dynamic_cast<Sprite*>(child);
        if (!sprite) {
            continue;
        }
        if (sprite->getName() == "healthBar") {
            continue;
        }
        Size size = sprite->getContentSize();
        float area = size.width * size.height;
        if (!largest || area > largestArea) {
            largest = sprite;
            largestArea = area;
        }
    }
    return largest;
}

bool hitTestBuilding(Node* building, const Vec2& worldPos) {
    if (!building) {
        return false;
    }
    auto* bodySprite = findBodySprite(building);
    if (bodySprite) {
        Vec2 localPos = building->convertToNodeSpace(worldPos);
        return bodySprite->getBoundingBox().containsPoint(localPos);
    }
    auto* parent = building->getParent();
    if (parent) {
        Vec2 localPos = parent->convertToNodeSpace(worldPos);
        return building->getBoundingBox().containsPoint(localPos);
    }
    return false;
}

void drawDashedCircle(DrawNode* node, const Vec2& center, float radius, const Color4F& color) {
    if (!node || radius <= 0.0f) {
        return;
    }
    node->clear();
    const int segments = 48;
    constexpr float kPi = 3.1415926f;
    const float step = 2.0f * kPi / segments;
    for (int i = 0; i < segments; ++i) {
        if (i % 2 != 0) {
            continue;
        }
        float angle1 = step * i;
        float angle2 = step * (i + 1);
        Vec2 p1(center.x + radius * std::cos(angle1), center.y + radius * std::sin(angle1));
        Vec2 p2(center.x + radius * std::cos(angle2), center.y + radius * std::sin(angle2));
        node->drawLine(p1, p2, color);
    }
}
} // namespace

// ==================== 场景创建与初始化 ====================

Scene* BaseScene::createScene() {
    return BaseScene::create();
}

bool BaseScene::init() {
    if (!Scene::init()) {
        return false;
    }

    // 清理战斗场景的静态引用，避免野指针
    DefenceBuilding::setEnemySoldiers(nullptr);

    // 按模块化方式初始化各个组件
    initGridMap();
    initBaseBuilding();
    initUIComponents();
    initBuildingSystem();
    _effectLayer = Node::create();
    this->addChild(_effectLayer, 150);

    // 加载兵种配置
    UnitManager::getInstance()->loadConfig("res/units_config.json");

    createTrainPanel();
    initTouchListener();
    initHoverInfo();
    AudioManager::playMainBgm();

    // 定时刷新资源显示，确保生产资源同步到UI
    this->schedule([this](float) {
        if (_uiPanel) {
            _uiPanel->updateResourceDisplay(
                Core::getInstance()->getResource(ResourceType::COIN),
                Core::getInstance()->getResource(ResourceType::DIAMOND)
            );
        }
    }, 0.5f, "resource_tick");

    CCLOG("[基地场景] 初始化完成（模块化版本）");

    return true;
}

// ==================== 网格地图初始化 ====================

void BaseScene::initGridMap() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 创建网格地图（80x80格子，每格32像素 - 4倍于原来的40x40）
    _gridMap = GridMap::create(80, 80, 32.0f);
    
    // 计算初始位置：将视图中心对准地图中心（建筑区域）
    // 建筑放置在(36,36)附近，将视图移动使其可见
    // 1280x720分辨率下，需要确保建筑区域在屏幕中心
    float cellSize = 32.0f;
    float mapCenterX = 36.0f * cellSize;  // 建筑区域中心X
    float mapCenterY = 36.0f * cellSize;  // 建筑区域中心Y
    
    // 计算偏移量：屏幕中心 - 地图上的目标点
    // 为左侧UI预留空间，地图中心略向右偏
    float targetCenterX = origin.x + visibleSize.width * 0.60f;
    float targetCenterY = origin.y + visibleSize.height * 0.52f;
    float offsetX = targetCenterX - mapCenterX;
    float offsetY = targetCenterY - mapCenterY;

    _gridMap->setPosition(Vec2(offsetX, offsetY));
    this->addChild(_gridMap, 0);

    // 创建建筑层（作为GridMap的子节点）
    _buildingLayer = Node::create();
    _gridMap->addChild(_buildingLayer, 10);

    // 创建网格背景组件
    _gridBackground = GridBackground::create(80, 80, 32.0f);
    if (_gridBackground) {
        _gridMap->addChild(_gridBackground, -1);
    }

    CCLOG("[基地场景] 网格地图初始化完成（80x80格子），视图已居中到建筑区域");
}

// ==================== UI组件初始化 ====================

void BaseScene::initUIComponents() {
    // 创建UI面板组件，传入回调函数
    BaseUICallbacks callbacks;
    callbacks.onAttack = [this]() { this->onAttackClicked(); };
    callbacks.onBuild = [this]() { this->onBuildClicked(); };
    callbacks.onExit = [this]() { this->onExitClicked(); };

    _uiPanel = BaseUIPanel::create(callbacks);
    if (_uiPanel) {
        this->addChild(_uiPanel, 100);
    }

    CCLOG("[基地场景] UI组件初始化完成");
}

// ==================== 建造系统初始化 ====================

void BaseScene::initBuildingSystem() {
    // 创建建筑商店面板
    _buildShopPanel = BuildShopPanel::create(
        // 建筑选择回调
        [this](const BuildingOption& option) {
            this->onBuildingSelected(option);
        },
        // 关闭回调
        [this]() {
            CCLOG("[基地场景] 建筑商店关闭");
            if (_uiPanel && (!_placementManager || !_placementManager->isPlacing())) {
                _uiPanel->setButtonsEnabled(true);
            }
        }
    );
    if (_buildShopPanel) {
        this->addChild(_buildShopPanel, 101);
    }

    // 创建放置管理器
    _placementManager = PlacementManager::create(
        _gridMap,
        // 放置确认回调
        [this](const BuildingOption& option, int gridX, int gridY) {
            this->onPlacementConfirmed(option, gridX, gridY);
        },
        // 放置取消回调
        [this]() {
            CCLOG("[基地场景] 建筑放置取消");
            AudioManager::playButtonCancel();
            if (_uiPanel) {
                _uiPanel->setButtonsEnabled(true);
            }
        }
    );
    if (_placementManager) {
        _gridMap->addChild(_placementManager, 50);
    }

    CCLOG("[基地场景] 建造系统初始化完成");
}

// ==================== 训练面板创建 ====================

void BaseScene::createTrainPanel() {
    _trainPanel = TrainPanel::create(
        // 训练完成回调
        [this](int unitId) {
            this->onUnitTrainComplete(unitId);
        },
        // 关闭回调
        [this]() {
            CCLOG("[基地场景] 训练面板关闭");
            if (_uiPanel) {
                _uiPanel->updateResourceDisplay(
                    Core::getInstance()->getResource(ResourceType::COIN),
                    Core::getInstance()->getResource(ResourceType::DIAMOND)
                );
                _uiPanel->setButtonsEnabled(true);
            }
        }
    );

    if (_trainPanel) {
        this->addChild(_trainPanel, 102);
        CCLOG("[基地场景] 训练面板创建成功");
    }
}

void BaseScene::showTrainPanel() {
    if (_trainPanel) {
        if (_uiPanel) {
            _uiPanel->setButtonsEnabled(false);
        }
        AudioManager::playButtonClick();
        _trainPanel->show();
        CCLOG("[基地场景] 打开训练面板");
    }
}

void BaseScene::onUnitTrainComplete(int unitId) {
    CCLOG("[基地场景] 兵种训练完成: ID=%d", unitId);
    if (_uiPanel) {
        _uiPanel->updateResourceDisplay(
            Core::getInstance()->getResource(ResourceType::COIN),
            Core::getInstance()->getResource(ResourceType::DIAMOND)
        );
    }
}

// ==================== 基地建筑初始化 ====================

void BaseScene::initBaseBuilding() {
    BuildingManager::getInstance()->setGridMap(_gridMap);
    float cellSize = _gridMap->getCellSize();

    // 初始化基地建筑配置（4x4格子，符合部落冲突设定）
    _baseConfig.id = 3001;
    _baseConfig.name = "Base";
    _baseConfig.spriteFrameName = "buildings/base.png";
    _baseConfig.HP = { 2000, 2500, 3000 };
    _baseConfig.DP = { 0, 0.05f, 0.1f };
    _baseConfig.length = 4;  // 大本营 4x4
    _baseConfig.width = 4;
    _baseConfig.MAXLEVEL = 2;
    _baseConfig.PRODUCE_GOLD = { 0, 0, 0 };
    _baseConfig.PRODUCE_ELIXIR = { 0, 0, 0 };
    _baseConfig.STORAGE_GOLD_CAPACITY = { 1000, 2000, 4000 };
    _baseConfig.STORAGE_ELIXIR_CAPACITY = { 500, 1000, 2000 };

    auto base = ProductionBuilding::create(&_baseConfig, 0);
    if (base) {
        _buildingLayer->addChild(base);
        // 将基地放置在地图中心位置（80x80网格的中心为36,36附近，确保4x4建筑在网格内）
        BuildingManager::getInstance()->placeBuilding(base, 36, 36, 4, 4);

        // 调整建筑缩放以适应格子大小
        scaleBuildingToFit(base, 4, 4, cellSize);
        setupProductionCollect(base);

        CCLOG("[基地场景] 基地建筑放置完成 (4x4格子，位置36,36)");
    }

    // 初始化兵营建筑配置（5x5格子）
    _barracksConfig.id = 3002;
    _barracksConfig.name = "SoldierBuilder";
    _barracksConfig.spriteFrameName = "buildings/soldierbuilder.png";
    _barracksConfig.HP = { 600, 750, 900 };
    _barracksConfig.DP = { 0, 0, 0 };
    _barracksConfig.length = 5;  // 兵营 5x5
    _barracksConfig.width = 5;
    _barracksConfig.MAXLEVEL = 2;
    _barracksConfig.PRODUCE_GOLD = { 0, 0, 0 };
    _barracksConfig.PRODUCE_ELIXIR = { 0, 0, 0 };
    _barracksConfig.STORAGE_GOLD_CAPACITY = { 0, 0, 0 };
    _barracksConfig.STORAGE_ELIXIR_CAPACITY = { 0, 0, 0 };

    auto barracks = ProductionBuilding::create(&_barracksConfig, 0);
    if (barracks) {
        _buildingLayer->addChild(barracks);
        // 将兵营放置在基地旁边（5x5格子，确保在网格内）
        BuildingManager::getInstance()->placeBuilding(barracks, 30, 36, 5, 5);

        // 调整建筑缩放以适应格子大小
        scaleBuildingToFit(barracks, 5, 5, cellSize);
        setupProductionCollect(barracks);

        CCLOG("[基地场景] 兵营建筑放置完成 (5x5格子，位置30,36)");
    }

    restoreSavedBuildings();
}

// ==================== 建筑缩放调整常量 ====================
namespace BuildingScaleConfig {
    constexpr float PADDING_FACTOR = 0.85f;  // 建筑与格子边缘的间距系数
}

// ==================== 建筑缩放调整 ====================

void BaseScene::scaleBuildingToFit(Node* building, int gridWidth, int gridHeight, float cellSize) {
    if (!building) return;

    auto sprite = findBodySprite(building);
    if (!sprite) return;

    // 计算目标尺寸（占据的格子空间，留一点边距）
    float targetWidth = gridWidth * cellSize * BuildingScaleConfig::PADDING_FACTOR;
    float targetHeight = gridHeight * cellSize * BuildingScaleConfig::PADDING_FACTOR;

    // 获取精灵原始尺寸
    Size originalSize = sprite->getContentSize();
    if (originalSize.width <= 0 || originalSize.height <= 0) return;

    // 计算缩放比例
    float scaleX = targetWidth / originalSize.width;
    float scaleY = targetHeight / originalSize.height;
    float scale = std::min(scaleX, scaleY);

    if (scale <= 0.0f) {
        return;
    }

    sprite->setScale(scale);
    if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
        defence->refreshHealthBarPosition();
    }
    else if (auto* production = dynamic_cast<ProductionBuilding*>(building)) {
        production->refreshHealthBarPosition();
        production->refreshCollectIconPosition();
    }
    else if (auto* storage = dynamic_cast<StorageBuilding*>(building)) {
        storage->refreshHealthBarPosition();
    }
    CCLOG("[基地场景] 建筑缩放调整: 原尺寸(%.1f, %.1f) -> 目标(%.1f, %.1f), scale=%.2f",
        originalSize.width, originalSize.height, targetWidth, targetHeight, scale);
}

// ==================== 触摸事件初始化 ====================

void BaseScene::initTouchListener() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BaseScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BaseScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BaseScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    CCLOG("[基地场景] 触摸事件监听初始化完成");
}

// ==================== 悬浮信息初始化 ====================

void BaseScene::initHoverInfo() {
    _hoverInfoPanel = Node::create();
    _hoverInfoPanel->setVisible(false);
    this->addChild(_hoverInfoPanel, 200);

    _hoverInfoBg = LayerColor::create(Color4B(12, 12, 12, 240), 260, 140);
    _hoverInfoBg->setAnchorPoint(Vec2(0, 1));
    _hoverInfoBg->setIgnoreAnchorPointForPosition(false);
    _hoverInfoPanel->addChild(_hoverInfoBg);

    auto border = DrawNode::create();
    border->setName("hoverBorder");
    _hoverInfoPanel->addChild(border, 1);

    _hoverInfoLabel = Label::createWithTTF("", "fonts/ScienceGothic.ttf", 18);
    if (!_hoverInfoLabel) {
        _hoverInfoLabel = Label::createWithSystemFont("", "Arial", 18);
    }
    _hoverInfoLabel->setAnchorPoint(Vec2(0, 1));
    _hoverInfoLabel->setAlignment(TextHAlignment::LEFT);
    _hoverInfoLabel->setTextColor(Color4B(255, 255, 255, 255));
    _hoverInfoLabel->enableShadow(Color4B(0, 0, 0, 200), Size(1, -1), 2);
    _hoverInfoLabel->setWidth(300);
    _hoverInfoPanel->addChild(_hoverInfoLabel);

    if (_gridMap) {
        _hoverRangeNode = DrawNode::create();
        _hoverFootprintNode = DrawNode::create();
        _hoverRangeNode->setVisible(false);
        _hoverFootprintNode->setVisible(false);
        _gridMap->addChild(_hoverRangeNode, 25);
        _gridMap->addChild(_hoverFootprintNode, 26);
    }

    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = [this](EventMouse* event) {
        updateHoverInfo(Vec2(event->getCursorX(), event->getCursorY()));
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

// ==================== 按钮回调 ====================

void BaseScene::onAttackClicked() {
    CCLOG("[基地场景] 点击进攻按钮，跳转到关卡选择");

    // 获取已训练的兵种
    std::map<int, int> trainedUnits;
    if (_trainPanel) {
        trainedUnits = _trainPanel->getTrainedUnits();
    }

    // 跳转到关卡选择场景
    auto scene = LevelSelectScene::createScene(trainedUnits);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void BaseScene::onBuildClicked() {
    if (_buildShopPanel) {
        if (_uiPanel) {
            _uiPanel->setButtonsEnabled(false);
        }
        _buildShopPanel->show();
    }
}

void BaseScene::onExitClicked() {
    auto scene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// ==================== 建筑选择与放置 ====================

void BaseScene::onBuildingSelected(const BuildingOption& option) {
    CCLOG("[基地场景] 选择建筑: %s (尺寸: %dx%d)",
        option.name.c_str(), option.gridWidth, option.gridHeight);

    // 通过放置管理器开始放置
    if (_placementManager) {
        _placementManager->startPlacement(option);
    }
}

void BaseScene::onPlacementConfirmed(const BuildingOption& option, int gridX, int gridY) {
    CCLOG("[基地场景] 确认放置建筑: %s 在位置 (%d, %d)",
        option.name.c_str(), gridX, gridY);

    // 先扣除资源，避免放置成功但资源不足
    int cost = option.cost;
    if (!Core::getInstance()->consumeResource(ResourceType::COIN, cost)) {
        CCLOG("[基地场景] 金币不足，无法建造: %s", option.name.c_str());
        AudioManager::playButtonCancel();
        if (_uiPanel) {
            _uiPanel->updateResourceDisplay(
                Core::getInstance()->getResource(ResourceType::COIN),
                Core::getInstance()->getResource(ResourceType::DIAMOND)
            );
            _uiPanel->setButtonsEnabled(true);
        }
        return;
    }

    // 创建实际建筑
    Node* newBuilding = createBuildingFromOption(option);

    if (newBuilding) {
        _buildingLayer->addChild(newBuilding);

        // 计算建筑位置
        Vec2 buildingPos = calculateBuildingPosition(gridX, gridY, option.gridWidth, option.gridHeight);
        newBuilding->setPosition(buildingPos);

        // 调整建筑缩放以适应格子大小
        float cellSize = _gridMap->getCellSize();
        scaleBuildingToFit(newBuilding, option.gridWidth, option.gridHeight, cellSize);

        // 标记网格为已占用
        _gridMap->occupyCell(gridX, gridY, option.gridWidth, option.gridHeight, newBuilding);

        // 放置完成反馈
        newBuilding->runAction(Sequence::create(
            EaseBackOut::create(ScaleTo::create(0.12f, 1.05f)),
            EaseBackOut::create(ScaleTo::create(0.12f, 1.0f)),
            nullptr));

        savePlacedBuilding(option, gridX, gridY);
        AudioManager::playButtonClick();

        CCLOG("[基地场景] 建筑创建成功，剩余金币: %d",
            Core::getInstance()->getResource(ResourceType::COIN));
    }
    else {
        // 建筑创建失败时返还资源
        Core::getInstance()->addResource(ResourceType::COIN, cost);
    }

    if (_uiPanel) {
        _uiPanel->updateResourceDisplay(
            Core::getInstance()->getResource(ResourceType::COIN),
            Core::getInstance()->getResource(ResourceType::DIAMOND)
        );
        _uiPanel->setButtonsEnabled(true);
    }
}

// ==================== 建筑创建 ====================

Node* BaseScene::createBuildingFromOption(const BuildingOption& option) {
    Node* newBuilding = nullptr;

    switch (option.type) {
    case 1: { // 箭塔 (3x3格子)
        static DefenceBuildingConfig config;
        config.id = 2001;
        config.name = "ArrowTower";
        config.spriteFrameName = option.spritePath;
        config.HP = { 500, 750, 1000 };
        config.DP = { 0, 0.05f, 0.1f };
        config.ATK = { 30, 45, 60 };
        config.ATK_RANGE = { 200, 250, 300 };
        config.ATK_SPEED = { 1.0f, 0.9f, 0.8f };
        config.SKY_ABLE = true;
        config.GROUND_ABLE = true;
        config.bulletSpriteFrameName = "bullet/arrow.png";
        config.bulletSpeed = 400.0f;
        config.bulletIsAOE = false;
        config.bulletAOERange = 0.0f;
        config.length = 3;  // 3x3格子
        config.width = 3;
        config.MAXLEVEL = 2;
        newBuilding = DefenceBuilding::create(&config, 0);
        break;
    }
    case 2: { // 炮塔 (3x3格子)
        static DefenceBuildingConfig config;
        config.id = 2002;
        config.name = "BoomTower";
        config.spriteFrameName = option.spritePath;
        config.HP = { 600, 900, 1200 };
        config.DP = { 0.05f, 0.1f, 0.15f };
        config.ATK = { 50, 75, 100 };
        config.ATK_RANGE = { 180, 220, 260 };
        config.ATK_SPEED = { 1.5f, 1.4f, 1.3f };
        config.SKY_ABLE = false;
        config.GROUND_ABLE = true;
        config.bulletSpriteFrameName = "bullet/bomb.png";
        config.bulletSpeed = 200.0f;
        config.bulletIsAOE = true;
        config.bulletAOERange = 40.0f;
        config.length = 3;  // 3x3格子
        config.width = 3;
        config.MAXLEVEL = 2;
        newBuilding = DefenceBuilding::create(&config, 0);
        break;
    }
    case 3: { // 装饰树 (2x2格子)
        static DefenceBuildingConfig config;
        config.id = 2003;
        config.name = "Tree";
        config.spriteFrameName = option.spritePath;
        config.HP = { 450, 650, 850 };
        config.DP = { 0, 0.05f, 0.1f };
        config.ATK = { 25, 40, 55 };
        config.ATK_RANGE = { 220, 270, 320 };
        config.ATK_SPEED = { 0.8f, 0.7f, 0.6f };
        config.SKY_ABLE = false;
        config.GROUND_ABLE = true;
        config.length = 2;  // 2x2格子
        config.width = 2;
        config.MAXLEVEL = 2;
        newBuilding = DefenceBuilding::create(&config, 0);
        break;
    }
    case 4: { // 雪人仓库 (3x3格子)
        static StorageBuildingConfig config;
        config.id = 4001;
        config.name = "Snowman";
        config.spriteFrameName = option.spritePath;
        config.HP = { 800, 1000, 1200 };
        config.DP = { 0.1f, 0.15f, 0.2f };
        config.length = 3;  // 3x3格子
        config.width = 3;
        config.MAXLEVEL = 2;
        config.ADD_STORAGE_ELIXIR_CAPACITY = { 1000, 2000, 4000 };
        config.ADD_STORAGE_GOLD_CAPACITY = { 1000, 2000, 4000 };
        newBuilding = StorageBuilding::create(&config, 0);
        break;
    }
    case 5: { // 兵营 (5x5格子)
        static ProductionBuildingConfig config;
        config.id = 3002;
        config.name = "SoldierBuilder";
        config.spriteFrameName = option.spritePath;
        config.HP = { 600, 750, 900 };
        config.DP = { 0, 0, 0 };
        config.length = 5;  // 5x5格子
        config.width = 5;
        config.MAXLEVEL = 2;
        config.PRODUCE_ELIXIR = { 0, 0, 0 };
        config.STORAGE_ELIXIR_CAPACITY = { 0, 0, 0 };
        config.PRODUCE_GOLD = { 0, 0, 0 };
        config.STORAGE_GOLD_CAPACITY = { 0, 0, 0 };
        newBuilding = ProductionBuilding::create(&config, 0);
        setupProductionCollect(static_cast<ProductionBuilding*>(newBuilding));
        break;
    }
    case 6: { // 金币工厂 (3x3格子)
        static ProductionBuildingConfig config;
        config.id = 3003;
        config.name = "GoldMaker";
        config.spriteFrameName = option.spritePath;
        config.HP = { 500, 650, 820 };
        config.DP = { 0, 0.05f, 0.1f };
        config.length = 3;
        config.width = 3;
        config.MAXLEVEL = 2;
        config.PRODUCE_GOLD = { 40, 55, 75 };
        config.PRODUCE_ELIXIR = { 0, 0, 0 };
        config.STORAGE_GOLD_CAPACITY = { 0, 0, 0 };
        config.STORAGE_ELIXIR_CAPACITY = { 0, 0, 0 };
        newBuilding = ProductionBuilding::create(&config, 0);
        setupProductionCollect(static_cast<ProductionBuilding*>(newBuilding));
        break;
    }
    case 7: { // 钻石工厂 (3x3格子)
        static ProductionBuildingConfig config;
        config.id = 3004;
        config.name = "DiamondMaker";
        config.spriteFrameName = option.spritePath;
        config.HP = { 520, 680, 860 };
        config.DP = { 0, 0.05f, 0.1f };
        config.length = 3;
        config.width = 3;
        config.MAXLEVEL = 2;
        config.PRODUCE_GOLD = { 0, 0, 0 };
        config.PRODUCE_ELIXIR = { 2, 3, 4 };
        config.STORAGE_GOLD_CAPACITY = { 0, 0, 0 };
        config.STORAGE_ELIXIR_CAPACITY = { 0, 0, 0 };
        newBuilding = ProductionBuilding::create(&config, 0);
        setupProductionCollect(static_cast<ProductionBuilding*>(newBuilding));
        break;
    }
    }

    return newBuilding;
}

// ==================== 位置计算 ====================

Vec2 BaseScene::calculateBuildingPosition(int gridX, int gridY, int width, int height) {
    // 计算建筑中心位置：左下角格子位置 + 建筑尺寸的一半
    float cellSize = _gridMap->getCellSize();
    float centerX = (gridX + width * 0.5f) * cellSize;
    float centerY = (gridY + height * 0.5f) * cellSize;
    return Vec2(centerX, centerY);
}

// ==================== 建筑状态保存/恢复 ====================

void BaseScene::savePlacedBuilding(const BuildingOption& option, int gridX, int gridY) {
    for (const auto& saved : s_savedBuildings) {
        if (saved.gridX == gridX && saved.gridY == gridY && saved.option.type == option.type) {
            return;
        }
    }

    SavedBuilding saved;
    saved.option = option;
    saved.option.canBuild = true;
    saved.gridX = gridX;
    saved.gridY = gridY;
    saved.level = 0;
    s_savedBuildings.push_back(saved);
}

void BaseScene::restoreSavedBuildings() {
    if (!_gridMap || !_buildingLayer) {
        return;
    }

    float cellSize = _gridMap->getCellSize();
    for (const auto& saved : s_savedBuildings) {
        const auto& option = saved.option;
        if (!_gridMap->canPlaceBuilding(saved.gridX, saved.gridY, option.gridWidth, option.gridHeight)) {
            continue;
        }

        Node* building = createBuildingFromOption(option);
        if (!building) {
            continue;
        }

        _buildingLayer->addChild(building);
        Vec2 buildingPos = calculateBuildingPosition(saved.gridX, saved.gridY, option.gridWidth, option.gridHeight);
        building->setPosition(buildingPos);
        scaleBuildingToFit(building, option.gridWidth, option.gridHeight, cellSize);
        _gridMap->occupyCell(saved.gridX, saved.gridY, option.gridWidth, option.gridHeight, building);
    }
}

void BaseScene::setupProductionCollect(ProductionBuilding* building) {
    if (!building) {
        return;
    }
    int id = building->getId();
    const std::string& name = building->getName();
    bool isCollector = (id == 3003 || id == 3004 || name == "GoldMaker" || name == "DiamondMaker");
    if (!isCollector) {
        return;
    }

    building->setCollectCallback([this](ProductionBuilding*, ResourceType type, int amount, const Vec2& worldPos) {
        Core::getInstance()->addResource(type, amount);
        if (_uiPanel) {
            _uiPanel->updateResourceDisplay(
                Core::getInstance()->getResource(ResourceType::COIN),
                Core::getInstance()->getResource(ResourceType::DIAMOND)
            );
        }
        playCollectEffect(type, amount, worldPos);
    });
}

void BaseScene::playCollectEffect(ResourceType type, int amount, const Vec2& worldPos) {
    if (!_effectLayer) {
        return;
    }

    auto sprite = Core::getInstance()->createResourceSprite(type);
    if (sprite) {
        sprite->setPosition(worldPos);
        sprite->setScale(0.75f);
        _effectLayer->addChild(sprite, 5);

        Vec2 targetPos = _uiPanel ? _uiPanel->getResourceIconWorldPosition(type) : Vec2::ZERO;
        if (targetPos == Vec2::ZERO) {
            targetPos = worldPos + Vec2(0.0f, 120.0f);
        }
        auto move = EaseSineIn::create(MoveTo::create(0.6f, targetPos));
        auto fade = FadeOut::create(0.6f);
        sprite->runAction(Sequence::create(Spawn::create(move, fade, nullptr), RemoveSelf::create(), nullptr));
    }

    std::string text = (type == ResourceType::COIN)
        ? StringUtils::format("Gold +%d", amount)
        : StringUtils::format("Diamond +%d", amount);

    auto label = Label::createWithTTF(text, "fonts/ScienceGothic.ttf", 16);
    if (!label) {
        label = Label::createWithSystemFont(text, "Arial", 16);
    }
    label->setPosition(worldPos + Vec2(0.0f, 12.0f));
    label->setColor(type == ResourceType::COIN ? Color3B(255, 220, 90) : Color3B(130, 210, 255));
    _effectLayer->addChild(label, 6);

    auto rise = MoveBy::create(0.7f, Vec2(0.0f, 26.0f));
    auto fade = FadeOut::create(0.7f);
    label->runAction(Sequence::create(Spawn::create(rise, fade, nullptr), RemoveSelf::create(), nullptr));
}

// ==================== 触摸事件处理 ====================

bool BaseScene::onTouchBegan(Touch* touch, Event* event) {
    // 如果训练面板正在显示，不处理触摸
    if (_trainPanel && _trainPanel->isShowing()) {
        return false;
    }

    // 如果建筑商店正在显示，不处理触摸
    if (_buildShopPanel && _buildShopPanel->isShowing()) {
        return false;
    }

    // 如果处于放置模式，由放置管理器处理
    if (_placementManager && _placementManager->isPlacing()) {
        return true;
    }

    // 检查是否点击了兵营建筑
    Vec2 touchPos = touch->getLocation();
    Vec2 localInLayer = _buildingLayer->convertToNodeSpace(touchPos);

    auto& buildings = _buildingLayer->getChildren();
    for (auto& child : buildings) {
        auto building = dynamic_cast<ProductionBuilding*>(child);
        if (!building) continue;

        // 使用建筑子精灵包围盒做点击检测，避免Node尺寸为0导致失效
        bool isHit = false;
        Sprite* bodySprite = nullptr;
        for (auto& bChild : building->getChildren()) {
            bodySprite = dynamic_cast<Sprite*>(bChild);
            if (bodySprite) break;
        }

        if (bodySprite) {
            // 使用世界坐标转到建筑本地坐标，避免坐标系不一致导致点击失效
            Vec2 localInBuilding = building->convertToNodeSpace(touchPos);
            Rect spriteBox = bodySprite->getBoundingBox();
            isHit = spriteBox.containsPoint(localInBuilding);
        }
        else {
            isHit = building->getBoundingBox().containsPoint(localInLayer);
        }

        if (isHit) {
            // 只有兵营（SoldierBuilder, ID=3002）才打开训练面板
            if (building->getId() == 3002 || building->getName() == "SoldierBuilder") {
                CCLOG("[基地场景] 点击兵营，打开训练面板");
                showTrainPanel();
                return true;
            }
            else {
                CCLOG("[基地场景] 点击建筑: %s (ID: %d)",
                    building->getName().c_str(), building->getId());
            }
        }
    }

    return false;
}

void BaseScene::onTouchMoved(Touch* touch, Event* event) {
    // 如果处于放置模式，更新预览位置
    if (_placementManager && _placementManager->isPlacing()) {
        Vec2 touchPos = touch->getLocation();
        Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
        _placementManager->updatePreviewPosition(localPos);
    }
}

void BaseScene::onTouchEnded(Touch* touch, Event* event) {
    // 如果处于放置模式，尝试确认放置
    if (_placementManager && _placementManager->isPlacing()) {
        Vec2 touchPos = touch->getLocation();
        Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);

        // 尝试确认放置，如果失败则取消
        if (!_placementManager->tryConfirmPlacement(localPos)) {
            _placementManager->cancelPlacement();
        }
    }
}

// ==================== 悬浮信息处理 ====================

void BaseScene::updateHoverInfo(const Vec2& worldPos) {
    if (!_gridMap || !_buildingLayer) {
        return;
    }
    if (_trainPanel && _trainPanel->isShowing()) {
        clearBuildingInfo();
        return;
    }
    if (_buildShopPanel && _buildShopPanel->isShowing()) {
        clearBuildingInfo();
        return;
    }
    if (_placementManager && _placementManager->isPlacing()) {
        clearBuildingInfo();
        return;
    }

    Node* building = pickBuildingAt(worldPos);
    if (!building) {
        clearBuildingInfo();
        return;
    }

    if (building != _hoveredBuilding) {
        _hoveredBuilding = building;
    }
    showBuildingInfo(building);
    updateHoverPanelPosition(worldPos);
}

Node* BaseScene::pickBuildingAt(const Vec2& worldPos) const {
    if (!_buildingLayer) {
        return nullptr;
    }

    for (auto* child : _buildingLayer->getChildren()) {
        if (!child) {
            continue;
        }
        if (dynamic_cast<DefenceBuilding*>(child)
            || dynamic_cast<ProductionBuilding*>(child)
            || dynamic_cast<StorageBuilding*>(child)) {
            if (hitTestBuilding(child, worldPos)) {
                return child;
            }
        }
    }
    return nullptr;
}

void BaseScene::showBuildingInfo(Node* building) {
    if (!_hoverInfoPanel || !_hoverInfoLabel) {
        return;
    }

    std::string name = "Unknown";
    int level = 0;
    float hp = 0.0f;
    float maxHp = 0.0f;
    float atk = 0.0f;
    float range = 0.0f;
    int sizeW = 0;
    int sizeH = 0;
    int produceGold = 0;
    int produceElixir = 0;
    bool hasAttack = false;

    if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
        name = defence->getName();
        level = defence->getLevel() + 1;
        hp = defence->getCurrentHP();
        maxHp = defence->getCurrentMaxHP();
        atk = defence->getCurrentATK();
        range = defence->getCurrentATK_RANGE();
        sizeW = defence->getWidth();
        sizeH = defence->getLength();
        hasAttack = true;
    }
    else if (auto* production = dynamic_cast<ProductionBuilding*>(building)) {
        name = production->getName();
        level = production->getLevel() + 1;
        hp = production->getCurrentHP();
        maxHp = production->getCurrentMaxHP();
        sizeW = production->getWidth();
        sizeH = production->getLength();
        produceGold = static_cast<int>(production->getCurrentPRODUCE_GOLD());
        produceElixir = static_cast<int>(production->getCurrentPRODUCE_ELIXIR());
    }
    else if (auto* storage = dynamic_cast<StorageBuilding*>(building)) {
        name = storage->getName();
        level = storage->getLevel() + 1;
        hp = storage->getCurrentHP();
        maxHp = storage->getCurrentMaxHP();
        sizeW = storage->getWidth();
        sizeH = storage->getLength();
    }

    std::string attackText = hasAttack ? StringUtils::format("%.0f", atk) : "-";
    std::string rangeText = hasAttack ? StringUtils::format("%.0f", range) : "-";
    std::string produceText = "-";
    if (produceGold > 0 || produceElixir > 0) {
        produceText = StringUtils::format("Gold+%d Elixir+%d", produceGold, produceElixir);
    }

    char infoText[256];
    snprintf(infoText, sizeof(infoText),
        "Name: %s\nLevel: %d\nHP: %.0f / %.0f\nATK: %s\nRange: %s\nProduction: %s\nFootprint: %dx%d",
        name.c_str(),
        level,
        hp,
        maxHp,
        attackText.c_str(),
        rangeText.c_str(),
        produceText.c_str(),
        sizeW,
        sizeH
    );
    _hoverInfoLabel->setString(infoText);

    Size textSize = _hoverInfoLabel->getContentSize();
    float panelWidth = std::max(kHoverPanelMinWidth, textSize.width + kHoverPanelPaddingX * 2);
    float panelHeight = std::max(kHoverPanelMinHeight, textSize.height + kHoverPanelPaddingY * 2);
    _hoverInfoBg->setContentSize(Size(panelWidth, panelHeight));
    _hoverInfoLabel->setPosition(Vec2(kHoverPanelPaddingX, -kHoverPanelPaddingY));

    auto border = dynamic_cast<DrawNode*>(_hoverInfoPanel->getChildByName("hoverBorder"));
    if (border) {
        border->clear();
        border->drawRect(Vec2(0, 0), Vec2(panelWidth, -panelHeight), Color4F(0.8f, 0.8f, 0.8f, 1.0f));
    }

    _hoverInfoPanel->setVisible(true);
    updateHoverOverlays(building);
}

void BaseScene::updateHoverOverlays(Node* building) {
    if (!_hoverFootprintNode || !_hoverRangeNode || !_gridMap || !building) {
        return;
    }

    _hoverFootprintNode->clear();
    _hoverRangeNode->clear();

    float cellSize = _gridMap->getCellSize();
    int sizeW = 0;
    int sizeH = 0;
    float range = 0.0f;

    if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
        sizeW = defence->getWidth();
        sizeH = defence->getLength();
        range = defence->getCurrentATK_RANGE();
    }
    else if (auto* production = dynamic_cast<ProductionBuilding*>(building)) {
        sizeW = production->getWidth();
        sizeH = production->getLength();
    }
    else if (auto* storage = dynamic_cast<StorageBuilding*>(building)) {
        sizeW = storage->getWidth();
        sizeH = storage->getLength();
    }

    Vec2 center = building->getPosition();
    int gridX = static_cast<int>(std::floor(center.x / cellSize - sizeW * 0.5f + 0.001f));
    int gridY = static_cast<int>(std::floor(center.y / cellSize - sizeH * 0.5f + 0.001f));

    Vec2 bottomLeft(gridX * cellSize, gridY * cellSize);
    Vec2 topRight((gridX + sizeW) * cellSize, (gridY + sizeH) * cellSize);
    _hoverFootprintNode->drawRect(bottomLeft, topRight, Color4F(0.9f, 0.9f, 0.9f, 1.0f));
    _hoverFootprintNode->setVisible(true);

    if (range > 0.0f) {
        drawDashedCircle(_hoverRangeNode, center, range, Color4F(0.85f, 0.85f, 0.85f, 0.8f));
        _hoverRangeNode->setVisible(true);
    }
    else {
        _hoverRangeNode->setVisible(false);
    }
}

void BaseScene::updateHoverPanelPosition(const Vec2& worldPos) {
    if (!_hoverInfoPanel || !_hoverInfoBg) {
        return;
    }
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    Size panelSize = _hoverInfoBg->getContentSize();
    Vec2 desiredPos = worldPos + Vec2(16.0f, 20.0f);

    float maxX = origin.x + visibleSize.width - panelSize.width - 6.0f;
    float minX = origin.x + 6.0f;
    float maxY = origin.y + visibleSize.height - 6.0f;
    float minY = origin.y + panelSize.height + 6.0f;

    float clampedX = std::min(std::max(desiredPos.x, minX), maxX);
    float clampedY = std::min(std::max(desiredPos.y, minY), maxY);

    _hoverInfoPanel->setPosition(Vec2(clampedX, clampedY));
}

void BaseScene::clearBuildingInfo() {
    _hoveredBuilding = nullptr;
    if (_hoverInfoPanel) {
        _hoverInfoPanel->setVisible(false);
    }
    if (_hoverRangeNode) {
        _hoverRangeNode->clear();
        _hoverRangeNode->setVisible(false);
    }
    if (_hoverFootprintNode) {
        _hoverFootprintNode->clear();
        _hoverFootprintNode->setVisible(false);
    }
}
