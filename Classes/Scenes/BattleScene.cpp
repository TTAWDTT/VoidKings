/**
 * @file BattleScene.cpp
 * @brief 战斗场景实现
 *
 * 实现战斗系统的核心逻辑，包括：
 * - 关卡初始化和敌方建筑生成
 * - 玩家士兵部署
 * - 战斗逻辑更新
 */

#include "BattleScene.h"
#include "BaseScene.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Soldier/UnitManager.h"
#include <algorithm>

USING_NS_CC;

// ===================================================
// 场景创建
// ===================================================

Scene* BattleScene::createScene(int levelId, const std::map<int, int>& units) {
    auto scene = new (std::nothrow) BattleScene();
    if (scene) {
        scene->setLevelId(levelId);
        scene->setDeployableUnits(units);
        if (scene->init()) {
            scene->autorelease();
            return scene;
        }
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}

// ===================================================
// 初始化
// ===================================================

bool BattleScene::init() {
    if (!Scene::init()) {
        return false;
    }

    CCLOG("[战斗场景] 初始化关卡 %d", _levelId);

    // 确保兵种配置已加载（避免直接进入战斗时配置缺失）
    if (UnitManager::getInstance()->getAllUnitIds().empty()) {
        UnitManager::getInstance()->loadConfig("res/units_config.json");
    }

    // 初始化剩余可部署单位
    if (_deployableUnits.empty()) {
        auto unitIds = UnitManager::getInstance()->getAllUnitIds();
        int defaultCounts[] = { 10, 5, 8 };
        int maxDefaults = static_cast<int>(unitIds.size());
        if (maxDefaults > 3) {
            maxDefaults = 3;
        }
        for (int i = 0; i < maxDefaults; ++i) {
            _remainingUnits[unitIds[i]] = defaultCounts[i];
        }
    }
    else {
        _remainingUnits = _deployableUnits;
    }

    // 初始化各个组件
    initGridMap();
    initLevel();
    // 绑定战斗对象列表，便于自动寻敌
    Soldier::setEnemyBuildings(&_enemyBuildings);
    DefenceBuilding::setEnemySoldiers(&_soldiers);
    initUI();
    initTouchListener();

    // 设置更新
    this->scheduleUpdate();

    return true;
}

// ===================================================
// 网格地图初始化
// ===================================================

void BattleScene::initGridMap() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _gridMap = GridMap::create(
        BattleConfig::GRID_WIDTH,
        BattleConfig::GRID_HEIGHT,
        BattleConfig::CELL_SIZE
    );

    float mapWidth = BattleConfig::GRID_WIDTH * BattleConfig::CELL_SIZE;
    float mapHeight = BattleConfig::GRID_HEIGHT * BattleConfig::CELL_SIZE;

    // 将地图缩放到上下UI之间，避免被裁切
    float availableWidth = visibleSize.width;
    float availableHeight = visibleSize.height - BattleConfig::UI_TOP_HEIGHT - BattleConfig::UI_BOTTOM_HEIGHT;
    float scale = std::min(1.0f, std::min(availableWidth / mapWidth, availableHeight / mapHeight));

    float scaledWidth = mapWidth * scale;
    float scaledHeight = mapHeight * scale;

    float offsetX = origin.x + (visibleSize.width - scaledWidth) / 2;
    float offsetY = origin.y + BattleConfig::UI_BOTTOM_HEIGHT + (availableHeight - scaledHeight) / 2;

    _gridMap->setPosition(Vec2(offsetX, offsetY));
    _gridMap->setScale(scale);
    this->addChild(_gridMap, 0);

    auto bgColor = LayerColor::create(Color4B(60, 100, 60, 255), mapWidth, mapHeight);
    _gridMap->addChild(bgColor, -2);

    _buildingLayer = Node::create();
    _gridMap->addChild(_buildingLayer, 5);

    _soldierLayer = Node::create();
    _gridMap->addChild(_soldierLayer, 10);

    _gridMap->showGrid(true);

    CCLOG("[BattleScene] Grid map initialized");
}


// ===================================================
// 关卡初始化
// ===================================================

void BattleScene::initLevel() {
    // 根据关卡ID创建不同的关卡
    switch (_levelId) {
    case 1:
    default:
        createLevel1();
        break;
    }

    CCLOG("[战斗场景] 关卡 %d 初始化完成，共 %d 个建筑", _levelId, _totalBuildingCount);
}

// ===================================================
// 创建第1关 - 简单测试关卡
// ===================================================

void BattleScene::createLevel1() {
    // 在地图右侧放置敌方基地（40x30，确保4x4建筑在边界内）
    // 基地位置需要留足够空间：右边界为40，最大X为36
    createEnemyBase(28, 13);

    // 创建一些防御塔（3x3），需要确保在边界内
    createDefenseTower(22, 11, 1);  // 箭塔
    createDefenseTower(22, 17, 1);  // 箭塔
    createDefenseTower(24, 8, 2);   // 炮塔
    createDefenseTower(24, 20, 2);  // 炮塔
}

// ===================================================
// 创建敌方基地
// ===================================================

void BattleScene::createEnemyBase(int gridX, int gridY) {
    // 创建敌方基地建筑配置
    static ProductionBuildingConfig baseConfig;
    baseConfig.id = 9001;
    baseConfig.name = "EnemyBase";
    baseConfig.spriteFrameName = "buildings/base.png";
    baseConfig.HP = { 1000, 1500, 2000 };
    baseConfig.DP = { 0, 0, 0 };
    baseConfig.length = 4;
    baseConfig.width = 4;
    baseConfig.MAXLEVEL = 2;

    auto base = ProductionBuilding::create(&baseConfig, 0);
    if (base) {
        _buildingLayer->addChild(base);

        // 计算位置
        float cellSize = _gridMap->getCellSize();
        float centerX = (gridX + 2.0f) * cellSize;
        float centerY = (gridY + 2.0f) * cellSize;
        base->setPosition(Vec2(centerX, centerY));

        // 缩放建筑以适应格子大小
        scaleBuildingToFit(base, 4, 4, cellSize);

        // 标记格子占用
        _gridMap->occupyCell(gridX, gridY, 4, 4, base);

        _enemyBuildings.push_back(base);
        base->retain();
        _totalBuildingCount++;

        CCLOG("[战斗场景] 敌方基地创建完成");
    }
}

// ===================================================
// 创建防御塔
// ===================================================

void BattleScene::createDefenseTower(int gridX, int gridY, int type) {
    // 创建防御塔建筑配置
    static DefenceBuildingConfig towerConfig;

    if (type == 1) {
        // 箭塔
        towerConfig.id = 9101;
        towerConfig.name = "EnemyArrowTower";
        towerConfig.spriteFrameName = "buildings/ArrowTower.png";
        towerConfig.HP = { 300, 400, 500 };
        towerConfig.DP = { 0, 0.05f, 0.1f };
        towerConfig.ATK = { 20, 30, 40 };
        towerConfig.ATK_RANGE = { 150, 180, 210 };
        towerConfig.ATK_SPEED = { 1.0f, 0.9f, 0.8f };
        towerConfig.SKY_ABLE = true;
        towerConfig.GROUND_ABLE = true;
        towerConfig.bulletSpriteFrameName = "bullet/arrow.png";
        towerConfig.bulletSpeed = 400.0f;
        towerConfig.bulletIsAOE = false;
        towerConfig.bulletAOERange = 0.0f;
    }
    else {
        // 炮塔
        towerConfig.id = 9102;
        towerConfig.name = "EnemyBoomTower";
        towerConfig.spriteFrameName = "buildings/BoomTower.png";
        towerConfig.HP = { 400, 550, 700 };
        towerConfig.DP = { 0.05f, 0.1f, 0.15f };
        towerConfig.ATK = { 40, 55, 70 };
        towerConfig.ATK_RANGE = { 120, 150, 180 };
        towerConfig.ATK_SPEED = { 1.5f, 1.4f, 1.3f };
        towerConfig.SKY_ABLE = false;
        towerConfig.GROUND_ABLE = true;
        towerConfig.bulletSpriteFrameName = "bullet/bomb.png";
        towerConfig.bulletSpeed = 200.0f;
        towerConfig.bulletIsAOE = true;
        towerConfig.bulletAOERange = 40.0f;
    }

    towerConfig.length = 3;
    towerConfig.width = 3;
    towerConfig.MAXLEVEL = 2;

    auto tower = DefenceBuilding::create(&towerConfig, 0);
    if (tower) {
        _buildingLayer->addChild(tower);

        // 计算位置
        float cellSize = _gridMap->getCellSize();
        float centerX = (gridX + 1.5f) * cellSize;
        float centerY = (gridY + 1.5f) * cellSize;
        tower->setPosition(Vec2(centerX, centerY));

        // 缩放建筑以适应格子大小
        scaleBuildingToFit(tower, 3, 3, cellSize);

        // 标记格子占用
        _gridMap->occupyCell(gridX, gridY, 3, 3, tower);

        _enemyBuildings.push_back(tower);
        tower->retain();
        _totalBuildingCount++;

        CCLOG("[战斗场景] 防御塔创建完成 (类型%d)", type);
    }
}

// ===================================================
// 建筑缩放调整
// ===================================================

void BattleScene::scaleBuildingToFit(Node* building, int gridWidth, int gridHeight, float cellSize) {
    if (!building) return;

    // 检查是否有子节点
    if (building->getChildrenCount() == 0) return;

    // 获取建筑的精灵
    auto sprite = dynamic_cast<Sprite*>(building->getChildren().at(0));
    if (!sprite) return;

    // 计算目标尺寸（占据的格子空间，留一点边距）
    constexpr float PADDING_FACTOR = 0.85f;
    float targetWidth = gridWidth * cellSize * PADDING_FACTOR;
    float targetHeight = gridHeight * cellSize * PADDING_FACTOR;

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
    }
}

// ===================================================
// UI初始化
// ===================================================

void BattleScene::initUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _uiLayer = Node::create();
    this->addChild(_uiLayer, 100);

    // 顶部信息条
    auto topBg = LayerColor::create(
        Color4B(30, 30, 30, 220),
        visibleSize.width,
        BattleConfig::UI_TOP_HEIGHT
    );
    topBg->setPosition(origin.x, origin.y + visibleSize.height - BattleConfig::UI_TOP_HEIGHT);
    _uiLayer->addChild(topBg);

    float topY = origin.y + visibleSize.height - BattleConfig::UI_TOP_HEIGHT / 2;

    _timerLabel = Label::createWithTTF("3:00", "fonts/arial.ttf", 18);
    if (!_timerLabel) {
        _timerLabel = Label::createWithSystemFont("3:00", "Arial", 18);
    }
    _timerLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, topY));
    _timerLabel->setColor(Color3B::WHITE);
    _uiLayer->addChild(_timerLabel);

    _progressLabel = Label::createWithTTF("0%", "fonts/arial.ttf", 14);
    if (!_progressLabel) {
        _progressLabel = Label::createWithSystemFont("0%", "Arial", 14);
    }
    _progressLabel->setPosition(Vec2(origin.x + visibleSize.width - 60, topY));
    _progressLabel->setColor(Color3B::YELLOW);
    _uiLayer->addChild(_progressLabel);

    auto exitNode = Node::create();
    float btnWidth = 60.0f;
    float btnHeight = 25.0f;

    auto exitBg = LayerColor::create(Color4B(80, 40, 40, 255), btnWidth, btnHeight);
    exitBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    exitBg->setIgnoreAnchorPointForPosition(false);
    exitNode->addChild(exitBg);

    auto exitBorder = DrawNode::create();
    exitBorder->drawRect(Vec2(-btnWidth / 2, -btnHeight / 2), Vec2(btnWidth / 2, btnHeight / 2), Color4F::WHITE);
    exitNode->addChild(exitBorder, 1);

    auto exitLabel = Label::createWithTTF("EXIT", "fonts/arial.ttf", 10);
    if (!exitLabel) {
        exitLabel = Label::createWithSystemFont("EXIT", "Arial", 10);
    }
    exitLabel->setColor(Color3B::WHITE);
    exitNode->addChild(exitLabel, 2);

    float exitX = origin.x + 40.0f;
    exitNode->setPosition(Vec2(exitX, topY));
    _uiLayer->addChild(exitNode);

    auto exitBtn = Button::create();
    exitBtn->setContentSize(Size(btnWidth, btnHeight));
    exitBtn->setScale9Enabled(true);
    exitBtn->setPosition(Vec2(exitX, topY));
    exitBtn->addClickEventListener([this](Ref* sender) {
        CCLOG("[BattleScene] Exit battle");
        this->onExitButton(sender);
        });
    _uiLayer->addChild(exitBtn, 10);

    setupDeployArea();
}


// ===================================================
// 设置部署区域
// ===================================================

void BattleScene::setupDeployArea() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 底部部署条
    auto bottomBg = LayerColor::create(
        Color4B(30, 30, 30, 220),
        visibleSize.width,
        BattleConfig::UI_BOTTOM_HEIGHT
    );
    bottomBg->setPosition(origin.x, origin.y);
    _uiLayer->addChild(bottomBg);

    if (_remainingUnits.empty()) {
        auto unitIds = UnitManager::getInstance()->getAllUnitIds();
        int defaultCounts[] = { 10, 5, 8 };
        int maxDefaults = static_cast<int>(unitIds.size());
        if (maxDefaults > 3) {
            maxDefaults = 3;
        }
        for (int i = 0; i < maxDefaults; ++i) {
            _remainingUnits[unitIds[i]] = defaultCounts[i];
        }
    }

    _unitDeployArea = Node::create();
    _unitDeployArea->setPosition(Vec2::ZERO);
    _uiLayer->addChild(_unitDeployArea);

    float btnSize = 50.0f;
    float spacing = 16.0f;
    int unitCount = static_cast<int>(_remainingUnits.size());
    float totalWidth = unitCount * btnSize + (unitCount - 1) * spacing;
    float startX = origin.x + (visibleSize.width - totalWidth) / 2 + btnSize / 2;
    float centerY = origin.y + BattleConfig::UI_BOTTOM_HEIGHT / 2;

    int index = 0;
    for (const auto& pair : _remainingUnits) {
        float xPos = startX + index * (btnSize + spacing);
        auto btn = createDeployButton(pair.first, pair.second, xPos);
        if (btn) {
            btn->setPosition(Vec2(xPos, centerY));
            _unitDeployArea->addChild(btn);
        }
        index++;
    }

    auto tipLabel = Label::createWithTTF("Tap on map to deploy", "fonts/arial.ttf", 10);
    if (!tipLabel) {
        tipLabel = Label::createWithSystemFont("Tap on map to deploy", "Arial", 10);
    }
    tipLabel->setPosition(Vec2(
        origin.x + visibleSize.width - 90,
        centerY
    ));
    tipLabel->setColor(Color3B::GRAY);
    _uiLayer->addChild(tipLabel);
}


// ===================================================
// 创建部署按钮
// ===================================================

Node* BattleScene::createDeployButton(int unitId, int count, float x) {
    auto node = Node::create();
    node->setPosition(Vec2(x, 0));

    float btnSize = 50.0f;

    // 获取单位配置
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    std::string unitName = config ? config->name.substr(0, 3) : "???";

    // 按钮背景
    auto bg = LayerColor::create(Color4B(50, 70, 50, 255), btnSize, btnSize);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    node->addChild(bg);

    // 边框
    auto border = DrawNode::create();
    border->drawRect(Vec2(-btnSize / 2, -btnSize / 2), Vec2(btnSize / 2, btnSize / 2), Color4F::WHITE);
    node->addChild(border, 1);

    // 单位名称
    auto nameLabel = Label::createWithTTF(unitName, "fonts/arial.ttf", 10);
    if (!nameLabel) {
        nameLabel = Label::createWithSystemFont(unitName, "Arial", 10);
    }
    nameLabel->setPosition(Vec2(0, 8));
    nameLabel->setColor(Color3B::WHITE);
    node->addChild(nameLabel, 2);

    // 数量
    auto countLabel = Label::createWithTTF("x" + std::to_string(count), "fonts/arial.ttf", 10);
    if (!countLabel) {
        countLabel = Label::createWithSystemFont("x" + std::to_string(count), "Arial", 10);
    }
    countLabel->setPosition(Vec2(0, -10));
    countLabel->setColor(Color3B::YELLOW);
    countLabel->setName("countLabel");
    node->addChild(countLabel, 2);

    // 存储unitId以便部署时使用
    node->setTag(unitId);

    return node;
}

// ===================================================
// 部署士兵
// ===================================================

void BattleScene::deploySoldier(int unitId, const Vec2& position) {
    // 检查是否有可部署的该类型单位
    auto it = _remainingUnits.find(unitId);
    if (it == _remainingUnits.end() || it->second <= 0) {
        CCLOG("[战斗场景] 没有可部署的单位: %d", unitId);
        return;
    }

    // 创建士兵
    auto soldier = UnitManager::getInstance()->spawnSoldier(unitId, position, 0);
    if (soldier) {
        _soldierLayer->addChild(soldier);
        _soldiers.push_back(soldier);
        soldier->retain();

        // 更新剩余数量
        it->second--;

        CCLOG("[战斗场景] 部署士兵: %d 在位置 (%.1f, %.1f), 剩余 %d",
            unitId, position.x, position.y, it->second);

        // 更新UI
        for (auto& child : _unitDeployArea->getChildren()) {
            if (child->getTag() == unitId) {
                auto countLabel = child->getChildByName("countLabel");
                if (countLabel) {
                    static_cast<Label*>(countLabel)->setString("x" + std::to_string(it->second));
                }
                break;
            }
        }
    }
}

// ===================================================
// 触摸事件初始化
// ===================================================

void BattleScene::initTouchListener() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

bool BattleScene::onTouchBegan(Touch* touch, Event* event) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    Vec2 touchPos = touch->getLocation();

    // 检查是否在部署区域内（底部）
    if (touchPos.y < origin.y + BattleConfig::UI_BOTTOM_HEIGHT) {
        return false;
    }

    // 检查是否在顶部UI区域
    if (touchPos.y > origin.y + visibleSize.height - BattleConfig::UI_TOP_HEIGHT) {
        return false;
    }

    // 在地图上部署士兵（默认部署第一个有剩余的单位）
    for (auto& pair : _remainingUnits) {
        if (pair.second > 0) {
            Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
            deploySoldier(pair.first, localPos);
            break;
        }
    }

    return true;
}

// ===================================================
// 每帧更新
// ===================================================

void BattleScene::update(float dt) {
    if (_battleEnded) return;

    // 更新战斗时间
    _battleTime += dt;

    // 更新计时器显示
    float remainingTime = std::max(0.0f, BattleConfig::BATTLE_TIME_LIMIT - _battleTime);
    int minutes = static_cast<int>(remainingTime) / 60;
    int seconds = static_cast<int>(remainingTime) % 60;
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%d:%02d", minutes, seconds);
    if (_timerLabel) {
        _timerLabel->setString(timeStr);
    }

    // 更新战斗逻辑
    updateBattle(dt);

    // 检查战斗结束
    checkBattleEnd();
}

void BattleScene::onExit() {
    // 释放保留的引用，避免内存泄漏
    for (auto& soldier : _soldiers) {
        if (soldier) {
            soldier->release();
            soldier = nullptr;
        }
    }
    for (auto& building : _enemyBuildings) {
        if (building) {
            building->release();
            building = nullptr;
        }
    }
    _soldiers.clear();
    _enemyBuildings.clear();

    Scene::onExit();
}

// ===================================================
// 战斗逻辑更新
// ===================================================

void BattleScene::updateBattle(float dt) {
    // 清理已移除的士兵，避免悬空指针
    for (auto& soldier : _soldiers) {
        if (soldier && !soldier->getParent()) {
            soldier->release();
            soldier = nullptr;
        }
    }

    // 检查被摧毁的建筑并释放引用
    int destroyed = 0;
    for (auto& building : _enemyBuildings) {
        if (building && !building->getParent()) {
            building->release();
            building = nullptr;
        }
        if (!building) {
            destroyed++;
        }
    }
    _destroyedBuildingCount = destroyed;

    // 更新进度显示
    int progress = _totalBuildingCount > 0 ?
        (destroyed * 100 / _totalBuildingCount) : 0;
    char progressStr[16];
    snprintf(progressStr, sizeof(progressStr), "%d%%", progress);
    if (_progressLabel) {
        _progressLabel->setString(progressStr);
    }
}

// ===================================================
// 检查战斗结束
// ===================================================

void BattleScene::checkBattleEnd() {
    // 所有建筑被摧毁 - 胜利
    if (_destroyedBuildingCount >= _totalBuildingCount) {
        onBattleWin();
        return;
    }

    // 时间耗尽 - 失败
    if (_battleTime >= BattleConfig::BATTLE_TIME_LIMIT) {
        onBattleLose();
        return;
    }

    // 所有士兵阵亡且没有剩余可部署单位 - 失败
    bool hasSurvivors = false;
    for (auto& soldier : _soldiers) {
        if (soldier && soldier->getParent()) {
            hasSurvivors = true;
            break;
        }
    }

    bool hasRemainingUnits = false;
    for (auto& pair : _remainingUnits) {
        if (pair.second > 0) {
            hasRemainingUnits = true;
            break;
        }
    }

    if (!hasSurvivors && !hasRemainingUnits && _soldiers.size() > 0) {
        onBattleLose();
    }
}

// ===================================================
// 战斗胜利
// ===================================================

void BattleScene::onBattleWin() {
    _battleEnded = true;
    CCLOG("[战斗场景] 战斗胜利！");

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 显示胜利消息
    auto winLabel = Label::createWithTTF("VICTORY!", "fonts/arial.ttf", 36);
    if (!winLabel) {
        winLabel = Label::createWithSystemFont("VICTORY!", "Arial", 36);
    }
    winLabel->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2
    ));
    winLabel->setColor(Color3B::YELLOW);
    _uiLayer->addChild(winLabel, 100);

    // 3秒后返回
    this->scheduleOnce([](float dt) {
        auto scene = BaseScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        }, 3.0f, "return_to_base");
}

// ===================================================
// 战斗失败
// ===================================================

void BattleScene::onBattleLose() {
    _battleEnded = true;
    CCLOG("[战斗场景] 战斗失败！");

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 显示失败消息
    auto loseLabel = Label::createWithTTF("DEFEAT", "fonts/arial.ttf", 36);
    if (!loseLabel) {
        loseLabel = Label::createWithSystemFont("DEFEAT", "Arial", 36);
    }
    loseLabel->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2
    ));
    loseLabel->setColor(Color3B::RED);
    _uiLayer->addChild(loseLabel, 100);

    // 3秒后返回
    this->scheduleOnce([](float dt) {
        auto scene = BaseScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        }, 3.0f, "return_to_base");
}

// ===================================================
// 退出按钮回调
// ===================================================

void BattleScene::onExitButton(Ref* sender) {
    CCLOG("[战斗场景] 退出战斗");

    auto scene = BaseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}