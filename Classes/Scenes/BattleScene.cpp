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
#include "Core/Core.h"
#include "Save/SaveManager.h"
#include "Buildings/BuildingManager.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Buildings/StorageBuilding.h"
#include "Buildings/Trap.h"
#include "Soldier/UnitManager.h"
#include "Utils/AnimationUtils.h"
#include "Utils/AudioManager.h"
#include "Utils/NodeUtils.h"
#include <algorithm>
#include <cmath>

USING_NS_CC;

namespace {
constexpr float kHoverPanelPaddingX = 16.0f;
constexpr float kHoverPanelPaddingY = 14.0f;
constexpr float kHoverPanelMinWidth = 260.0f;
constexpr float kHoverPanelMinHeight = 140.0f;
constexpr float kHoverPanelOffsetX = 16.0f;
constexpr int kMaxLevelId = 12;
constexpr int kDefenseLevelOffset = 100;
constexpr int kDefenseMaxLevelId = 6;
constexpr int kEnemyMaxBuildingLevel = 2;
constexpr int kKingUnitId = 1007;
constexpr int kTowerArrow = 1;
constexpr int kTowerBoom = 2;
constexpr int kTowerDoubleBoom = 3;
constexpr int kTowerMagic = 4;
constexpr int kTowerFire = 5;

int resolveAttackTowerLevel(int levelId) {
    if (levelId >= 7) {
        return kEnemyMaxBuildingLevel;
    }
    if (levelId >= 4) {
        return 1;
    }
    return 0;
}

cocos2d::Vec2 getHoverAnchorWorldPos(cocos2d::Node* building) {
    if (!building) {
        return cocos2d::Vec2::ZERO;
    }
    const auto* bodySprite = NodeUtils::findBodySprite(building);
    cocos2d::Rect localBounds = bodySprite ? bodySprite->getBoundingBox() : building->getBoundingBox();
    if (localBounds.size.width <= 0.0f || localBounds.size.height <= 0.0f) {
        auto* parent = building->getParent();
        return parent ? parent->convertToWorldSpace(building->getPosition()) : building->getPosition();
    }
    cocos2d::Vec2 localRightCenter(localBounds.getMaxX(), localBounds.getMidY());
    return building->convertToWorldSpace(localRightCenter);
}

cocos2d::Rect getBuildingWorldRect(cocos2d::Node* building) {
    if (!building) {
        return cocos2d::Rect::ZERO;
    }
    const auto* bodySprite = NodeUtils::findBodySprite(building);
    if (bodySprite) {
        cocos2d::Rect localRect = bodySprite->getBoundingBox();
        cocos2d::Vec2 worldBL = building->convertToWorldSpace(localRect.origin);
        cocos2d::Vec2 worldTR = building->convertToWorldSpace(
            localRect.origin + cocos2d::Vec2(localRect.size.width, localRect.size.height));
        float minX = std::min(worldBL.x, worldTR.x);
        float minY = std::min(worldBL.y, worldTR.y);
        float maxX = std::max(worldBL.x, worldTR.x);
        float maxY = std::max(worldBL.y, worldTR.y);
        return cocos2d::Rect(minX, minY, maxX - minX, maxY - minY);
    }
    auto* parent = building->getParent();
    if (parent) {
        cocos2d::Rect localRect = building->getBoundingBox();
        cocos2d::Vec2 worldBL = parent->convertToWorldSpace(localRect.origin);
        cocos2d::Vec2 worldTR = parent->convertToWorldSpace(
            localRect.origin + cocos2d::Vec2(localRect.size.width, localRect.size.height));
        float minX = std::min(worldBL.x, worldTR.x);
        float minY = std::min(worldBL.y, worldTR.y);
        float maxX = std::max(worldBL.x, worldTR.x);
        float maxY = std::max(worldBL.y, worldTR.y);
        return cocos2d::Rect(minX, minY, maxX - minX, maxY - minY);
    }
    return cocos2d::Rect::ZERO;
}

cocos2d::Rect getHoverPanelWorldRect(const cocos2d::Node* panel,
                                    const cocos2d::LayerColor* bg) {
    if (!panel || !bg) {
        return cocos2d::Rect::ZERO;
    }
    cocos2d::Size panelSize = bg->getContentSize();
    cocos2d::Vec2 panelPos = panel->getPosition();
    return cocos2d::Rect(panelPos.x,
                         panelPos.y - panelSize.height * 0.5f,
                         panelSize.width,
                         panelSize.height);
}

cocos2d::Rect getHoverKeepRect(cocos2d::Node* building,
                              const cocos2d::Node* panel,
                              const cocos2d::LayerColor* bg) {
    cocos2d::Rect buildingRect = getBuildingWorldRect(building);
    cocos2d::Rect panelRect = getHoverPanelWorldRect(panel, bg);
    if (buildingRect.size.width <= 0.0f || buildingRect.size.height <= 0.0f) {
        return panelRect;
    }
    if (panelRect.size.width <= 0.0f || panelRect.size.height <= 0.0f) {
        return buildingRect;
    }
    float minX = std::min(buildingRect.getMinX(), panelRect.getMinX());
    float minY = std::min(buildingRect.getMinY(), panelRect.getMinY());
    float maxX = std::max(buildingRect.getMaxX(), panelRect.getMaxX());
    float maxY = std::max(buildingRect.getMaxY(), panelRect.getMaxY());
    return cocos2d::Rect(minX, minY, maxX - minX, maxY - minY);
}

} // namespace

// ===================================================
// 场景创建
// ===================================================

Scene* BattleScene::createScene(int levelId, const std::map<int, int>& units, bool useDefaultUnits, bool defenseMode) {
    auto scene = new (std::nothrow) BattleScene();
    if (scene) {
        scene->setLevelId(levelId);
        scene->setDeployableUnits(units);
        scene->_allowDefaultUnits = useDefaultUnits;
        scene->setBattleMode(defenseMode ? BattleMode::Defense : BattleMode::Attack);
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

    // 切关时先清空旧战斗引用，避免伤害残留
    Soldier::setEnemyBuildings(nullptr);
    DefenceBuilding::setEnemySoldiers(nullptr);
    TrapBase::setEnemySoldiers(nullptr);

    _soldiers.clear();
    _enemyBuildings.clear();
    _enemyBase = nullptr;
    _enemyBaseDestroyed = false;
    _battleTime = 0.0f;
    _battleEnded = false;
    _destroyedBuildingCount = 0;
    _totalBuildingCount = 0;
    _totalDeployedCount = 0;
    _deadSoldierCount = 0;
    _resultLayer = nullptr;

    CCLOG("[战斗场景] 初始化关卡 %d", _levelId);
    BuildingManager::getInstance()->loadConfigs();

    // 确保兵种配置已加载（避免直接进入战斗时配置缺失）
    if (UnitManager::getInstance()->getAllUnitIds().empty()) {
        UnitManager::getInstance()->loadConfig("res/units_config.json");
    }

    // 初始化剩余可部署单位
    if (_battleMode == BattleMode::Defense) {
        _remainingUnits.clear();
    }
    else if (_deployableUnits.empty() && _allowDefaultUnits) {
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
    TrapBase::setEnemySoldiers(&_soldiers);
    initUI();
    initTouchListener();
    initHoverInfo();
    AudioManager::playBattleBgm(getRewardLevel());

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

    auto bgColor = LayerColor::create(Color4B(50, 80, 50, 255), mapWidth, mapHeight);
    _gridMap->addChild(bgColor, -3);

    auto tileBg = Sprite::create("grass/grass_0000.png");
    if (tileBg) {
        auto texture = tileBg->getTexture();
        if (texture) {
            Texture2D::TexParams params = { GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT };
            texture->setTexParameters(params);
        }
        tileBg->setAnchorPoint(Vec2::ZERO);
        tileBg->setPosition(Vec2::ZERO);
        tileBg->setTextureRect(Rect(0, 0, mapWidth, mapHeight));
        tileBg->setOpacity(200);
        _gridMap->addChild(tileBg, -2);
    }

    // 部署范围提示
    if (_battleMode != BattleMode::Defense) {
        setupDeployRangeHint();
    }

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
    if (_battleMode == BattleMode::Defense) {
        int defenseId = getDefenseLevelIndex();
        switch (defenseId) {
        case 1:
            createDefenseLevel1();
            break;
        case 2:
            createDefenseLevel2();
            break;
        case 3:
            createDefenseLevel3();
            break;
        case 4:
            createDefenseLevel4();
            break;
        case 5:
            createDefenseLevel5();
            break;
        case 6:
            createDefenseLevel6();
            break;
        default:
            createDefenseLevel1();
            break;
        }

        CCLOG("[战斗场景] 防守关卡 %d 初始化完成，共 %d 个建筑", defenseId, _totalBuildingCount);
        return;
    }

    // 根据关卡ID创建不同的关卡
    switch (_levelId) {
    case 1:
        createLevel1();
        break;
    case 2:
        createLevel2();
        break;
    case 3:
        createLevel3();
        break;
    case 4:
        createLevel4();
        break;
    case 5:
        createLevel5();
        break;
    case 6:
        createLevel6();
        break;
    case 7:
        createLevel7();
        break;
    case 8:
        createLevel8();
        break;
    case 9:
        createLevel9();
        break;
    case 10:
        createLevel10();
        break;
    case 11:
        createLevel11();
        break;
    case 12:
        createLevel12();
        break;
    default:
        createLevel1();
        break;
    }

    CCLOG("[战斗场景] 关卡 %d 初始化完成，共 %d 个建筑", _levelId, _totalBuildingCount);
}

// ===================================================
// 创建第1关 - 入门试炼：双箭护卫
// ===================================================

void BattleScene::createLevel1() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(16, 8, kTowerArrow, towerLevel);
    createDefenseTower(16, 18, kTowerArrow, towerLevel);

    for (int x = 10; x <= 14; ++x) {
        createSpikeTrap(x, 13);
    }
    createSnapTrap(12, 10);
}

// ===================================================
// 创建第2关 - 交叉火力走廊
// ===================================================

void BattleScene::createLevel2() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(14, 6, kTowerArrow, towerLevel);
    createDefenseTower(14, 22, kTowerArrow, towerLevel);
    createDefenseTower(18, 14, kTowerBoom, towerLevel);

    for (int x = 8; x <= 18; ++x) {
        createSpikeTrap(x, 10);
        createSpikeTrap(x, 18);
    }
    createSnapTrap(10, 12);
    createSnapTrap(10, 16);
}

// ===================================================
// 创建第3关 - 陷阱方阵
// ===================================================

void BattleScene::createLevel3() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(12, 8, kTowerArrow, towerLevel);
    createDefenseTower(12, 20, kTowerArrow, towerLevel);
    createDefenseTower(18, 12, kTowerDoubleBoom, towerLevel);
    createDefenseTower(20, 20, kTowerMagic, towerLevel);

    for (int x = 8; x <= 16; ++x) {
        createSpikeTrap(x, 12);
        createSpikeTrap(x, 18);
    }
    for (int y = 12; y <= 18; ++y) {
        createSpikeTrap(8, y);
        createSpikeTrap(16, y);
    }
    createSnapTrap(10, 14);
    createSnapTrap(14, 16);
}

// ===================================================
// 创建第4关 - 断层火力
// ===================================================

void BattleScene::createLevel4() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(10, 14, kTowerArrow, towerLevel);
    createDefenseTower(16, 6, kTowerBoom, towerLevel);
    createDefenseTower(16, 20, kTowerBoom, towerLevel);
    createDefenseTower(20, 14, kTowerMagic, towerLevel);

    for (int x = 8; x <= 20; ++x) {
        if (x == 14) {
            continue;
        }
        createSpikeTrap(x, 10);
        createSpikeTrap(x, 18);
    }
    createSnapTrap(14, 12);
    createSnapTrap(14, 16);
}

// ===================================================
// 创建第5关 - 双管压制
// ===================================================

void BattleScene::createLevel5() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(12, 6, kTowerArrow, towerLevel);
    createDefenseTower(12, 22, kTowerArrow, towerLevel);
    createDefenseTower(18, 8, kTowerDoubleBoom, towerLevel);
    createDefenseTower(18, 18, kTowerDoubleBoom, towerLevel);
    createDefenseTower(22, 14, kTowerFire, towerLevel);

    for (int x = 8; x <= 22; ++x) {
        createSpikeTrap(x, 11);
        createSpikeTrap(x, 17);
    }
    for (int y = 12; y <= 16; ++y) {
        createSpikeTrap(10, y);
        createSpikeTrap(20, y);
    }
    createSnapTrap(14, 14);
}

// ===================================================
// 创建第6关 - 火焰走廊
// ===================================================

void BattleScene::createLevel6() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(10, 14, kTowerBoom, towerLevel);
    createDefenseTower(16, 8, kTowerFire, towerLevel);
    createDefenseTower(16, 18, kTowerFire, towerLevel);
    createDefenseTower(20, 14, kTowerMagic, towerLevel);

    for (int x = 6; x <= 20; ++x) {
        if (x == 14) {
            continue;
        }
        createSpikeTrap(x, 12);
    }
    for (int x = 8; x <= 18; ++x) {
        if (x == 14) {
            continue;
        }
        if (x >= 10 && x <= 12) {
            continue;
        }
        createSpikeTrap(x, 16);
    }
    createSnapTrap(14, 12);
    createSnapTrap(14, 18);
}

// ===================================================
// 创建第7关 - 极限交叉火力
// ===================================================

void BattleScene::createLevel7() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(6, 6, kTowerArrow, towerLevel);
    createDefenseTower(6, 22, kTowerArrow, towerLevel);
    createDefenseTower(12, 6, kTowerBoom, towerLevel);
    createDefenseTower(12, 22, kTowerBoom, towerLevel);
    createDefenseTower(18, 12, kTowerDoubleBoom, towerLevel);
    createDefenseTower(20, 6, kTowerMagic, towerLevel);
    createDefenseTower(20, 22, kTowerMagic, towerLevel);
    createDefenseTower(22, 14, kTowerFire, towerLevel);

    for (int x = 6; x <= 24; ++x) {
        createSpikeTrap(x, 10);
        createSpikeTrap(x, 18);
    }
    for (int y = 11; y <= 17; ++y) {
        createSpikeTrap(10, y);
        createSpikeTrap(16, y);
    }
    createSnapTrap(14, 14);
    createSnapTrap(18, 16);
}

// ===================================================
// 创建第8关 - 交错火网
// ===================================================

void BattleScene::createLevel8() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(8, 14, kTowerArrow, towerLevel);
    createDefenseTower(12, 6, kTowerMagic, towerLevel);
    createDefenseTower(12, 22, kTowerMagic, towerLevel);
    createDefenseTower(18, 8, kTowerDoubleBoom, towerLevel);
    createDefenseTower(18, 18, kTowerDoubleBoom, towerLevel);
    createDefenseTower(22, 14, kTowerFire, towerLevel);

    for (int x = 8; x <= 20; ++x) {
        if (x == 14) {
            continue;
        }
        createSpikeTrap(x, 12);
    }
    for (int x = 10; x <= 18; ++x) {
        if (x == 10) {
            continue;
        }
        if (x == 14) {
            continue;
        }
        createSpikeTrap(x, 16);
    }
    for (int y = 10; y <= 18; ++y) {
        createSpikeTrap(14, y);
    }
    createSnapTrap(12, 14);
    createSnapTrap(16, 14);
}

// ===================================================
// 创建第9关 - 究极地刺
// ===================================================

void BattleScene::createLevel9() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(18, 12, kTowerDoubleBoom, towerLevel);
    createDefenseTower(22, 6, kTowerArrow, towerLevel);
    createDefenseTower(22, 18, kTowerMagic, towerLevel);

    for (int y = 8; y <= 22; y += 2) {
        for (int x = 6; x <= 16; ++x) {
            createSpikeTrap(x, y);
        }
    }
    for (int x = 8; x <= 16; x += 4) {
        for (int y = 9; y <= 21; ++y) {
            createSpikeTrap(x, y);
        }
    }
    createSnapTrap(14, 12);
    createSnapTrap(14, 18);
}

// ===================================================
// 创建第10关 - 双管炮的极意
// ===================================================

void BattleScene::createLevel10() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(10, 6, kTowerDoubleBoom, towerLevel);
    createDefenseTower(10, 20, kTowerDoubleBoom, towerLevel);
    createDefenseTower(16, 6, kTowerDoubleBoom, towerLevel);
    createDefenseTower(16, 20, kTowerDoubleBoom, towerLevel);
    createDefenseTower(14, 12, kTowerDoubleBoom, towerLevel);
    createDefenseTower(22, 12, kTowerFire, towerLevel);

    for (int x = 6; x <= 20; ++x) {
        if (x == 14) {
            continue;
        }
        createSpikeTrap(x, 10);
        createSpikeTrap(x, 18);
    }
    for (int y = 11; y <= 17; ++y) {
        createSpikeTrap(12, y);
    }
    createSnapTrap(14, 10);
    createSnapTrap(14, 18);
}

// ===================================================
// 创建第11关 - 夺命连环箭
// ===================================================

void BattleScene::createLevel11() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    createDefenseTower(8, 6, kTowerArrow, towerLevel);
    createDefenseTower(12, 6, kTowerArrow, towerLevel);
    createDefenseTower(16, 6, kTowerArrow, towerLevel);
    createDefenseTower(8, 22, kTowerArrow, towerLevel);
    createDefenseTower(12, 22, kTowerArrow, towerLevel);
    createDefenseTower(16, 22, kTowerArrow, towerLevel);

    for (int x = 6; x <= 22; ++x) {
        createSpikeTrap(x, 12);
        createSpikeTrap(x, 18);
    }
    createSnapTrap(10, 14);
    createSnapTrap(14, 14);
    createSnapTrap(18, 14);
}

// ===================================================
// 创建第12关 - 地狱的火焰
// ===================================================

void BattleScene::createLevel12() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    createEnemyBase(26, 12, towerLevel);

    for (int y = 6; y <= 22; ++y) {
        createSnapTrap(6, y);
        createSnapTrap(8, y);
    }

    for (int y = 6; y <= 22; ++y) {
        createSpikeTrap(10, y);
        createSpikeTrap(12, y);
        createSpikeTrap(14, y);
        createSpikeTrap(16, y);
    }

    int towerRows[] = { 6, 14, 22 };
    for (int row : towerRows) {
        createDefenseTower(17, row, kTowerFire, towerLevel);
        createDefenseTower(20, row, kTowerFire, towerLevel);
        createDefenseTower(23, row, kTowerBoom, towerLevel);
    }
}

// ===================================================
// 防守关卡
// ===================================================

void BattleScene::createDefenseBaseLayout(int towerLevel) {
    (void)towerLevel;
    if (!_gridMap || !_buildingLayer) {
        return;
    }

    auto* manager = BuildingManager::getInstance();
    manager->loadConfigs();

    const float cellSize = _gridMap->getCellSize();

    int baseId = manager->getMainBaseId();
    if (baseId <= 0) {
        baseId = 3001;
    }
    const auto* baseConfig = manager->getProductionConfig(baseId);
    int baseWidth = baseConfig ? baseConfig->width : 4;
    int baseHeight = baseConfig ? baseConfig->length : 4;
    int defenseBaseX = BattleConfig::GRID_WIDTH / 2 - baseWidth / 2;
    int defenseBaseY = BattleConfig::GRID_HEIGHT / 2 - baseHeight / 2;
    if (defenseBaseX < 0) defenseBaseX = 0;
    if (defenseBaseY < 0) defenseBaseY = 0;

    Vec2 baseAnchor = BaseScene::getBaseAnchorGrid();
    Vec2 barracksAnchor = BaseScene::getBarracksAnchorGrid();
    int anchorX = static_cast<int>(std::round(baseAnchor.x));
    int anchorY = static_cast<int>(std::round(baseAnchor.y));
    int barracksX = static_cast<int>(std::round(barracksAnchor.x));
    int barracksY = static_cast<int>(std::round(barracksAnchor.y));

    int offsetX = defenseBaseX - anchorX;
    int offsetY = defenseBaseY - anchorY;

    auto registerBuilding = [this](Node* building, bool isBase) {
        if (!building) {
            return;
        }
        _enemyBuildings.push_back(building);
        building->retain();
        _totalBuildingCount++;
        if (isBase) {
            _enemyBase = building;
            _enemyBaseDestroyed = false;
        }
    };

    auto placeBuilding = [this, cellSize, &registerBuilding](Node* building,
                                                             int gridX,
                                                             int gridY,
                                                             int width,
                                                             int height,
                                                             bool isBase) {
        if (!building) {
            return;
        }
        if (!_gridMap->canPlaceBuilding(gridX, gridY, width, height)) {
            return;
        }

        _buildingLayer->addChild(building);

        float centerX = (gridX + width * 0.5f) * cellSize;
        float centerY = (gridY + height * 0.5f) * cellSize;
        building->setPosition(Vec2(centerX, centerY));

        scaleBuildingToFit(building, width, height, cellSize);
        _gridMap->occupyCell(gridX, gridY, width, height, building);

        if (auto* trap = dynamic_cast<TrapBase*>(building)) {
            trap->setGridContext(_gridMap, gridX, gridY, width, height);
            return;
        }

        registerBuilding(building, isBase);
    };

    int baseLevel = Core::getInstance()->getBaseLevel() - 1;
    if (baseLevel < 0) {
        baseLevel = 0;
    }
    if (baseConfig && baseLevel > baseConfig->MAXLEVEL) {
        baseLevel = baseConfig->MAXLEVEL;
    }
    auto base = manager->createProductionBuilding(baseId, baseLevel);
    placeBuilding(base, defenseBaseX, defenseBaseY, baseWidth, baseHeight, true);

    int barracksId = manager->getBarracksId();
    if (barracksId <= 0) {
        barracksId = 3002;
    }
    const auto* barracksConfig = manager->getProductionConfig(barracksId);
    int barracksWidth = barracksConfig ? barracksConfig->width : 5;
    int barracksHeight = barracksConfig ? barracksConfig->length : 5;

    int defenseBarracksX = barracksX + offsetX;
    int defenseBarracksY = barracksY + offsetY;
    int barracksLevel = BaseScene::getBarracksLevel();
    if (barracksLevel < 0) {
        barracksLevel = 0;
    }
    if (barracksConfig && barracksLevel > barracksConfig->MAXLEVEL) {
        barracksLevel = barracksConfig->MAXLEVEL;
    }
    auto barracks = manager->createProductionBuilding(barracksId, barracksLevel);
    placeBuilding(barracks, defenseBarracksX, defenseBarracksY, barracksWidth, barracksHeight, false);

    const auto& savedBuildings = BaseScene::getSavedBuildings();
    for (const auto& saved : savedBuildings) {
        const auto& option = saved.option;
        int gridX = saved.gridX + offsetX;
        int gridY = saved.gridY + offsetY;

        Node* building = BaseScene::createBuildingFromOptionForDefense(option, saved.level);
        if (!building) {
            continue;
        }

        if (!_gridMap->canPlaceBuilding(gridX, gridY, option.gridWidth, option.gridHeight)) {
            continue;
        }

        _buildingLayer->addChild(building);

        float centerX = (gridX + option.gridWidth * 0.5f) * cellSize;
        float centerY = (gridY + option.gridHeight * 0.5f) * cellSize;
        building->setPosition(Vec2(centerX, centerY));

        scaleBuildingToFit(building, option.gridWidth, option.gridHeight, cellSize);
        _gridMap->occupyCell(gridX, gridY, option.gridWidth, option.gridHeight, building);

        if (auto* trap = dynamic_cast<TrapBase*>(building)) {
            trap->setGridContext(_gridMap, gridX, gridY, option.gridWidth, option.gridHeight);
            continue;
        }

        registerBuilding(building, false);
    }
}

void BattleScene::createDefenseLevel1() {
    createDefenseBaseLayout(0);
    resetDefenseSpawns();
    addDefenseWave(1012, 6, 0.45f, 0.6f);
    addDefenseWave(1011, 4, 0.55f, 1.0f);
}

void BattleScene::createDefenseLevel2() {
    createDefenseBaseLayout(0);
    resetDefenseSpawns();
    addDefenseWave(1012, 6, 0.45f, 0.6f);
    addDefenseWave(1010, 4, 0.55f, 0.8f);
    addDefenseWave(1001, 4, 0.6f, 0.8f);
}

void BattleScene::createDefenseLevel3() {
    createDefenseBaseLayout(1);
    resetDefenseSpawns();
    addDefenseWave(1011, 6, 0.45f, 0.6f);
    addDefenseWave(1005, 4, 0.55f, 0.8f);
    addDefenseWave(1004, 4, 0.55f, 0.8f);
    addDefenseWave(1008, 3, 0.65f, 0.9f);
}

void BattleScene::createDefenseLevel4() {
    createDefenseBaseLayout(1);
    resetDefenseSpawns();
    addDefenseWave(1006, 5, 0.5f, 0.6f);
    addDefenseWave(1003, 4, 0.55f, 0.8f);
    addDefenseWave(1001, 4, 0.6f, 0.8f);
    addDefenseWave(1008, 4, 0.65f, 0.8f);
}

void BattleScene::createDefenseLevel5() {
    createDefenseBaseLayout(2);
    resetDefenseSpawns();
    addDefenseWave(1010, 6, 0.5f, 0.6f);
    addDefenseWave(1004, 5, 0.6f, 0.8f);
    addDefenseWave(1002, 3, 0.75f, 1.0f);
    addDefenseWave(1009, 3, 0.65f, 0.8f);
}

void BattleScene::createDefenseLevel6() {
    createDefenseBaseLayout(2);
    resetDefenseSpawns();
    addDefenseWave(1007, 20, 0.55f, 0.6f);
    addDefenseWave(1009, 4, 0.7f, 0.8f);
    addDefenseWave(1002, 4, 0.75f, 0.9f);
    addDefenseWave(1003, 4, 0.55f, 0.8f);
    addDefenseWave(1006, 4, 0.55f, 0.8f);
}

// ===================================================
// 创建敌方基地
// ===================================================

void BattleScene::createEnemyBase(int gridX, int gridY, int level) {
    auto* manager = BuildingManager::getInstance();
    manager->loadConfigs();

    int baseId = manager->getEnemyBaseId();
    if (baseId <= 0) {
        baseId = 9001;
    }
    const auto* baseConfig = manager->getProductionConfig(baseId);
    int baseWidth = baseConfig ? baseConfig->width : 4;
    int baseHeight = baseConfig ? baseConfig->length : 4;

    int resolvedLevel = level;
    if (resolvedLevel < 0) {
        resolvedLevel = 0;
    }
    if (baseConfig && resolvedLevel > baseConfig->MAXLEVEL) {
        resolvedLevel = baseConfig->MAXLEVEL;
    }
    auto base = manager->createProductionBuilding(baseId, resolvedLevel);
    if (base) {
        _buildingLayer->addChild(base);

        float cellSize = _gridMap->getCellSize();
        float centerX = (gridX + baseWidth * 0.5f) * cellSize;
        float centerY = (gridY + baseHeight * 0.5f) * cellSize;
        base->setPosition(Vec2(centerX, centerY));

        scaleBuildingToFit(base, baseWidth, baseHeight, cellSize);

        _gridMap->occupyCell(gridX, gridY, baseWidth, baseHeight, base);

        _enemyBuildings.push_back(base);
        base->retain();
        _totalBuildingCount++;
        _enemyBase = base;
        _enemyBaseDestroyed = false;

        CCLOG("[战斗场景] 敌方基地创建完成");
    }
}

// ===================================================
// 创建防御塔
// ===================================================

void BattleScene::createDefenseTower(int gridX, int gridY, int type, int level) {
    auto* manager = BuildingManager::getInstance();
    manager->loadConfigs();

    int configId = manager->getBattleTowerConfigId(type);
    if (configId <= 0) {
        CCLOG("[战斗场景] 未找到防御塔配置类型: %d", type);
        return;
    }
    const auto* towerConfig = manager->getDefenceConfig(configId);
    if (!towerConfig) {
        CCLOG("[战斗场景] 防御塔配置缺失: %d", configId);
        return;
    }

    int towerWidth = towerConfig->width > 0 ? towerConfig->width : 3;
    int towerHeight = towerConfig->length > 0 ? towerConfig->length : 3;

    int resolvedLevel = level;
    if (resolvedLevel < 0) {
        resolvedLevel = 0;
    }
    if (resolvedLevel > towerConfig->MAXLEVEL) {
        resolvedLevel = towerConfig->MAXLEVEL;
    }

    auto tower = manager->createDefenceBuilding(configId, resolvedLevel);
    if (tower) {
        _buildingLayer->addChild(tower);

        float cellSize = _gridMap->getCellSize();
        float centerX = (gridX + towerWidth * 0.5f) * cellSize;
        float centerY = (gridY + towerHeight * 0.5f) * cellSize;
        tower->setPosition(Vec2(centerX, centerY));

        scaleBuildingToFit(tower, towerWidth, towerHeight, cellSize);

        _gridMap->occupyCell(gridX, gridY, towerWidth, towerHeight, tower);

        _enemyBuildings.push_back(tower);
        tower->retain();
        _totalBuildingCount++;

        CCLOG("[战斗场景] 防御塔创建完成 (类型%d)", type);
    }
}

// ===================================================
// 创建陷阱
// ===================================================

void BattleScene::createSpikeTrap(int gridX, int gridY) {
    auto* trap = SpikeTrap::create();
    if (!trap) {
        return;
    }

    _buildingLayer->addChild(trap);

    float cellSize = _gridMap->getCellSize();
    float centerX = (gridX + 0.5f) * cellSize;
    float centerY = (gridY + 0.5f) * cellSize;
    trap->setPosition(Vec2(centerX, centerY));

    scaleBuildingToFit(trap, 1, 1, cellSize);
    _gridMap->occupyCell(gridX, gridY, 1, 1, trap);
    trap->setGridContext(_gridMap, gridX, gridY, 1, 1);
}

void BattleScene::createSnapTrap(int gridX, int gridY) {
    auto* trap = SnapTrap::create();
    if (!trap) {
        return;
    }

    _buildingLayer->addChild(trap);

    float cellSize = _gridMap->getCellSize();
    float centerX = (gridX + 0.5f) * cellSize;
    float centerY = (gridY + 0.5f) * cellSize;
    trap->setPosition(Vec2(centerX, centerY));

    scaleBuildingToFit(trap, 1, 1, cellSize);
    _gridMap->occupyCell(gridX, gridY, 1, 1, trap);
    trap->setGridContext(_gridMap, gridX, gridY, 1, 1);
}

void BattleScene::addTrapCluster(int startX, int startY, int width, int height, const std::vector<Vec2>& snapCells) {
    auto isSnapCell = [&snapCells](int x, int y) {
        for (const auto& cell : snapCells) {
            if (static_cast<int>(cell.x) == x && static_cast<int>(cell.y) == y) {
                return true;
            }
        }
        return false;
    };

    for (int x = startX; x < startX + width; ++x) {
        for (int y = startY; y < startY + height; ++y) {
            if (isSnapCell(x, y)) {
                continue;
            }
            createSpikeTrap(x, y);
        }
    }

    for (const auto& cell : snapCells) {
        createSnapTrap(static_cast<int>(cell.x), static_cast<int>(cell.y));
    }
}


// ===================================================
// 建筑缩放调整
// ===================================================

void BattleScene::scaleBuildingToFit(Node* building, int gridWidth, int gridHeight, float cellSize) {
    if (!building) return;

    auto sprite = NodeUtils::findBodySprite(building);
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

    constexpr float kExitButtonSize = 50.0f;
    constexpr float kExitButtonMargin = 18.0f;
    auto exitBtn = Button::create("UI/exit.png", "UI/exit.png");
    if (!exitBtn) {
        exitBtn = Button::create("exit.png", "exit.png");
    }
    if (exitBtn) {
        Size btnSize = exitBtn->getContentSize();
        if (btnSize.width > 0.0f && btnSize.height > 0.0f) {
            float scale = kExitButtonSize / std::max(btnSize.width, btnSize.height);
            exitBtn->setScale(scale);
        }
        float exitX = origin.x + kExitButtonMargin + kExitButtonSize / 2;
        exitBtn->setPosition(Vec2(exitX, topY));
        exitBtn->setPressedActionEnabled(true);
        exitBtn->setZoomScale(0.06f);
        exitBtn->setSwallowTouches(true);
        exitBtn->addClickEventListener([this](Ref*) {
            AudioManager::playButtonCancel();
            CCLOG("[BattleScene] Exit battle");
            this->onExitButton(nullptr);
        });
        _uiLayer->addChild(exitBtn, 10);
    }

    setupDeployArea();
}


// ===================================================
// 部署范围提示
// ===================================================

void BattleScene::setupDeployRangeHint() {
    if (!_gridMap) {
        return;
    }

    int deployWidth = BattleConfig::DEPLOY_MAX_X - BattleConfig::DEPLOY_MIN_X + 1;
    int deployHeight = BattleConfig::DEPLOY_MAX_Y - BattleConfig::DEPLOY_MIN_Y + 1;
    if (deployWidth <= 0 || deployHeight <= 0) {
        return;
    }

    float cellSize = _gridMap->getCellSize();
    auto hintLayer = LayerColor::create(
        Color4B(80, 130, 90, 70),
        deployWidth * cellSize,
        deployHeight * cellSize
    );
    hintLayer->setAnchorPoint(Vec2(0.0f, 0.0f));
    hintLayer->setIgnoreAnchorPointForPosition(false);
    hintLayer->setPosition(Vec2(
        BattleConfig::DEPLOY_MIN_X * cellSize,
        BattleConfig::DEPLOY_MIN_Y * cellSize
    ));
    _gridMap->addChild(hintLayer, -1);
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

    if (_battleMode == BattleMode::Defense) {
        auto infoLabel = Label::createWithTTF("Defense mode", "fonts/arial.ttf", 12);
        if (!infoLabel) {
            infoLabel = Label::createWithSystemFont("Defense mode", "Arial", 12);
        }
        infoLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + BattleConfig::UI_BOTTOM_HEIGHT / 2));
        infoLabel->setColor(Color3B::GRAY);
        _uiLayer->addChild(infoLabel);
        _selectedUnitId = -1;
        return;
    }

    if (_remainingUnits.empty()) {
        auto emptyLabel = Label::createWithTTF("No units trained.", "fonts/arial.ttf", 12);
        if (!emptyLabel) {
            emptyLabel = Label::createWithSystemFont("No units trained.", "Arial", 12);
        }
        emptyLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + BattleConfig::UI_BOTTOM_HEIGHT / 2));
        emptyLabel->setColor(Color3B::GRAY);
        _uiLayer->addChild(emptyLabel);
        _selectedUnitId = -1;
        return;
    }

    _unitDeployArea = Node::create();
    _unitDeployArea->setPosition(Vec2::ZERO);
    _uiLayer->addChild(_unitDeployArea);

    float btnSize = BattleConfig::DEPLOY_BUTTON_SIZE;
    float spacing = BattleConfig::DEPLOY_BUTTON_SPACING;
    int unitCount = static_cast<int>(_remainingUnits.size());
    float totalWidth = unitCount * btnSize + (unitCount - 1) * spacing;
    float startX = origin.x + (visibleSize.width - totalWidth) / 2 + btnSize / 2;
    float centerY = origin.y + BattleConfig::UI_BOTTOM_HEIGHT / 2;

    _deployButtons.clear();
    _selectedUnitId = -1;

    int index = 0;
    for (const auto& pair : _remainingUnits) {
        float xPos = startX + index * (btnSize + spacing);
        auto btn = createDeployButton(pair.first, pair.second, xPos);
        if (btn) {
            btn->setPosition(Vec2(xPos, centerY));
            _unitDeployArea->addChild(btn);
            _deployButtons[pair.first] = btn;
            refreshDeployButton(pair.first);
        }
        index++;
    }

    setSelectedUnit(getFirstAvailableUnitId());

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

    float btnSize = BattleConfig::DEPLOY_BUTTON_SIZE;
    node->setContentSize(Size(btnSize, btnSize));

    // 获取单位配置
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    std::string unitName = config ? config->name.substr(0, 3) : "???";

    // 按钮背景
    auto bg = LayerColor::create(Color4B(50, 70, 50, 255), btnSize, btnSize);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    bg->setName("bg");
    node->addChild(bg);

    // 边框
    auto border = DrawNode::create();
    border->drawRect(Vec2(-btnSize / 2, -btnSize / 2), Vec2(btnSize / 2, btnSize / 2), Color4F::WHITE);
    border->setName("border");
    node->addChild(border, 1);

    // 选中高亮
    auto selectBorder = DrawNode::create();
    selectBorder->drawRect(Vec2(-btnSize / 2, -btnSize / 2), Vec2(btnSize / 2, btnSize / 2), Color4F::YELLOW);
    selectBorder->setVisible(false);
    selectBorder->setName("selectBorder");
    node->addChild(selectBorder, 3);

    // 单位名称
    auto nameLabel = Label::createWithTTF(unitName, "fonts/arial.ttf", 9);
    if (!nameLabel) {
        nameLabel = Label::createWithSystemFont(unitName, "Arial", 9);
    }
    nameLabel->setPosition(Vec2(0, 16));
    nameLabel->setColor(Color3B::WHITE);
    nameLabel->setName("nameLabel");
    node->addChild(nameLabel, 2);

    // 待机动画图标（持续显示）
    auto idleIcon = createUnitIdleIcon(unitId, 26.0f, true);
    if (idleIcon) {
        idleIcon->setPosition(Vec2(0, 0));
        idleIcon->setVisible(true);
        idleIcon->setName("idleIcon");
        node->addChild(idleIcon, 2);
    }

    // 数量
    auto countLabel = Label::createWithTTF("x" + std::to_string(count), "fonts/arial.ttf", 10);
    if (!countLabel) {
        countLabel = Label::createWithSystemFont("x" + std::to_string(count), "Arial", 10);
    }
    countLabel->setPosition(Vec2(0, -16));
    countLabel->setColor(Color3B::YELLOW);
    countLabel->setName("countLabel");
    node->addChild(countLabel, 2);

    // 点击选择单位
    auto selectButton = Button::create();
    selectButton->setContentSize(Size(btnSize, btnSize));
    selectButton->setScale9Enabled(true);
    selectButton->setAnchorPoint(Vec2(0.5f, 0.5f));
    selectButton->setPosition(Vec2::ZERO);
    const float originScale = node->getScale();
    selectButton->setSwallowTouches(true);
    selectButton->addTouchEventListener([this, unitId, node, originScale](Ref*, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::BEGAN) {
            node->setScale(originScale * 0.96f);
            return;
        }
        if (type == Widget::TouchEventType::CANCELED) {
            node->setScale(originScale);
            return;
        }
        if (type != Widget::TouchEventType::ENDED) {
            return;
        }

        node->setScale(originScale);
        AudioManager::playButtonClick();
        setSelectedUnit(unitId);
    });
    selectButton->setName("selectButton");
    node->addChild(selectButton, 4);

    // 存储unitId以便部署时使用
    node->setTag(unitId);

    return node;
}

// ===================================================
// 创建部署栏待机动画图标
// ===================================================
Sprite* BattleScene::createUnitIdleIcon(int unitId, float targetSize, bool forceAnimate) {
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) {
        return nullptr;
    }

    std::string baseName = config->spriteFrameName;
    if (baseName.size() > 4) {
        std::string suffix = baseName.substr(baseName.size() - 4);
        if (suffix == ".png" || suffix == ".PNG") {
            baseName = baseName.substr(0, baseName.size() - 4);
        }
    }

    auto sprite = Sprite::create(baseName + ".png");
    if (!sprite) {
        sprite = Sprite::create(baseName + "_" + config->anim_idle + "_1.png");
    }
    if (!sprite) {
        return nullptr;
    }

    Size contentSize = sprite->getContentSize();
    if (contentSize.width > 0 && contentSize.height > 0) {
        float scale = targetSize / std::max(contentSize.width, contentSize.height);
        sprite->setScale(scale);
    }

    auto anim = AnimationUtils::buildAnimationFromFrames(
        baseName,
        config->anim_idle,
        config->anim_idle_frames,
        config->anim_idle_delay,
        false
    );
    if ((!anim || config->anim_idle_frames <= 1) && forceAnimate) {
        anim = AnimationUtils::buildAnimationFromFrames(
            baseName,
            config->anim_walk,
            config->anim_walk_frames,
            config->anim_walk_delay,
            false
        );
    }

    if (anim && anim->getFrames().size() > 1) {
        auto animate = Animate::create(anim);
        sprite->runAction(RepeatForever::create(animate));
    }

    return sprite;
}

// ===================================================
// 获取第一个可部署单位
// ===================================================

int BattleScene::getFirstAvailableUnitId() const {
    for (const auto& pair : _remainingUnits) {
        if (pair.second > 0) {
            return pair.first;
        }
    }
    return -1;
}

// ===================================================
// 设置当前选中的单位
// ===================================================

void BattleScene::setSelectedUnit(int unitId) {
    if (unitId != -1) {
        auto it = _remainingUnits.find(unitId);
        if (it == _remainingUnits.end() || it->second <= 0) {
            unitId = -1;
        }
    }

    _selectedUnitId = unitId;

    for (const auto& pair : _deployButtons) {
        if (!pair.second) continue;
        bool hasCount = false;
        auto countIt = _remainingUnits.find(pair.first);
        if (countIt != _remainingUnits.end()) {
            hasCount = countIt->second > 0;
        }
        auto selectBorder = pair.second->getChildByName("selectBorder");
        if (selectBorder) {
            selectBorder->setVisible(hasCount && pair.first == _selectedUnitId);
        }
    }
}

// ===================================================
// 刷新部署按钮显示
// ===================================================

void BattleScene::refreshDeployButton(int unitId) {
    auto buttonIt = _deployButtons.find(unitId);
    if (buttonIt == _deployButtons.end() || !buttonIt->second) {
        return;
    }

    int count = 0;
    auto countIt = _remainingUnits.find(unitId);
    if (countIt != _remainingUnits.end()) {
        count = countIt->second;
    }

    bool hasCount = count > 0;
    auto node = buttonIt->second;
    auto countLabel = dynamic_cast<Label*>(node->getChildByName("countLabel"));
    if (countLabel) {
        countLabel->setString("x" + std::to_string(count));
        countLabel->setColor(hasCount ? Color3B::YELLOW : Color3B(160, 160, 160));
    }

    auto nameLabel = dynamic_cast<Label*>(node->getChildByName("nameLabel"));
    if (nameLabel) {
        nameLabel->setColor(hasCount ? Color3B::WHITE : Color3B(160, 160, 160));
    }

    auto bg = dynamic_cast<LayerColor*>(node->getChildByName("bg"));
    if (bg) {
        bg->setColor(hasCount ? Color3B(50, 70, 50) : Color3B(70, 70, 70));
    }

    auto border = dynamic_cast<DrawNode*>(node->getChildByName("border"));
    if (border) {
        border->clear();
        Color4F borderColor = hasCount ? Color4F::WHITE : Color4F(0.5f, 0.5f, 0.5f, 1.0f);
        float btnSize = BattleConfig::DEPLOY_BUTTON_SIZE;
        border->drawRect(Vec2(-btnSize / 2, -btnSize / 2), Vec2(btnSize / 2, btnSize / 2), borderColor);
    }

    auto selectBorder = node->getChildByName("selectBorder");
    if (selectBorder) {
        selectBorder->setVisible(hasCount && unitId == _selectedUnitId);
    }
    auto idleIcon = node->getChildByName("idleIcon");
    if (idleIcon) {
        idleIcon->setVisible(true);
    }

    auto selectButton = dynamic_cast<Button*>(node->getChildByName("selectButton"));
    if (selectButton) {
        selectButton->setEnabled(hasCount);
        selectButton->setBright(hasCount);
    }
}

// ===================================================
// 部署范围检查
// ===================================================

bool BattleScene::isDeployGridAllowed(int gridX, int gridY) const {
    if (!_gridMap) {
        return false;
    }

    int gridWidth = _gridMap->getGridWidth();
    int gridHeight = _gridMap->getGridHeight();
    if (gridX < 0 || gridY < 0 || gridX >= gridWidth || gridY >= gridHeight) {
        return false;
    }

    if (gridX < BattleConfig::DEPLOY_MIN_X || gridX > BattleConfig::DEPLOY_MAX_X) {
        return false;
    }
    if (gridY < BattleConfig::DEPLOY_MIN_Y || gridY > BattleConfig::DEPLOY_MAX_Y) {
        return false;
    }

    return true;
}

// ===================================================
// 部署士兵
// ===================================================

void BattleScene::deploySoldier(int unitId, const Vec2& position) {
    // 兜底检查，避免非法位置部署
    if (_gridMap) {
        Vec2 gridPos = _gridMap->worldToGrid(position);
        int gridX = static_cast<int>(gridPos.x);
        int gridY = static_cast<int>(gridPos.y);
        if (!isDeployGridAllowed(gridX, gridY)) {
            return;
        }
    }

    // 检查是否有可部署的该类型单位
    auto it = _remainingUnits.find(unitId);
    if (it == _remainingUnits.end() || it->second <= 0) {
        CCLOG("[战斗场景] 没有可部署的单位: %d", unitId);
        return;
    }

    // 创建士兵（同步训练等级）
    int unitLevel = UnitManager::getInstance()->getUnitLevel(unitId);
    auto soldier = UnitManager::getInstance()->spawnSoldier(unitId, position, unitLevel);
    if (soldier) {
        _soldierLayer->addChild(soldier);
        _soldiers.push_back(soldier);
        soldier->retain();
        _totalDeployedCount++;

        spawnDeployEffect(position);

        // 更新剩余数量
        it->second--;
        // 训练兵种仅作为出战上限，战斗中不消耗库存

        CCLOG("[战斗场景] 部署士兵: %d 在位置 (%.1f, %.1f), 剩余 %d",
            unitId, position.x, position.y, it->second);

        // 更新UI并处理选中状态
        refreshDeployButton(unitId);
        if (it->second <= 0 && unitId == _selectedUnitId) {
            setSelectedUnit(getFirstAvailableUnitId());
        }
    }
}

void BattleScene::spawnEnemySoldier(int unitId, const Vec2& position, int level) {
    auto soldier = UnitManager::getInstance()->spawnSoldier(unitId, position, level);
    if (!soldier) {
        return;
    }

    _soldierLayer->addChild(soldier);
    _soldiers.push_back(soldier);
    soldier->retain();
    _totalDeployedCount++;

    spawnDeployEffect(position);
}

void BattleScene::spawnDeployEffect(const Vec2& position) {
    if (!_gridMap) {
        return;
    }

    auto ring = DrawNode::create();
    ring->drawCircle(Vec2::ZERO, 14.0f, 0.0f, 20, false, Color4F(0.8f, 1.0f, 0.8f, 0.8f));
    ring->setPosition(position);
    _gridMap->addChild(ring, 30);

    auto scale = ScaleTo::create(0.25f, 1.6f);
    auto fade = FadeTo::create(0.25f, 0);
    ring->runAction(Sequence::create(Spawn::create(scale, fade, nullptr), RemoveSelf::create(), nullptr));
}

// ===================================================
// 触摸事件初始化
// ===================================================

void BattleScene::initTouchListener() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

// ===================================================
// 悬浮信息初始化
// ===================================================

void BattleScene::initHoverInfo() {
    _hoverInfoPanel = Node::create();
    _hoverInfoPanel->setAnchorPoint(Vec2(0, 0.5f));
    _hoverInfoPanel->setIgnoreAnchorPointForPosition(false);
    _hoverInfoPanel->setVisible(false);
    this->addChild(_hoverInfoPanel, 200);

    _hoverInfoBg = LayerColor::create(Color4B(12, 12, 12, 128), 260, 140);
    _hoverInfoBg->setAnchorPoint(Vec2(0, 0));
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

bool BattleScene::onTouchBegan(Touch* touch, Event* event) {
    if (_battleMode == BattleMode::Defense) {
        return false;
    }
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

    if (!_gridMap) {
        return false;
    }

    Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
    Vec2 gridPos = _gridMap->worldToGrid(localPos);
    int gridX = static_cast<int>(gridPos.x);
    int gridY = static_cast<int>(gridPos.y);

    // 限制在允许部署的格子范围内
    if (!isDeployGridAllowed(gridX, gridY)) {
        return false;
    }

    // 在地图上部署士兵（使用当前选中的单位）
    int unitId = _selectedUnitId;
    if (unitId == -1) {
        unitId = getFirstAvailableUnitId();
        if (unitId != -1) {
            setSelectedUnit(unitId);
        }
    }
    if (unitId != -1) {
        deploySoldier(unitId, localPos);
    }

    return true;
}

// ===================================================
// 悬浮信息处理
// ===================================================

void BattleScene::updateHoverInfo(const Vec2& worldPos) {
    if (!_gridMap || !_buildingLayer || _battleEnded) {
        clearBuildingInfo();
        return;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    if (worldPos.y < origin.y + BattleConfig::UI_BOTTOM_HEIGHT
        || worldPos.y > origin.y + visibleSize.height - BattleConfig::UI_TOP_HEIGHT) {
        clearBuildingInfo();
        return;
    }

    if (_hoverInfoPanel && _hoverInfoBg && _hoverInfoPanel->isVisible()) {
        Size panelSize = _hoverInfoBg->getContentSize();
        Vec2 panelPos = _hoverInfoPanel->getPosition();
        Rect worldRect(panelPos.x, panelPos.y - panelSize.height * 0.5f, panelSize.width, panelSize.height);
        if (worldRect.containsPoint(worldPos)) {
            return;
        }
    }

    Node* building = pickBuildingAt(worldPos);
    if (!building) {
        if (_hoverInfoPanel && _hoverInfoBg && _hoverInfoPanel->isVisible() && _hoveredBuilding) {
            Rect keepRect = getHoverKeepRect(_hoveredBuilding, _hoverInfoPanel, _hoverInfoBg);
            if (keepRect.containsPoint(worldPos)) {
                return;
            }
        }
        clearBuildingInfo();
        return;
    }

    if (building != _hoveredBuilding) {
        _hoveredBuilding = building;
    }
    showBuildingInfo(building);
    Vec2 anchorPos = getHoverAnchorWorldPos(building);
    if (anchorPos == Vec2::ZERO) {
        anchorPos = worldPos;
    }
    updateHoverPanelPosition(anchorPos);
}

Node* BattleScene::pickBuildingAt(const Vec2& worldPos) const {
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
            if (NodeUtils::hitTestBuilding(child, worldPos)) {
                return child;
            }
        }
    }
    return nullptr;
}

void BattleScene::showBuildingInfo(Node* building) {
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
    _hoverInfoLabel->setPosition(Vec2(kHoverPanelPaddingX, panelHeight - kHoverPanelPaddingY));
    _hoverInfoPanel->setContentSize(Size(panelWidth, panelHeight));

    auto border = dynamic_cast<DrawNode*>(_hoverInfoPanel->getChildByName("hoverBorder"));
    if (border) {
        border->clear();
        border->drawRect(Vec2(0, 0), Vec2(panelWidth, panelHeight), Color4F(0.8f, 0.8f, 0.8f, 1.0f));
    }

    _hoverInfoPanel->setVisible(true);
    updateHoverOverlays(building);
}

void BattleScene::updateHoverOverlays(Node* building) {
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
        NodeUtils::drawDashedCircle(_hoverRangeNode, center, range, Color4F(0.85f, 0.85f, 0.85f, 0.8f));
        _hoverRangeNode->setVisible(true);
    }
    else {
        _hoverRangeNode->setVisible(false);
    }
}

void BattleScene::updateHoverPanelPosition(const Vec2& worldPos) {
    if (!_hoverInfoPanel || !_hoverInfoBg) {
        return;
    }
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    Size panelSize = _hoverInfoBg->getContentSize();
    Vec2 desiredPos(
        worldPos.x + kHoverPanelOffsetX,
        worldPos.y
    );

    float maxX = origin.x + visibleSize.width - panelSize.width - 6.0f;
    float minX = origin.x + 6.0f;
    float maxY = origin.y + visibleSize.height - panelSize.height * 0.5f - 6.0f;
    float minY = origin.y + panelSize.height * 0.5f + 6.0f;

    float clampedX = std::min(std::max(desiredPos.x, minX), maxX);
    float clampedY = std::min(std::max(desiredPos.y, minY), maxY);

    _hoverInfoPanel->setPosition(Vec2(clampedX, clampedY));
}

void BattleScene::clearBuildingInfo() {
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
    DefenceBuilding::clearEnemySoldiersIf(&_soldiers);
    TrapBase::clearEnemySoldiersIf(&_soldiers);
    Soldier::clearEnemyBuildingsIf(&_enemyBuildings);

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
    _enemyBase = nullptr;
    _enemyBaseDestroyed = false;

    Scene::onExit();
}

// ===================================================
// 战斗逻辑更新
// ===================================================

void BattleScene::updateBattle(float dt) {
    if (_battleMode == BattleMode::Defense) {
        updateDefenseSpawns();
    }

    // 清理已移除的士兵，避免悬空指针
    for (auto& soldier : _soldiers) {
        if (soldier && !soldier->getParent()) {
            _deadSoldierCount++;
            soldier->release();
            soldier = nullptr;
        }
    }

    // 检查被摧毁的建筑并释放引用
    int destroyed = 0;
    for (auto& building : _enemyBuildings) {
        if (building && !building->getParent()) {
            if (building == _enemyBase) {
                _enemyBaseDestroyed = true;
                _enemyBase = nullptr;
            }
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

void BattleScene::updateDefenseSpawns() {
    if (_nextDefenseSpawnIndex >= _defenseSpawns.size()) {
        return;
    }
    int defenseId = getDefenseLevelIndex();
    int enemyLevel = 0;
    if (defenseId >= 5) {
        enemyLevel = 2;
    }
    else if (defenseId >= 3) {
        enemyLevel = 1;
    }

    while (_nextDefenseSpawnIndex < _defenseSpawns.size()) {
        const auto& spawn = _defenseSpawns[_nextDefenseSpawnIndex];
        if (_battleTime < spawn.time) {
            break;
        }
        int resolvedLevel = enemyLevel;
        if (defenseId == kDefenseMaxLevelId && spawn.unitId == kKingUnitId) {
            const UnitConfig* cfg = UnitManager::getInstance()->getConfig(spawn.unitId);
            if (cfg) {
                resolvedLevel = cfg->MAXLEVEL;
            }
        }
        Vec2 pos = getDefenseSpawnPosition(static_cast<int>(_nextDefenseSpawnIndex));
        spawnEnemySoldier(spawn.unitId, pos, resolvedLevel);
        _nextDefenseSpawnIndex++;
    }
}

// ===================================================
// 检查战斗结束
// ===================================================

void BattleScene::checkBattleEnd() {
    if (_battleMode == BattleMode::Defense) {
        if (_enemyBaseDestroyed) {
            onBattleLose();
            return;
        }

        bool hasEnemies = false;
        for (auto& soldier : _soldiers) {
            if (soldier && soldier->getParent()) {
                hasEnemies = true;
                break;
            }
        }
        bool hasPendingSpawns = _nextDefenseSpawnIndex < _defenseSpawns.size();
        if (!hasEnemies && !hasPendingSpawns) {
            onBattleWin();
            return;
        }

        if (_battleTime >= BattleConfig::BATTLE_TIME_LIMIT) {
            onBattleWin();
            return;
        }

        return;
    }

    // 基地被摧毁则直接胜利
    if (_enemyBaseDestroyed) {
        onBattleWin();
        return;
    }

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

void BattleScene::freezeBattleActors() {
    // 战斗结束后冻结场上单位，避免切换关卡时残留伤害
    Soldier::setEnemyBuildings(nullptr);
    DefenceBuilding::setEnemySoldiers(nullptr);
    TrapBase::setEnemySoldiers(nullptr);

    for (auto* soldier : _soldiers) {
        if (soldier) {
            soldier->unscheduleUpdate();
        }
    }
    for (auto* building : _enemyBuildings) {
        if (building) {
            building->unscheduleUpdate();
        }
    }
}

// ===================================================
// 战斗胜利
// ===================================================

void BattleScene::onBattleWin() {
    _battleEnded = true;
    freezeBattleActors();
    CCLOG("[战斗场景] 战斗胜利！");
    AudioManager::stopBgm();
    AudioManager::playVictory();

    // 计算并发放奖励
    float destroyedRatio = _totalBuildingCount > 0
        ? static_cast<float>(_destroyedBuildingCount) / _totalBuildingCount
        : 1.0f;
    float remainingRatio = std::max(0.0f, BattleConfig::BATTLE_TIME_LIMIT - _battleTime)
        / BattleConfig::BATTLE_TIME_LIMIT;
    float rewardFactor = 0.75f + destroyedRatio * 0.35f + remainingRatio * 0.15f;
    if (_enemyBaseDestroyed) {
        rewardFactor += 0.1f;
    }
    if (rewardFactor > 1.6f) {
        rewardFactor = 1.6f;
    }

    int rewardLevel = getRewardLevel();
    int baseCoin = 180 + rewardLevel * 120;
    int baseDiamond = 4 + rewardLevel * 2;
    int rewardCoin = static_cast<int>(std::round(baseCoin * rewardFactor));
    int rewardDiamond = static_cast<int>(std::round(baseDiamond * rewardFactor));
    if (rewardCoin < 0) rewardCoin = 0;
    if (rewardDiamond < 0) rewardDiamond = 0;

    Core::getInstance()->addResource(ResourceType::COIN, rewardCoin);
    Core::getInstance()->addResource(ResourceType::DIAMOND, rewardDiamond);

    int stars = calculateStarCount();
    Core::getInstance()->setLevelStars(getRecordLevelId(), stars);
    showBattleResult(true, stars);
}

// ===================================================
// 战斗失败
// ===================================================

void BattleScene::onBattleLose() {
    _battleEnded = true;
    freezeBattleActors();
    CCLOG("[战斗场景] 战斗失败！");
    AudioManager::stopBgm();
    AudioManager::playLose();

    int stars = calculateStarCount();
    showBattleResult(false, stars);
}

int BattleScene::calculateStarCount() const {
    if (_battleMode == BattleMode::Defense) {
        if (_enemyBaseDestroyed) {
            return 0;
        }
        float destroyedRatio = _totalBuildingCount > 0
            ? static_cast<float>(_destroyedBuildingCount) / static_cast<float>(_totalBuildingCount)
            : 0.0f;
        if (destroyedRatio <= 0.2f) {
            return 3;
        }
        if (destroyedRatio <= 0.5f) {
            return 2;
        }
        return 1;
    }

    // 以“死亡兵种数量 / 总兵种数量”的比例评星
    int total = _totalDeployedCount;
    if (total <= 0) {
        return 1;
    }
    float ratio = static_cast<float>(_deadSoldierCount) / static_cast<float>(total);
    if (ratio <= 0.34f) {
        return 3;
    }
    if (ratio <= 0.67f) {
        return 2;
    }
    return 1;
}

int BattleScene::getDefenseLevelIndex() const {
    if (_battleMode != BattleMode::Defense) {
        return 0;
    }
    int index = _levelId;
    if (_levelId > kDefenseLevelOffset) {
        index = _levelId - kDefenseLevelOffset;
    }
    if (index < 1) {
        index = 1;
    }
    if (index > kDefenseMaxLevelId) {
        index = kDefenseMaxLevelId;
    }
    return index;
}

int BattleScene::getRewardLevel() const {
    if (_battleMode == BattleMode::Defense) {
        return getDefenseLevelIndex();
    }
    return _levelId;
}

int BattleScene::getRecordLevelId() const {
    if (_battleMode == BattleMode::Defense) {
        if (_levelId <= kDefenseLevelOffset) {
            return kDefenseLevelOffset + getDefenseLevelIndex();
        }
        return _levelId;
    }
    return _levelId;
}

void BattleScene::resetDefenseSpawns() {
    _defenseSpawns.clear();
    _nextDefenseSpawnIndex = 0;
    _defenseSpawnCursor = 0.0f;
    _defenseTotalUnits = 0;
}

void BattleScene::addDefenseWave(int unitId, int count, float interval, float delay) {
    if (count <= 0) {
        return;
    }
    if (delay > 0.0f) {
        _defenseSpawnCursor += delay;
    }
    if (interval < 0.05f) {
        interval = 0.05f;
    }
    for (int i = 0; i < count; ++i) {
        DefenseSpawn spawn;
        spawn.time = _defenseSpawnCursor;
        spawn.unitId = unitId;
        _defenseSpawns.push_back(spawn);
        _defenseSpawnCursor += interval;
    }
    _defenseTotalUnits += count;
}

Vec2 BattleScene::getDefenseSpawnPosition(int index) const {
    if (!_gridMap) {
        return Vec2::ZERO;
    }

    const int maxX = BattleConfig::GRID_WIDTH - 3;
    const int maxY = BattleConfig::GRID_HEIGHT - 3;
    const Vec2 points[] = {
        Vec2(2, 4), Vec2(2, 12), Vec2(2, 20), Vec2(2, maxY),
        Vec2(maxX, 4), Vec2(maxX, 12), Vec2(maxX, 20), Vec2(maxX, maxY),
        Vec2(8, 2), Vec2(18, 2), Vec2(28, 2),
        Vec2(8, maxY), Vec2(18, maxY), Vec2(28, maxY)
    };
    int total = static_cast<int>(sizeof(points) / sizeof(points[0]));
    int idx = index % total;
    int gridX = static_cast<int>(points[idx].x);
    int gridY = static_cast<int>(points[idx].y);
    return _gridMap->gridToWorld(gridX, gridY);
}

void BattleScene::showBattleResult(bool isWin, int stars) {
    if (_resultLayer) {
        _resultLayer->removeFromParent();
        _resultLayer = nullptr;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _resultLayer = Node::create();
    this->addChild(_resultLayer, 1000);

    auto mask = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    mask->setPosition(origin);
    _resultLayer->addChild(mask, 0);

    // 吞掉触摸，避免底层UI误触
    auto swallow = EventListenerTouchOneByOne::create();
    swallow->setSwallowTouches(true);
    swallow->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(swallow, mask);

    const char* resultPath = isWin ? "UI/Victory.png" : "UI/Defeat.png";
    auto resultSprite = Sprite::create(resultPath);
    if (resultSprite) {
        float maxWidth = visibleSize.width * 0.6f;
        float maxHeight = visibleSize.height * 0.25f;
        Size size = resultSprite->getContentSize();
        if (size.width > 0 && size.height > 0) {
            float scale = std::min(maxWidth / size.width, maxHeight / size.height);
            resultSprite->setScale(scale);
        }
        resultSprite->setPosition(Vec2(
            origin.x + visibleSize.width * 0.5f,
            origin.y + visibleSize.height * 0.72f
        ));
        _resultLayer->addChild(resultSprite, 1);
    }

    // 统一用三颗星展示评级（左到右：实心/空心）
    const char* filledStarPath = "UI/LevelSelect/result_1.png";
    const char* emptyStarPath = "UI/LevelSelect/result_2.png";
    int clampedStars = std::max(0, std::min(3, stars));
    auto sampleStar = Sprite::create(filledStarPath);
    if (!sampleStar) {
        sampleStar = Sprite::create(emptyStarPath);
    }
    if (sampleStar) {
        float maxWidth = visibleSize.width * 0.45f;
        float maxHeight = visibleSize.height * 0.18f;
        Size size = sampleStar->getContentSize();
        float spacing = size.width * 0.35f;
        float totalWidth = size.width * 3 + spacing * 2;
        float scale = 1.0f;
        if (size.width > 0.0f && size.height > 0.0f) {
            scale = std::min(maxWidth / totalWidth, maxHeight / size.height);
        }
        float scaledWidth = size.width * scale;
        float scaledSpacing = spacing * scale;
        float startX = -(scaledWidth * 3 + scaledSpacing * 2) * 0.5f + scaledWidth * 0.5f;

        auto starGroup = Node::create();
        starGroup->setPosition(Vec2(
            origin.x + visibleSize.width * 0.5f,
            origin.y + visibleSize.height * 0.53f
        ));
        for (int i = 0; i < 3; ++i) {
            const char* starPath = (i < clampedStars) ? filledStarPath : emptyStarPath;
            auto starSprite = Sprite::create(starPath);
            if (!starSprite) {
                continue;
            }
            starSprite->setScale(scale);
            starSprite->setPosition(Vec2(startX + i * (scaledWidth + scaledSpacing), 0.0f));
            starGroup->addChild(starSprite, 1);
        }
        _resultLayer->addChild(starGroup, 1);
    }

    createResultButtons(_resultLayer, isWin);
}

void BattleScene::createResultButtons(Node* parent, bool isWin) {
    if (!parent) {
        return;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto applyStyle = [](Button* button) {
        if (!button) return;
        button->setPressedActionEnabled(true);
        button->setZoomScale(0.06f);
        button->setSwallowTouches(true);
    };

    auto exitBtn = Button::create("UI/exit.png", "UI/exit.png");
    auto retryBtn = Button::create("UI/retry.png", "UI/retry.png");
    auto nextBtn = Button::create("UI/next.png", "UI/next.png");

    float centerX = origin.x + visibleSize.width * 0.5f;
    float baseY = origin.y + visibleSize.height * 0.22f;
    float spacing = 120.0f;
    float buttonScale = 1.35f;

    if (exitBtn) {
        exitBtn->setScale(buttonScale);
        exitBtn->setPosition(Vec2(centerX - spacing, baseY));
        applyStyle(exitBtn);
        exitBtn->addClickEventListener([this](Ref*) {
            AudioManager::playButtonCancel();
            SaveManager::getInstance()->saveActiveSlot();
            auto scene = BaseScene::createScene();
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });
        parent->addChild(exitBtn, 2);
    }

    if (retryBtn) {
        retryBtn->setScale(buttonScale);
        retryBtn->setPosition(Vec2(centerX, baseY));
        applyStyle(retryBtn);
        retryBtn->addClickEventListener([this](Ref*) {
            AudioManager::playButtonClick();
            SaveManager::getInstance()->saveActiveSlot();
            bool defenseMode = (_battleMode == BattleMode::Defense);
            int retryLevel = defenseMode ? getRecordLevelId() : _levelId;
            auto scene = BattleScene::createScene(retryLevel, _deployableUnits, _allowDefaultUnits, defenseMode);
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });
        parent->addChild(retryBtn, 2);
    }

    if (nextBtn) {
        nextBtn->setScale(buttonScale);
        nextBtn->setPosition(Vec2(centerX + spacing, baseY));
        applyStyle(nextBtn);
        bool defenseMode = (_battleMode == BattleMode::Defense);
        int currentLevelId = defenseMode ? getRecordLevelId() : _levelId;
        int maxLevelId = defenseMode ? (kDefenseLevelOffset + kDefenseMaxLevelId) : kMaxLevelId;
        bool canNext = isWin && currentLevelId < maxLevelId;
        if (!canNext) {
            nextBtn->setEnabled(false);
            nextBtn->setBright(false);
            nextBtn->setOpacity(130);
        }
        nextBtn->addClickEventListener([this](Ref*) {
            AudioManager::playButtonClick();
            SaveManager::getInstance()->saveActiveSlot();
            bool defenseMode = (_battleMode == BattleMode::Defense);
            int currentLevelId = defenseMode ? getRecordLevelId() : _levelId;
            int maxLevelId = defenseMode ? (kDefenseLevelOffset + kDefenseMaxLevelId) : kMaxLevelId;
            int nextLevel = currentLevelId + 1;
            if (nextLevel > maxLevelId) {
                auto scene = BaseScene::createScene();
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
                return;
            }
            auto scene = BattleScene::createScene(nextLevel, _deployableUnits, _allowDefaultUnits, defenseMode);
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });
        parent->addChild(nextBtn, 2);
    }
}

// ===================================================
// 退出按钮回调
// ===================================================

void BattleScene::onExitButton(Ref* sender) {
    CCLOG("[战斗场景] 退出战斗");

    SaveManager::getInstance()->saveActiveSlot();
    auto scene = BaseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}
