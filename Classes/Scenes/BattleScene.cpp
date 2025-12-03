// BattleScene.cpp
// 战斗场景实现文件
// 功能: 实现进攻战斗的主要逻辑

#include "BattleScene.h"
#include "BaseScene.h"
#include "../Core/ResourceManager.h"
#include "../Buildings/Building.h"
#include "../Buildings/DefenseBuilding.h"
#include "../Buildings/ProductionBuilding.h"
#include "../Buildings/StorageBuilding.h"
#include "../Units/Unit.h"
#include "../Potions/Potion.h"
#include <algorithm>

// ==================== 场景创建 ====================

Scene* BattleScene::createScene() {
    return BattleScene::create();
}

bool BattleScene::init() {
    if (!Scene::init()) {
        return false;
    }

    // 初始化变量
    _totalBuildingHP = 0;
    _destroyedBuildingHP = 0;
    _destructionPercent = 0;
    _goldLooted = 0;
    _elixirLooted = 0;
    _battleStarted = false;
    _battleEnded = false;
    _isUnitSelected = false;
    _selectedUnitType = UnitType::BARBARIAN;
    _battleTime = 0;
    _maxBattleTime = 180.0f;  // 3分钟
    _cameraOffset = Vec2::ZERO;
    _isDragging = false;

    // 初始化可用兵种(测试用)
    _availableUnits[UnitType::BARBARIAN] = 20;
    _availableUnits[UnitType::ARCHER] = 15;
    _availableUnits[UnitType::GIANT] = 3;
    _availableUnits[UnitType::GOBLIN] = 10;

    // 创建地图层
    createMapLayer();

    // 创建敌方基地
    createEnemyBase();

    // 创建UI层
    createUILayer();

    // 添加触摸监听
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(false);
    touchListener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(BattleScene::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(BattleScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    // 启用帧更新
    scheduleUpdate();

    return true;
}

// ==================== 地图创建 ====================

void BattleScene::createMapLayer() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _mapLayer = Node::create();
    this->addChild(_mapLayer, Z_GROUND);

    // 创建地面背景
    auto ground = DrawNode::create();
    float mapWidth = MAP_WIDTH * GRID_SIZE;
    float mapHeight = MAP_HEIGHT * GRID_SIZE;

    // 绘制敌方基地草地(略带红色调表示敌方领地)
    ground->drawSolidRect(Vec2(0, 0), Vec2(mapWidth, mapHeight),
        Color4F(0.35f, 0.5f, 0.3f, 1.0f));

    _mapLayer->addChild(ground, 0);

    // 创建建筑层
    _buildingLayer = Node::create();
    _mapLayer->addChild(_buildingLayer, Z_BUILDING);

    // 创建单位层
    _unitLayer = Node::create();
    _mapLayer->addChild(_unitLayer, Z_UNIT);

    // 创建特效层
    _effectLayer = Node::create();
    _mapLayer->addChild(_effectLayer, Z_EFFECT);

    // 创建部署区域指示
    _deployZone = DrawNode::create();

    // 绘制可部署区域(地图边缘)
    Color4F deployColor = Color4F(0.0f, 1.0f, 0.0f, 0.15f);
    int borderSize = 5;  // 边缘5格可部署

    // 左边
    _deployZone->drawSolidRect(Vec2(0, 0),
        Vec2(borderSize * GRID_SIZE, mapHeight), deployColor);
    // 右边
    _deployZone->drawSolidRect(Vec2(mapWidth - borderSize * GRID_SIZE, 0),
        Vec2(mapWidth, mapHeight), deployColor);
    // 下边
    _deployZone->drawSolidRect(Vec2(borderSize * GRID_SIZE, 0),
        Vec2(mapWidth - borderSize * GRID_SIZE, borderSize * GRID_SIZE), deployColor);
    // 上边
    _deployZone->drawSolidRect(Vec2(borderSize * GRID_SIZE, mapHeight - borderSize * GRID_SIZE),
        Vec2(mapWidth - borderSize * GRID_SIZE, mapHeight), deployColor);

    _mapLayer->addChild(_deployZone, 1);

    // 居中地图
    _cameraOffset = Vec2(origin.x + visibleSize.width / 2 - mapWidth / 2,
        origin.y + visibleSize.height / 2 - mapHeight / 2 + 40);
    _mapLayer->setPosition(_cameraOffset);
}

// ==================== 敌方基地创建 ====================

void BattleScene::createEnemyBase() {
    // 创建敌方大本营
    auto townHall = Building::create(BuildingType::TOWN_HALL, Faction::ENEMY);
    townHall->setGridPosition(MAP_WIDTH / 2 - 2, MAP_HEIGHT / 2 - 2);
    _buildingLayer->addChild(townHall);
    _enemyBuildings.push_back(townHall);
    _totalBuildingHP += townHall->getMaxHP();

    // 创建敌方加农炮
    auto cannon1 = DefenseBuilding::create(BuildingType::CANNON, Faction::ENEMY);
    cannon1->setGridPosition(MAP_WIDTH / 2 - 6, MAP_HEIGHT / 2);
    _buildingLayer->addChild(cannon1);
    _enemyBuildings.push_back(cannon1);
    _defenseBuildings.push_back(cannon1);
    _totalBuildingHP += cannon1->getMaxHP();

    auto cannon2 = DefenseBuilding::create(BuildingType::CANNON, Faction::ENEMY);
    cannon2->setGridPosition(MAP_WIDTH / 2 + 4, MAP_HEIGHT / 2);
    _buildingLayer->addChild(cannon2);
    _enemyBuildings.push_back(cannon2);
    _defenseBuildings.push_back(cannon2);
    _totalBuildingHP += cannon2->getMaxHP();

    // 创建敌方箭塔
    auto archerTower = DefenseBuilding::create(BuildingType::ARCHER_TOWER, Faction::ENEMY);
    archerTower->setGridPosition(MAP_WIDTH / 2, MAP_HEIGHT / 2 + 5);
    _buildingLayer->addChild(archerTower);
    _enemyBuildings.push_back(archerTower);
    _defenseBuildings.push_back(archerTower);
    _totalBuildingHP += archerTower->getMaxHP();

    // 创建敌方金矿
    auto goldMine = ProductionBuilding::create(BuildingType::GOLD_MINE, Faction::ENEMY);
    goldMine->setGridPosition(MAP_WIDTH / 2 - 8, MAP_HEIGHT / 2 - 4);
    _buildingLayer->addChild(goldMine);
    _enemyBuildings.push_back(goldMine);
    _totalBuildingHP += goldMine->getMaxHP();

    // 创建敌方圣水收集器
    auto elixirCollector = ProductionBuilding::create(BuildingType::ELIXIR_COLLECTOR, Faction::ENEMY);
    elixirCollector->setGridPosition(MAP_WIDTH / 2 + 6, MAP_HEIGHT / 2 - 4);
    _buildingLayer->addChild(elixirCollector);
    _enemyBuildings.push_back(elixirCollector);
    _totalBuildingHP += elixirCollector->getMaxHP();

    // 创建敌方金库
    auto goldStorage = StorageBuilding::create(BuildingType::GOLD_STORAGE, Faction::ENEMY);
    goldStorage->setGridPosition(MAP_WIDTH / 2 - 5, MAP_HEIGHT / 2 - 6);
    _buildingLayer->addChild(goldStorage);
    _enemyBuildings.push_back(goldStorage);
    _totalBuildingHP += goldStorage->getMaxHP();

    // 创建敌方圣水库
    auto elixirStorage = StorageBuilding::create(BuildingType::ELIXIR_STORAGE, Faction::ENEMY);
    elixirStorage->setGridPosition(MAP_WIDTH / 2 + 3, MAP_HEIGHT / 2 - 6);
    _buildingLayer->addChild(elixirStorage);
    _enemyBuildings.push_back(elixirStorage);
    _totalBuildingHP += elixirStorage->getMaxHP();

    // 设置建筑摧毁回调
    for (auto building : _enemyBuildings) {
        building->setDestroyCallback([this](Building* b) {
            // 计算掠夺资源
            if (auto prod = dynamic_cast<ProductionBuilding*>(b)) {
                if (prod->getProductionType() == ResourceType::GOLD) {
                    _goldLooted += 100 + rand() % 200;
                }
                else {
                    _elixirLooted += 100 + rand() % 200;
                }
            }
            else if (auto storage = dynamic_cast<StorageBuilding*>(b)) {
                if (storage->getStorageType() == ResourceType::GOLD) {
                    _goldLooted += 200 + rand() % 400;
                }
                else {
                    _elixirLooted += 200 + rand() % 400;
                }
            }

            updateDestructionPercent();
            updateLootDisplay();
            checkVictoryCondition();
            });
    }
}

// ==================== UI创建 ====================

void BattleScene::createUILayer() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _uiLayer = Node::create();
    this->addChild(_uiLayer, Z_UI);

    // 创建兵种选择栏
    createUnitBar();

    // 创建战斗信息显示
    createBattleInfo();
}

void BattleScene::createUnitBar() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _unitBar = Node::create();

    // 统一常量
    const float barHeight = 85;
    const float buttonSize = 55;
    const float buttonSpacing = 85;
    const float startX = 55;
    const float buttonCenterY = barHeight / 2;

    // 背景
    auto bg = DrawNode::create();
    bg->drawSolidRect(Vec2(0, 0), Vec2(visibleSize.width, barHeight),
        Color4F(0.08f, 0.08f, 0.1f, 0.95f));
    bg->drawLine(Vec2(0, barHeight), Vec2(visibleSize.width, barHeight),
        Color4F(0.4f, 0.4f, 0.5f, 1.0f));
    bg->drawLine(Vec2(0, barHeight - 1), Vec2(visibleSize.width, barHeight - 1),
        Color4F(0.3f, 0.3f, 0.4f, 0.5f));
    _unitBar->addChild(bg);

    // 兵种按钮
    Vector<MenuItem*> unitButtons;

    struct UnitOption {
        UnitType type;
        std::string name;
        Color4F color;
    };

    std::vector<UnitOption> options = {
        {UnitType::BARBARIAN, "Barbarian", Color4F(0.8f, 0.6f, 0.2f, 1.0f)},
        {UnitType::ARCHER, "Archer", Color4F(0.2f, 0.8f, 0.2f, 1.0f)},
        {UnitType::GIANT, "Giant", Color4F(0.6f, 0.4f, 0.2f, 1.0f)},
        {UnitType::GOBLIN, "Goblin", Color4F(0.2f, 0.6f, 0.2f, 1.0f)}
    };

    for (size_t i = 0; i < options.size(); i++) {
        auto& opt = options[i];

        auto itemNode = Node::create();

        // 按钮背景框
        auto btnBg = DrawNode::create();
        btnBg->drawSolidRect(Vec2(-buttonSize / 2 - 5, -buttonSize / 2 - 12),
            Vec2(buttonSize / 2 + 5, buttonSize / 2 + 5),
            Color4F(0.15f, 0.15f, 0.2f, 0.9f));
        btnBg->drawRect(Vec2(-buttonSize / 2 - 5, -buttonSize / 2 - 12),
            Vec2(buttonSize / 2 + 5, buttonSize / 2 + 5),
            Color4F(0.4f, 0.4f, 0.5f, 1.0f));
        itemNode->addChild(btnBg, -1);

        // 图标
        auto icon = DrawNode::create();
        icon->drawSolidCircle(Vec2(0, 5), buttonSize / 2 - 8, 0, 20, opt.color);
        icon->drawCircle(Vec2(0, 5), buttonSize / 2 - 8, 0, 20, false, Color4F(0.0f, 0.0f, 0.0f, 0.5f));
        itemNode->addChild(icon);

        // 数量标签
        char countStr[16];
        int count = 0;
        if (_availableUnits.find(opt.type) != _availableUnits.end()) {
            count = _availableUnits[opt.type];
        }
        snprintf(countStr, sizeof(countStr), "x%d", count);
        auto countLabel = Label::createWithSystemFont(countStr, "Arial", 13);
        countLabel->setPosition(Vec2(0, -buttonSize / 2 + 2));
        countLabel->setColor(Color3B::WHITE);
        countLabel->setName("countLabel");
        itemNode->addChild(countLabel);

        // 创建按钮
        auto button = MenuItemLabel::create(Label::createWithSystemFont("", "Arial", 1),
            [this, opt](Ref* sender) {
                this->selectUnitType(opt.type);
            });
        button->setContentSize(Size(buttonSize + 10, buttonSize + 17));
        button->addChild(itemNode);

        button->setPosition(Vec2(startX + i * buttonSpacing, buttonCenterY));
        unitButtons.pushBack(button);
    }

    // 撤退按钮 - 右侧对齐
    const float retreatBtnWidth = 100;
    const float retreatBtnHeight = 40;
    auto retreatLabel = Label::createWithSystemFont("Retreat", "Arial", 18);
    retreatLabel->setColor(Color3B::WHITE);
    auto retreatButton = MenuItemLabel::create(retreatLabel, CC_CALLBACK_1(BattleScene::onRetreat, this));
    auto retreatBg = DrawNode::create();
    retreatBg->drawSolidRect(Vec2(-retreatBtnWidth / 2, -retreatBtnHeight / 2),
        Vec2(retreatBtnWidth / 2, retreatBtnHeight / 2),
        Color4F(0.6f, 0.2f, 0.2f, 1.0f));
    retreatBg->drawRect(Vec2(-retreatBtnWidth / 2, -retreatBtnHeight / 2),
        Vec2(retreatBtnWidth / 2, retreatBtnHeight / 2),
        Color4F(0.8f, 0.3f, 0.3f, 1.0f));
    retreatBg->setPosition(
        Vec2(
            retreatButton->getContentSize().width / 2,
            retreatButton->getContentSize().height / 2
        )
    );
    retreatButton->addChild(retreatBg, -1);
    retreatButton->setPosition(Vec2(visibleSize.width - 80, 40));
    unitButtons.pushBack(retreatButton);

    auto menu = Menu::createWithArray(unitButtons);
    menu->setPosition(Vec2::ZERO);
    _unitBar->addChild(menu);

    _unitBar->setPosition(origin);
    _uiLayer->addChild(_unitBar);
}

void BattleScene::createBattleInfo() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _battleInfoPanel = Node::create();

    // 统一常量
    const float panelWidth = 320;
    const float panelHeight = 65;
    const float margin = 10;

    // 背景
    auto bg = DrawNode::create();
    bg->drawSolidRect(Vec2(0, 0), Vec2(panelWidth, panelHeight),
        Color4F(0.08f, 0.08f, 0.12f, 0.9f));
    bg->drawRect(Vec2(0, 0), Vec2(panelWidth, panelHeight),
        Color4F(0.4f, 0.4f, 0.5f, 1.0f));
    bg->drawLine(Vec2(2, panelHeight - 1), Vec2(panelWidth - 2, panelHeight - 1),
        Color4F(0.5f, 0.5f, 0.6f, 0.5f));
    _battleInfoPanel->addChild(bg);

    // 摧毁百分比 - 左侧居中
    _percentLabel = Label::createWithSystemFont("0%", "Arial", 30);
    _percentLabel->setPosition(Vec2(55, panelHeight / 2));
    _percentLabel->setColor(Color3B(255, 215, 0));
    _battleInfoPanel->addChild(_percentLabel);

    // 星级显示(3颗星) - 中间
    const float starStartX = 115;
    const float starSpacing = 28;
    const float starY = panelHeight / 2;
    for (int i = 0; i < 3; i++) {
        auto star = DrawNode::create();
        float r = 11;
        Vec2 points[10];
        for (int j = 0; j < 10; j++) {
            float angle = M_PI / 2 + j * M_PI / 5;
            float radius = (j % 2 == 0) ? r : r * 0.4f;
            points[j] = Vec2(cos(angle) * radius, sin(angle) * radius);
        }
        star->drawPolygon(points, 10, Color4F(0.3f, 0.3f, 0.3f, 1.0f), 1, Color4F(0.5f, 0.5f, 0.5f, 1.0f));
        star->setPosition(Vec2(starStartX + i * starSpacing, starY));
        star->setName("star" + std::to_string(i));
        _battleInfoPanel->addChild(star);
    }

    // 掠夺资源显示 - 右侧
    const float lootStartX = 220;
    const float iconSize = 10;
    const float lootLabelOffsetX = 18;

    // 金币
    auto goldIcon = DrawNode::create();
    goldIcon->drawSolidCircle(Vec2(0, 0), iconSize, 0, 12, Color4F(1.0f, 0.84f, 0.0f, 1.0f));
    goldIcon->setPosition(Vec2(lootStartX, panelHeight / 2 + 12));
    _battleInfoPanel->addChild(goldIcon);

    _goldLootLabel = Label::createWithSystemFont("0", "Arial", 14);
    _goldLootLabel->setPosition(Vec2(lootStartX + lootLabelOffsetX, panelHeight / 2 + 12));
    _goldLootLabel->setAnchorPoint(Vec2(0, 0.5f));
    _goldLootLabel->setColor(Color3B(255, 215, 0));
    _battleInfoPanel->addChild(_goldLootLabel);

    // 圣水
    auto elixirIcon = DrawNode::create();
    elixirIcon->drawSolidCircle(Vec2(0, 0), iconSize, 0, 12, Color4F(0.6f, 0.2f, 0.8f, 1.0f));
    elixirIcon->setPosition(Vec2(lootStartX, panelHeight / 2 - 12));
    _battleInfoPanel->addChild(elixirIcon);

    _elixirLootLabel = Label::createWithSystemFont("0", "Arial", 14);
    _elixirLootLabel->setPosition(Vec2(lootStartX + lootLabelOffsetX, panelHeight / 2 - 12));
    _elixirLootLabel->setAnchorPoint(Vec2(0, 0.5f));
    _elixirLootLabel->setColor(Color3B(180, 80, 220));
    _battleInfoPanel->addChild(_elixirLootLabel);

    // 设置面板位置 - 左上角
    _battleInfoPanel->setPosition(Vec2(origin.x + margin, origin.y + visibleSize.height - panelHeight - margin));
    _uiLayer->addChild(_battleInfoPanel);

    // 时间显示 - 右上角，带背景
    auto timePanel = Node::create();
    auto timeBg = DrawNode::create();
    timeBg->drawSolidRect(Vec2(0, 0), Vec2(80, 35), Color4F(0.08f, 0.08f, 0.12f, 0.9f));
    timeBg->drawRect(Vec2(0, 0), Vec2(80, 35), Color4F(0.4f, 0.4f, 0.5f, 1.0f));
    timePanel->addChild(timeBg);

    _timeLabel = Label::createWithSystemFont("3:00", "Arial", 22);
    _timeLabel->setPosition(Vec2(40, 17.5f));
    _timeLabel->setColor(Color3B::WHITE);
    timePanel->addChild(_timeLabel);

    timePanel->setPosition(Vec2(origin.x + visibleSize.width - 80 - margin,
        origin.y + visibleSize.height - 35 - margin));
    _uiLayer->addChild(timePanel);
}

// ==================== 战斗操作 ====================

void BattleScene::deployUnit(UnitType type, const Vec2& position) {
    // 检查是否有可用兵种
    if (_availableUnits.find(type) == _availableUnits.end() || _availableUnits[type] <= 0) {
        return;
    }

    // 检查位置是否可部署
    if (!canDeployAt(position)) {
        return;
    }

    // 创建单位
    auto unit = Unit::create(type, Faction::PLAYER);
    if (!unit) return;

    // 设置位置
    unit->deploy(position);

    // 设置敌方建筑列表
    std::vector<Building*> enemyBuildings(_enemyBuildings.begin(), _enemyBuildings.end());
    unit->setBuildingList(enemyBuildings);

    // 添加到场景
    _unitLayer->addChild(unit);
    _playerUnits.push_back(unit);

    // 减少可用数量
    _availableUnits[type]--;
    updateRemainingUnits();

    // 开始战斗
    if (!_battleStarted) {
        startBattle();
    }

    // 设置防御建筑的敌人列表
    for (auto defense : _defenseBuildings) {
        defense->setEnemyList(_playerUnits);
    }
}

void BattleScene::usePotion(PotionType type, const Vec2& position) {
    // 创建药水
    auto potion = Potion::create(type, Faction::PLAYER);
    if (!potion) return;

    // 设置友军和敌军列表
    potion->setFriendlyUnits(_playerUnits);

    std::vector<Unit*> enemyUnits;  // 敌方没有可移动单位
    potion->setEnemyUnits(enemyUnits);

    std::vector<Building*> enemyBuildings(_enemyBuildings.begin(), _enemyBuildings.end());
    potion->setEnemyBuildings(enemyBuildings);

    // 使用药水
    _effectLayer->addChild(potion);
    potion->use(position);
}

void BattleScene::selectUnitType(UnitType type) {
    _selectedUnitType = type;
    _isUnitSelected = true;
}

void BattleScene::deselectUnit() {
    _isUnitSelected = false;
}

// ==================== 战斗逻辑 ====================

void BattleScene::startBattle() {
    _battleStarted = true;
    _battleTime = 0;
}

void BattleScene::updateBattle(float dt) {
    if (!_battleStarted || _battleEnded) return;

    // 更新战斗时间
    _battleTime += dt;

    // 更新时间显示
    int remainingTime = (int)(_maxBattleTime - _battleTime);
    if (remainingTime < 0) remainingTime = 0;
    int minutes = remainingTime / 60;
    int seconds = remainingTime % 60;
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%d:%02d", minutes, seconds);
    _timeLabel->setString(timeStr);

    // 检查时间是否用尽
    if (_battleTime >= _maxBattleTime) {
        endBattle(false);
        return;
    }

    // 移除死亡或无效的单位
    _playerUnits.erase(
        std::remove_if(_playerUnits.begin(), _playerUnits.end(),
            [](Unit* u) { return u == nullptr || u->isDead() || u->getParent() == nullptr; }),
        _playerUnits.end()
    );

    // 更新防御建筑的敌人列表（确保有效性）
    for (auto defense : _defenseBuildings) {
        if (defense && !defense->isDestroyed()) {
            defense->setEnemyList(_playerUnits);
        }
    }

    // 检查是否所有单位都已部署且全部死亡
    bool allDeployed = true;
    for (auto& pair : _availableUnits) {
        if (pair.second > 0) {
            allDeployed = false;
            break;
        }
    }

    if (allDeployed && _playerUnits.empty()) {
        endBattle(false);
    }
}

void BattleScene::checkVictoryCondition() {
    // 检查大本营是否被摧毁
    for (auto building : _enemyBuildings) {
        if (building->getBuildingType() == BuildingType::TOWN_HALL && building->isDestroyed()) {
            // 大本营被摧毁，立即胜利
            endBattle(true);
            return;
        }
    }

    // 检查摧毁率
    if (_destructionPercent >= 100.0f) {
        endBattle(true);
    }
}

void BattleScene::endBattle(bool victory) {
    _battleEnded = true;

    // 给予奖励
    if (victory || _destructionPercent >= 50.0f) {
        ResourceManager::getInstance()->addResource(ResourceType::GOLD, _goldLooted);
        ResourceManager::getInstance()->addResource(ResourceType::ELIXIR, _elixirLooted);
    }

    // 显示结算界面
    showResultPanel(victory);
}

void BattleScene::showResultPanel(bool victory) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _resultPanel = Node::create();

    // 背景遮罩
    auto mask = DrawNode::create();
    mask->drawSolidRect(Vec2(0, 0), Vec2(visibleSize.width, visibleSize.height),
        Color4F(0, 0, 0, 0.7f));
    _resultPanel->addChild(mask);

    // 面板
    float panelWidth = 400;
    float panelHeight = 300;
    float panelX = (visibleSize.width - panelWidth) / 2;
    float panelY = (visibleSize.height - panelHeight) / 2;

    auto panel = DrawNode::create();
    panel->drawSolidRect(Vec2(panelX, panelY), Vec2(panelX + panelWidth, panelY + panelHeight),
        Color4F(0.15f, 0.15f, 0.2f, 0.95f));
    panel->drawRect(Vec2(panelX, panelY), Vec2(panelX + panelWidth, panelY + panelHeight),
        Color4F(0.5f, 0.5f, 0.6f, 1.0f));
    _resultPanel->addChild(panel);

    // 结果标题
    std::string titleText = victory ? "Victory!" : "Battle Ended";
    auto titleLabel = Label::createWithSystemFont(titleText, "Arial", 36);
    titleLabel->setPosition(Vec2(visibleSize.width / 2, panelY + panelHeight - 50));
    titleLabel->setColor(victory ? Color3B(100, 255, 100) : Color3B(255, 200, 100));
    _resultPanel->addChild(titleLabel);

    // 摧毁率
    char percentStr[32];
    snprintf(percentStr, sizeof(percentStr), "Destruction: %.0f%%", _destructionPercent);
    auto percentLabel = Label::createWithSystemFont(percentStr, "Arial", 24);
    percentLabel->setPosition(Vec2(visibleSize.width / 2, panelY + panelHeight - 100));
    percentLabel->setColor(Color3B(255, 215, 0));
    _resultPanel->addChild(percentLabel);

    // 星级
    int stars = 0;
    if (_destructionPercent >= 50.0f) stars = 1;
    if (_destructionPercent >= 100.0f) stars = 3;

    // 检查大本营是否被摧毁
    for (auto building : _enemyBuildings) {
        if (building->getBuildingType() == BuildingType::TOWN_HALL && building->isDestroyed()) {
            stars = std::max(stars, 2);
            break;
        }
    }

    for (int i = 0; i < 3; i++) {
        auto star = DrawNode::create();
        float r = 20;
        Vec2 points[10];
        for (int j = 0; j < 10; j++) {
            float angle = M_PI / 2 + j * M_PI / 5;
            float radius = (j % 2 == 0) ? r : r * 0.4f;
            points[j] = Vec2(cos(angle) * radius, sin(angle) * radius);
        }
        Color4F starColor = (i < stars) ? Color4F(1.0f, 0.84f, 0.0f, 1.0f) : Color4F(0.3f, 0.3f, 0.3f, 1.0f);
        star->drawPolygon(points, 10, starColor, 1, Color4F(0.8f, 0.6f, 0.0f, 1.0f));
        star->setPosition(Vec2(visibleSize.width / 2 - 50 + i * 50, panelY + panelHeight - 150));
        _resultPanel->addChild(star);
    }

    // 掠夺资源
    char goldStr[32];
    snprintf(goldStr, sizeof(goldStr), "Gold: +%d", _goldLooted);
    auto goldLabel = Label::createWithSystemFont(goldStr, "Arial", 20);
    goldLabel->setPosition(Vec2(visibleSize.width / 2, panelY + panelHeight - 190));
    goldLabel->setColor(Color3B(255, 215, 0));
    _resultPanel->addChild(goldLabel);

    char elixirStr[32];
    snprintf(elixirStr, sizeof(elixirStr), "Elixir: +%d", _elixirLooted);
    auto elixirLabel = Label::createWithSystemFont(elixirStr, "Arial", 20);
    elixirLabel->setPosition(Vec2(visibleSize.width / 2, panelY + panelHeight - 220));
    elixirLabel->setColor(Color3B(180, 80, 220));
    _resultPanel->addChild(elixirLabel);

    // 返回按钮
    auto backLabel = Label::createWithSystemFont("Return to Base", "Arial", 22);
    backLabel->setColor(Color3B::WHITE);
    backLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    auto backButton = MenuItemLabel::create(backLabel, CC_CALLBACK_1(BattleScene::onBackToBase, this));
    auto backBg = DrawNode::create();
    backBg->drawSolidRect(Vec2(0, 0), Vec2(200, 50), Color4F(0.2f, 0.5f, 0.3f, 1.0f));
    backBg->drawRect(Vec2(0, 0), Vec2(200, 50), Color4F(0.4f, 0.8f, 0.5f, 1.0f));
    backBg->setPosition(backButton->getContentSize().width / 2 - 200 / 2, backButton->getContentSize().height / 2 - 50 / 2);
    backButton->addChild(backBg, -1);
    backButton->setPosition(Vec2(visibleSize.width / 2, panelY + 40));

    auto menu = Menu::create(backButton, nullptr);
    menu->setPosition(Vec2::ZERO);
    _resultPanel->addChild(menu);

    _resultPanel->setPosition(origin);
    _uiLayer->addChild(_resultPanel, 100);
}

// ==================== UI更新 ====================

void BattleScene::updateDestructionPercent() {
    int currentHP = 0;
    for (auto building : _enemyBuildings) {
        if (!building->isDestroyed()) {
            currentHP += building->getCurrentHP();
        }
    }

    _destroyedBuildingHP = _totalBuildingHP - currentHP;
    _destructionPercent = ((float)_destroyedBuildingHP / (float)_totalBuildingHP) * 100.0f;

    // 更新显示
    char percentStr[16];
    snprintf(percentStr, sizeof(percentStr), "%.0f%%", _destructionPercent);
    if (_percentLabel) {
        _percentLabel->setString(percentStr);
    }

    // 更新星级显示
    int stars = 0;
    if (_destructionPercent >= 50.0f) stars = 1;
    if (_destructionPercent >= 100.0f) stars = 3;

    // 检查大本营
    for (auto building : _enemyBuildings) {
        if (building->getBuildingType() == BuildingType::TOWN_HALL && building->isDestroyed()) {
            stars = std::max(stars, 2);
            break;
        }
    }

    for (int i = 0; i < 3; i++) {
        auto star = dynamic_cast<DrawNode*>(_battleInfoPanel->getChildByName("star" + std::to_string(i)));
        if (star) {
            star->clear();
            float r = 12;
            Vec2 points[10];
            for (int j = 0; j < 10; j++) {
                float angle = M_PI / 2 + j * M_PI / 5;
                float radius = (j % 2 == 0) ? r : r * 0.4f;
                points[j] = Vec2(cos(angle) * radius, sin(angle) * radius);
            }
            Color4F starColor = (i < stars) ? Color4F(1.0f, 0.84f, 0.0f, 1.0f) : Color4F(0.3f, 0.3f, 0.3f, 1.0f);
            star->drawPolygon(points, 10, starColor, 1, Color4F(0.8f, 0.6f, 0.0f, 1.0f));
        }
    }
}

void BattleScene::updateLootDisplay() {
    char goldStr[16];
    snprintf(goldStr, sizeof(goldStr), "%d", _goldLooted);
    if (_goldLootLabel) _goldLootLabel->setString(goldStr);

    char elixirStr[16];
    snprintf(elixirStr, sizeof(elixirStr), "%d", _elixirLooted);
    if (_elixirLootLabel) _elixirLootLabel->setString(elixirStr);
}

void BattleScene::updateRemainingUnits() {
    // 更新兵种栏的数量显示
    if (!_unitBar) return;

    auto menu = _unitBar->getChildren().at(1);
    if (!menu) return;

    int index = 0;
    UnitType types[] = { UnitType::BARBARIAN, UnitType::ARCHER, UnitType::GIANT, UnitType::GOBLIN };

    for (auto& child : menu->getChildren()) {
        if (auto item = dynamic_cast<MenuItem*>(child)) {
            if (index < 4) {
                auto itemNode = item->getChildren().at(0);
                if (itemNode) {
                    auto countLabel = dynamic_cast<Label*>(itemNode->getChildByName("countLabel"));
                    if (countLabel) {
                        int count = 0;
                        if (_availableUnits.find(types[index]) != _availableUnits.end()) {
                            count = _availableUnits[types[index]];
                        }
                        char countStr[16];
                        snprintf(countStr, sizeof(countStr), "x%d", count);
                        countLabel->setString(countStr);
                    }
                }
            }
            index++;
        }
    }
}

// ==================== 输入处理 ====================

bool BattleScene::onTouchBegan(Touch* touch, Event* event) {
    Vec2 touchPos = touch->getLocation();
    _lastTouchPos = touchPos;
    _isDragging = false;

    // 检查是否在UI区域
    if (touchPos.y < 80) {  // 兵种栏区域
        return false;
    }

    return true;
}

void BattleScene::onTouchMoved(Touch* touch, Event* event) {
    Vec2 touchPos = touch->getLocation();
    Vec2 delta = touchPos - _lastTouchPos;

    if (delta.length() > 5) {
        _isDragging = true;
    }

    if (_isDragging && !_isUnitSelected) {
        // 拖动地图
        _cameraOffset += delta;
        _mapLayer->setPosition(_cameraOffset);
    }

    _lastTouchPos = touchPos;
}

void BattleScene::onTouchEnded(Touch* touch, Event* event) {
    if (!_isDragging && _isUnitSelected) {
        // 部署单位
        Vec2 touchPos = touch->getLocation();
        Vec2 mapPos = screenToMap(touchPos);

        deployUnit(_selectedUnitType, mapPos);
    }

    _isDragging = false;
}

// ==================== UI回调 ====================

void BattleScene::onRetreat(Ref* sender) {
    endBattle(false);
}

void BattleScene::onBackToBase(Ref* sender) {
    auto scene = BaseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
}

// ==================== 辅助函数 ====================

bool BattleScene::canDeployAt(const Vec2& position) {
    // 检查是否在部署区域内
    // 部署区域是地图边缘的一圈区域
    float mapWidth = MAP_WIDTH * GRID_SIZE;
    float mapHeight = MAP_HEIGHT * GRID_SIZE;
    int borderSize = 5 * GRID_SIZE;

    // 检查是否在地图范围内
    if (position.x < 0 || position.x > mapWidth ||
        position.y < 0 || position.y > mapHeight) {
        return false;
    }

    // 检查是否在边缘部署区域
    // 左边缘
    if (position.x < borderSize) return true;
    // 右边缘
    if (position.x > mapWidth - borderSize) return true;
    // 下边缘
    if (position.y < borderSize) return true;
    // 上边缘
    if (position.y > mapHeight - borderSize) return true;

    // 不在任何边缘区域，不可部署
    return false;
}

Vec2 BattleScene::screenToMap(const Vec2& screenPos) {
    return screenPos - _cameraOffset;
}

// ==================== 帧更新 ====================

void BattleScene::update(float dt) {
    updateBattle(dt);
}
