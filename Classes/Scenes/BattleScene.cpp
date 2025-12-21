/**
 * @file BattleScene.cpp
 * @brief ս������ʵ��
 *
 * ʵ��ս��ϵͳ�ĺ����߼���������
 * - �ؿ���ʼ���͵з���������
 * - ���ʿ������
 * - ս���߼�����
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
// ��������
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
// ��ʼ��
// ===================================================

bool BattleScene::init() {
    if (!Scene::init()) {
        return false;
    }

    CCLOG("[ս������] ��ʼ���ؿ� %d", _levelId);

    // ��ʼ�������
    initGridMap();
    initLevel();
    initUI();
    initTouchListener();

    // ��ʼ��ʣ��ɲ������
    _remainingUnits = _deployableUnits;

    // ���ø���
    this->scheduleUpdate();

    return true;
}

// ===================================================
// ===================================================
// 网格地图初始化
// ===================================================

void BattleScene::initGridMap() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 创建战斗场景网格地图
    _gridMap = GridMap::create(
        BattleConfig::GRID_WIDTH,
        BattleConfig::GRID_HEIGHT,
        BattleConfig::CELL_SIZE
    );
    
    // 设置地图位置，使地图在可见区域内居中显示
    float mapWidth = BattleConfig::GRID_WIDTH * BattleConfig::CELL_SIZE;
    float mapHeight = BattleConfig::GRID_HEIGHT * BattleConfig::CELL_SIZE;
    
    // 考虑UI区域的偏移（顶部和底部UI）
    float availableHeight = visibleSize.height - BattleConfig::UI_TOP_HEIGHT - BattleConfig::UI_BOTTOM_HEIGHT;
    float offsetX = origin.x + (visibleSize.width - mapWidth) / 2;
    float offsetY = origin.y + BattleConfig::UI_BOTTOM_HEIGHT + (availableHeight - mapHeight) / 2;
    
    // 确保地图至少显示在负数区
    if (offsetX < origin.x) offsetX = origin.x;
    if (offsetY < origin.y + BattleConfig::UI_BOTTOM_HEIGHT) offsetY = origin.y + BattleConfig::UI_BOTTOM_HEIGHT;
    
    _gridMap->setPosition(Vec2(offsetX, offsetY));
    this->addChild(_gridMap, 0);

    // 创建草地背景（在gridMap内部）
    auto bgColor = LayerColor::create(Color4B(60, 100, 60, 255), mapWidth, mapHeight);
    _gridMap->addChild(bgColor, -2);

    // 创建建筑层
    _buildingLayer = Node::create();
    _gridMap->addChild(_buildingLayer, 5);

    // 创建士兵层
    _soldierLayer = Node::create();
    _gridMap->addChild(_soldierLayer, 10);

    // 显示网格线（调试用）
    _gridMap->showGrid(true);

    CCLOG("[战斗场景] 网格地图初始化完成，地图将居中显示");
}

// ===================================================
// �ؿ���ʼ��
// ===================================================

void BattleScene::initLevel() {
    // ���ݹؿ�ID������ͬ�Ĺؿ�
    switch (_levelId) {
    case 1:
    default:
        createLevel1();
        break;
    }

    CCLOG("[ս������] �ؿ� %d ��ʼ����ɣ��� %d ������", _levelId, _totalBuildingCount);
}

// ===================================================
// ������1�� - �򵥲��Թؿ�
// ===================================================

void BattleScene::createLevel1() {
    // �ڵ�ͼ���Ҳ���õз����أ�40x30����ȷ��4x4�����������ڣ�
    // ����λ����Ҫ�����㹻�ռ䣺�ұ߽�Ϊ40���������XΪ36
    createEnemyBase(28, 13);

    // ����һЩ��������3x3������Ҫȷ���������ڣ�
    createDefenseTower(22, 11, 1);  // ����
    createDefenseTower(22, 17, 1);  // ����
    createDefenseTower(24, 8, 2);   // ����
    createDefenseTower(24, 20, 2);  // ����
}

// ===================================================
// �����з�����
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

    // 顶部UI区域
    auto topBg = LayerColor::create(
        Color4B(30, 30, 30, 220),
        visibleSize.width,
        BattleConfig::UI_TOP_HEIGHT
    );
    topBg->setPosition(origin.x, origin.y + visibleSize.height - BattleConfig::UI_TOP_HEIGHT);
    _uiLayer->addChild(topBg);

    // ��ʱ��
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

    // ������ʾ
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

    // 退出按钮 - 使用透明Button确保点击可靠
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

    float btnX = origin.x + 40;
    float btnY = origin.y + visibleSize.height - 25;
    exitNode->setPosition(Vec2(btnX, btnY));
    _uiLayer->addChild(exitNode);

    // 创建透明Button覆盖在节点上，确保点击可靠
    auto exitBtn = Button::create();
    exitBtn->setContentSize(Size(btnWidth, btnHeight));
    exitBtn->setScale9Enabled(true);
    exitBtn->setPosition(Vec2(btnX, btnY));
    exitBtn->addClickEventListener([this](Ref* sender) {
        CCLOG("[战斗场景] 点击退出按钮");
        this->onExitButton(sender);
    });
    _uiLayer->addChild(exitBtn, 10);

    // 底部部署区域
    setupDeployArea();
}

// ===================================================
// ������������
// ===================================================

void BattleScene::setupDeployArea() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // �ײ�UI����
    auto bottomBg = LayerColor::create(
        Color4B(30, 30, 30, 220),
        visibleSize.width,
        BattleConfig::UI_BOTTOM_HEIGHT
    );
    bottomBg->setPosition(origin.x, origin.y);
    _uiLayer->addChild(bottomBg);

    // ������������
    _unitDeployArea = Node::create();
    _unitDeployArea->setPosition(Vec2(origin.x + 20, origin.y + BattleConfig::UI_BOTTOM_HEIGHT / 2));
    _uiLayer->addChild(_unitDeployArea);

    // �����ɲ�����ֵİ�ť
    float xPos = 0;
    float btnSpacing = 70.0f;

    // ���û��ָ�����֣�ʹ��Ĭ�ϲ��Ա���
    if (_remainingUnits.empty()) {
        _remainingUnits[101] = 10;  // 10��Goblin
        _remainingUnits[102] = 5;   // 5��Barbarian
        _remainingUnits[103] = 8;   // 8��Archer
    }

    for (const auto& pair : _remainingUnits) {
        auto btn = createDeployButton(pair.first, pair.second, xPos);
        if (btn) {
            _unitDeployArea->addChild(btn);
        }
        xPos += btnSpacing;
    }

    // ������ʾ
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
// ��������ť
// ===================================================

Node* BattleScene::createDeployButton(int unitId, int count, float x) {
    auto node = Node::create();
    node->setPosition(Vec2(x, 0));

    float btnSize = 50.0f;

    // ��ȡ��������
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    std::string unitName = config ? config->name.substr(0, 3) : "???";

    // ��ť����
    auto bg = LayerColor::create(Color4B(50, 70, 50, 255), btnSize, btnSize);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    node->addChild(bg);

    // �߿�
    auto border = DrawNode::create();
    border->drawRect(Vec2(-btnSize / 2, -btnSize / 2), Vec2(btnSize / 2, btnSize / 2), Color4F::WHITE);
    node->addChild(border, 1);

    // ��������
    auto nameLabel = Label::createWithTTF(unitName, "fonts/arial.ttf", 10);
    if (!nameLabel) {
        nameLabel = Label::createWithSystemFont(unitName, "Arial", 10);
    }
    nameLabel->setPosition(Vec2(0, 8));
    nameLabel->setColor(Color3B::WHITE);
    node->addChild(nameLabel, 2);

    // ����
    auto countLabel = Label::createWithTTF("x" + std::to_string(count), "fonts/arial.ttf", 10);
    if (!countLabel) {
        countLabel = Label::createWithSystemFont("x" + std::to_string(count), "Arial", 10);
    }
    countLabel->setPosition(Vec2(0, -10));
    countLabel->setColor(Color3B::YELLOW);
    countLabel->setName("countLabel");
    node->addChild(countLabel, 2);

    // ����unitId�Ա㲿��ʱʹ��
    node->setTag(unitId);

    return node;
}

// ===================================================
// ����ʿ��
// ===================================================

void BattleScene::deploySoldier(int unitId, const Vec2& position) {
    // ����Ƿ��пɲ���ĸ����ͱ���
    auto it = _remainingUnits.find(unitId);
    if (it == _remainingUnits.end() || it->second <= 0) {
        CCLOG("[ս������] û�пɲ���ı���: %d", unitId);
        return;
    }

    // ����ʿ��
    auto soldier = UnitManager::getInstance()->spawnSoldier(unitId, position, 0);
    if (soldier) {
        _soldierLayer->addChild(soldier);
        _soldiers.push_back(soldier);

        // ����ʣ������
        it->second--;

        CCLOG("[ս������] ����ʿ��: %d ��λ�� (%.1f, %.1f), ʣ�� %d",
            unitId, position.x, position.y, it->second);

        // ����UI
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
// �����¼���ʼ��
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

    // ����Ƿ��ڲ��������ڣ��ײ���
    if (touchPos.y < origin.y + BattleConfig::UI_BOTTOM_HEIGHT) {
        return false;
    }

    // ����Ƿ��ڶ���UI����
    if (touchPos.y > origin.y + visibleSize.height - BattleConfig::UI_TOP_HEIGHT) {
        return false;
    }

    // �ڵ�ͼ�ϲ���ʿ����Ĭ�ϲ����һ����ʣ��ı��֣�
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
// ÿ֡����
// ===================================================

void BattleScene::update(float dt) {
    if (_battleEnded) return;

    // ����ս��ʱ��
    _battleTime += dt;

    // ���¼�ʱ����ʾ
    float remainingTime = std::max(0.0f, BattleConfig::BATTLE_TIME_LIMIT - _battleTime);
    int minutes = static_cast<int>(remainingTime) / 60;
    int seconds = static_cast<int>(remainingTime) % 60;
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%d:%02d", minutes, seconds);
    if (_timerLabel) {
        _timerLabel->setString(timeStr);
    }

    // ����ս���߼�
    updateBattle(dt);

    // ���ս������
    checkBattleEnd();
}

// ===================================================
// ս���߼�����
// ===================================================

void BattleScene::updateBattle(float dt) {
    // ��鱻�ݻٵĽ���
    int destroyed = 0;
    for (auto& building : _enemyBuildings) {
        if (!building || !building->getParent()) {
            destroyed++;
        }
    }
    _destroyedBuildingCount = destroyed;

    // ���½�����ʾ
    int progress = _totalBuildingCount > 0 ?
        (destroyed * 100 / _totalBuildingCount) : 0;
    char progressStr[16];
    snprintf(progressStr, sizeof(progressStr), "%d%%", progress);
    if (_progressLabel) {
        _progressLabel->setString(progressStr);
    }
}

// ===================================================
// ���ս������
// ===================================================

void BattleScene::checkBattleEnd() {
    // ���н������ݻ� - ʤ��
    if (_destroyedBuildingCount >= _totalBuildingCount) {
        onBattleWin();
        return;
    }

    // ʱ��ľ� - ʧ��
    if (_battleTime >= BattleConfig::BATTLE_TIME_LIMIT) {
        onBattleLose();
        return;
    }

    // ����ʿ��������û��ʣ��ɲ������ - ʧ��
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
// ս��ʤ��
// ===================================================

void BattleScene::onBattleWin() {
    _battleEnded = true;
    CCLOG("[ս������] ս��ʤ����");

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // ��ʾʤ����Ϣ
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

    // 3��󷵻�
    this->scheduleOnce([](float dt) {
        auto scene = BaseScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        }, 3.0f, "return_to_base");
}

// ===================================================
// ս��ʧ��
// ===================================================

void BattleScene::onBattleLose() {
    _battleEnded = true;
    CCLOG("[ս������] ս��ʧ�ܣ�");

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // ��ʾʧ����Ϣ
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

    // 3��󷵻�
    this->scheduleOnce([](float dt) {
        auto scene = BaseScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        }, 3.0f, "return_to_base");
}

// ===================================================
// �˳���ť�ص�
// ===================================================

void BattleScene::onExitButton(Ref* sender) {
    CCLOG("[ս������] �˳�ս��");

    auto scene = BaseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}