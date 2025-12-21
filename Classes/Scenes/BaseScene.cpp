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
#include "Soldier/UnitManager.h"
#include "UI/IDCardPanel.h"
#include <algorithm>

USING_NS_CC;

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

    // 初始化资源
    _currentGold = 1000;
    _currentDiamond = 100;

    // 按模块化方式初始化各个组件
    initGridMap();
    initBaseBuilding();
    initUIComponents();
    initBuildingSystem();

    // 加载兵种配置
    UnitManager::getInstance()->loadConfig("res/units_config.json");

    createTrainPanel();
    initTouchListener();

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
        }
    );

    if (_trainPanel) {
        this->addChild(_trainPanel, 102);
        CCLOG("[基地场景] 训练面板创建成功");
    }
}

void BaseScene::showTrainPanel() {
    if (_trainPanel) {
        _trainPanel->show();
        CCLOG("[基地场景] 打开训练面板");
    }
}

void BaseScene::onUnitTrainComplete(int unitId) {
    CCLOG("[基地场景] 兵种训练完成: ID=%d", unitId);
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

        CCLOG("[基地场景] 兵营建筑放置完成 (5x5格子，位置30,36)");
    }
}

// ==================== 建筑缩放调整常量 ====================
namespace BuildingScaleConfig {
    constexpr float PADDING_FACTOR = 0.85f;  // 建筑与格子边缘的间距系数
}

// ==================== 建筑缩放调整 ====================

void BaseScene::scaleBuildingToFit(Node* building, int gridWidth, int gridHeight, float cellSize) {
    if (!building) return;

    // 检查是否有子节点
    if (building->getChildrenCount() == 0) return;

    // 获取建筑的精灵
    auto sprite = dynamic_cast<Sprite*>(building->getChildren().at(0));
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

    // 应用缩放（最小缩放0.1，确保小图能放大）
    if (scale > 0.1f) {
        sprite->setScale(scale);
        CCLOG("[基地场景] 建筑缩放调整: 原尺寸(%.1f, %.1f) -> 目标(%.1f, %.1f), scale=%.2f",
            originalSize.width, originalSize.height, targetWidth, targetHeight, scale);
    }
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

        // 扣除费用
        _currentGold -= option.cost;

        CCLOG("[基地场景] 建筑创建成功，剩余金币: %d", _currentGold);
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
