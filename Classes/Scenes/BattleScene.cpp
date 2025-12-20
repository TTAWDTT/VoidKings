/**
 * @file BattleScene.cpp
 * @brief 战斗场景实现
 * 
 * 实现战斗系统的核心逻辑，包括：
 * - 关卡初始化和敌方建筑布置
 * - 玩家士兵部署
 * - 战斗逻辑更新
 */

#include "BattleScene.h"
#include "BaseScene.h"
#include "LevelSelectScene.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Soldier/UnitManager.h"
#include "Buildings/BuildingManager.h"
#include <algorithm>

USING_NS_CC;

// ===================================================
// 场景创建
// ===================================================

Scene* BattleScene::createScene(int levelId, const std::map<int, int>& units) {
    auto scene = BattleScene::create();
    if (scene) {
        scene->setLevelId(levelId);
        scene->setDeployableUnits(units);
    }
    return scene;
}

// ===================================================
// 初始化
// ===================================================

bool BattleScene::init() {
    if (!Scene::init()) {
        return false;
    }
    
    CCLOG("[战斗场景] 初始化关卡 %d", _levelId);
    
    // 初始化各组件
    initGridMap();
    initLevel();
    initUI();
    initTouchListener();
    
    // 初始化剩余可部署兵种
    _remainingUnits = _deployableUnits;
    
    // 启用更新
    this->scheduleUpdate();
    
    return true;
}

// ===================================================
// 网格地图初始化
// ===================================================

void BattleScene::initGridMap() {
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建战斗用网格地图
    _gridMap = GridMap::create(
        BattleConfig::GRID_WIDTH,
        BattleConfig::GRID_HEIGHT,
        BattleConfig::CELL_SIZE
    );
    _gridMap->setPosition(origin);
    this->addChild(_gridMap, 0);
    
    // 创建建筑层
    _buildingLayer = Node::create();
    _gridMap->addChild(_buildingLayer, 5);
    
    // 创建士兵层
    _soldierLayer = Node::create();
    _gridMap->addChild(_soldierLayer, 10);
    
    // 显示网格线（调试用）
    _gridMap->showGrid(true);
    
    CCLOG("[战斗场景] 网格地图初始化完成");
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
    // 在地图右侧放置敌方基地
    createEnemyBase(30, 12);
    
    // 放置一些防御塔
    createDefenseTower(25, 10, 1);  // 箭塔
    createDefenseTower(25, 14, 1);  // 箭塔
    createDefenseTower(27, 8, 2);   // 炮塔
    createDefenseTower(27, 16, 2);  // 炮塔
}

// ===================================================
// 创建敌方基地
// ===================================================

void BattleScene::createEnemyBase(int gridX, int gridY) {
    // 创建敌方基地配置
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
        
        // 标记格子占用
        _gridMap->occupyCell(gridX, gridY, 4, 4, base);
        
        _enemyBuildings.push_back(base);
        _totalBuildingCount++;
        
        CCLOG("[战斗场景] 敌方基地创建完成");
    }
}

// ===================================================
// 创建防御塔
// ===================================================

void BattleScene::createDefenseTower(int gridX, int gridY, int type) {
    // 创建防御塔配置
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
    } else {
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
        
        // 标记格子占用
        _gridMap->occupyCell(gridX, gridY, 3, 3, tower);
        
        _enemyBuildings.push_back(tower);
        _totalBuildingCount++;
        
        CCLOG("[战斗场景] 防御塔创建完成 (类型%d)", type);
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
    
    // 顶部UI背景
    auto topBg = LayerColor::create(
        Color4B(30, 30, 30, 220),
        visibleSize.width,
        BattleConfig::UI_TOP_HEIGHT
    );
    topBg->setPosition(origin.x, origin.y + visibleSize.height - BattleConfig::UI_TOP_HEIGHT);
    _uiLayer->addChild(topBg);
    
    // 计时器
    _timerLabel = Label::createWithTTF("3:00", "fonts/arial.ttf", 18);
    if (!_timerLabel) {
        _timerLabel = Label::createWithSystemFont("3:00", "Arial", 18);
    }
    _timerLabel->setPosition(Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 25
    ));
    _timerLabel->setColor(Color3B::WHITE);
    _uiLayer->addChild(_timerLabel);
    
    // 进度显示
    _progressLabel = Label::createWithTTF("0%", "fonts/arial.ttf", 14);
    if (!_progressLabel) {
        _progressLabel = Label::createWithSystemFont("0%", "Arial", 14);
    }
    _progressLabel->setPosition(Vec2(
        origin.x + visibleSize.width - 50,
        origin.y + visibleSize.height - 25
    ));
    _progressLabel->setColor(Color3B::YELLOW);
    _uiLayer->addChild(_progressLabel);
    
    // 退出按钮
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
    
    exitNode->setPosition(Vec2(origin.x + 40, origin.y + visibleSize.height - 25));
    _uiLayer->addChild(exitNode);
    
    auto exitBtn = Button::create();
    exitBtn->setContentSize(Size(btnWidth, btnHeight));
    exitBtn->setPosition(Vec2(origin.x + 40, origin.y + visibleSize.height - 25));
    exitBtn->addClickEventListener(CC_CALLBACK_1(BattleScene::onExitButton, this));
    _uiLayer->addChild(exitBtn);
    
    // 底部部署区域
    setupDeployArea();
}

// ===================================================
// 部署区域设置
// ===================================================

void BattleScene::setupDeployArea() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 底部UI背景
    auto bottomBg = LayerColor::create(
        Color4B(30, 30, 30, 220),
        visibleSize.width,
        BattleConfig::UI_BOTTOM_HEIGHT
    );
    bottomBg->setPosition(origin.x, origin.y);
    _uiLayer->addChild(bottomBg);
    
    // 部署区域容器
    _unitDeployArea = Node::create();
    _unitDeployArea->setPosition(Vec2(origin.x + 20, origin.y + BattleConfig::UI_BOTTOM_HEIGHT / 2));
    _uiLayer->addChild(_unitDeployArea);
    
    // 创建可部署兵种的按钮
    float xPos = 0;
    float btnSpacing = 70.0f;
    
    // 如果没有指定兵种，使用默认测试兵种
    if (_remainingUnits.empty()) {
        _remainingUnits[101] = 10;  // 10个Goblin
        _remainingUnits[102] = 5;   // 5个Barbarian
        _remainingUnits[103] = 8;   // 8个Archer
    }
    
    for (const auto& pair : _remainingUnits) {
        auto btn = createDeployButton(pair.first, pair.second, xPos);
        if (btn) {
            _unitDeployArea->addChild(btn);
        }
        xPos += btnSpacing;
    }
    
    // 部署提示
    auto tipLabel = Label::createWithTTF("Tap on map to deploy", "fonts/arial.ttf", 10);
    if (!tipLabel) {
        tipLabel = Label::createWithSystemFont("Tap on map to deploy", "Arial", 10);
    }
    tipLabel->setPosition(Vec2(
        origin.x + visibleSize.width - 80,
        origin.y + BattleConfig::UI_BOTTOM_HEIGHT / 2
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
    
    // 获取兵种配置
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
    
    // 兵种名称
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
    
    // 保存unitId以便部署时使用
    node->setTag(unitId);
    
    return node;
}

// ===================================================
// 部署士兵
// ===================================================

void BattleScene::deploySoldier(int unitId, const Vec2& position) {
    // 检查是否还有可部署的该类型兵种
    auto it = _remainingUnits.find(unitId);
    if (it == _remainingUnits.end() || it->second <= 0) {
        CCLOG("[战斗场景] 没有可部署的兵种: %d", unitId);
        return;
    }
    
    // 创建士兵
    auto soldier = UnitManager::getInstance()->spawnSoldier(unitId, position, 0);
    if (soldier) {
        _soldierLayer->addChild(soldier);
        _soldiers.push_back(soldier);
        
        // 减少剩余数量
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
    
    // 在地图上部署士兵（默认部署第一个有剩余的兵种）
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

// ===================================================
// 战斗逻辑更新
// ===================================================

void BattleScene::updateBattle(float dt) {
    // 检查被摧毁的建筑
    int destroyed = 0;
    for (auto& building : _enemyBuildings) {
        if (!building || !building->getParent()) {
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
    
    // 所有士兵阵亡且没有剩余可部署兵种 - 失败
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
    
    // 显示胜利信息
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
    
    // 显示失败信息
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
