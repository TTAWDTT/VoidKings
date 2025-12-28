# 第10章 战斗场景 BattleScene

> **本章目标**：全面理解战斗场景的实现，包括场景初始化、关卡设计、战斗逻辑、结算系统等。

---

## 10.1 场景创建与初始化

### 10.1.1 createScene() 工厂方法

```cpp
// Classes/Scenes/BattleScene.cpp
Scene* BattleScene::createScene(int levelId, 
                                 const std::map<int, int>& units,
                                 bool useDefaultUnits, 
                                 bool defenseMode) {
    auto scene = new (std::nothrow) BattleScene();
    if (scene) {
        scene->setLevelId(levelId);
        scene->setDeployableUnits(units);
        scene->_allowDefaultUnits = useDefaultUnits;
        scene->setBattleMode(defenseMode ? 
            BattleMode::Defense : BattleMode::Attack);
        
        if (scene->init()) {
            scene->autorelease();
            return scene;
        }
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}
```

### 10.1.2 战斗模式（攻击/防守）

```cpp
enum class BattleMode {
    Attack,   // 玩家进攻敌方基地
    Defense   // 玩家防守自己的基地
};

// 根据模式初始化不同内容
void BattleScene::initLevel() {
    if (_battleMode == BattleMode::Defense) {
        // 使用玩家基地布局
        createDefenseBaseLayout();
        // 生成敌方进攻波次
        setupDefenseWaves();
    } else {
        // 创建敌方基地
        switch (_levelId) {
            case 1: createLevel1(); break;
            case 2: createLevel2(); break;
            // ...
        }
    }
}
```

### 10.1.3 回放模式创建

```cpp
Scene* BattleScene::createReplayScene(const BattleReplay& replay) {
    auto scene = new (std::nothrow) BattleScene();
    if (scene) {
        scene->_isReplay = true;        // 标记为回放
        scene->_replayData = replay;    // 存储回放数据
        scene->_hasReplayData = true;
        scene->setLevelId(replay.levelId);
        scene->setDeployableUnits(replay.deployableUnits);
        scene->_allowDefaultUnits = replay.allowDefaultUnits;
        scene->setBattleMode(replay.defenseMode ? 
            BattleMode::Defense : BattleMode::Attack);
        
        if (scene->init()) {
            scene->autorelease();
            return scene;
        }
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}
```

### 10.1.4 快照模式创建

```cpp
Scene* BattleScene::createSnapshotScene(const BaseSnapshot& snapshot,
                                         const std::map<int, int>& units,
                                         bool useDefaultUnits) {
    auto scene = new (std::nothrow) BattleScene();
    if (scene) {
        scene->setLevelId(1);  // 快照模式不使用关卡ID
        scene->setDeployableUnits(units);
        scene->_allowDefaultUnits = useDefaultUnits;
        scene->setBattleMode(BattleMode::Attack);
        scene->_useSnapshotLayout = true;
        scene->_snapshotLayout = snapshot;
        
        if (scene->init()) {
            scene->autorelease();
            return scene;
        }
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}
```

---

## 10.2 网格地图初始化

### 10.2.1 地图缩放适配屏幕

```cpp
void BattleScene::initGridMap() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 创建网格地图
    _gridMap = GridMap::create(
        BattleConfig::GRID_WIDTH,   // 40 格
        BattleConfig::GRID_HEIGHT,  // 30 格
        BattleConfig::CELL_SIZE     // 32 像素/格
    );

    float mapWidth = GRID_WIDTH * CELL_SIZE;
    float mapHeight = GRID_HEIGHT * CELL_SIZE;

    // 计算可用空间（排除 UI 区域）
    float availableWidth = visibleSize.width;
    float availableHeight = visibleSize.height - 
        UI_TOP_HEIGHT - UI_BOTTOM_HEIGHT;

    // 计算缩放比例
    float scale = std::min(1.0f, 
        std::min(availableWidth / mapWidth, 
                 availableHeight / mapHeight));

    // 居中放置
    float scaledWidth = mapWidth * scale;
    float scaledHeight = mapHeight * scale;
    float offsetX = origin.x + (visibleSize.width - scaledWidth) / 2;
    float offsetY = origin.y + UI_BOTTOM_HEIGHT + 
        (availableHeight - scaledHeight) / 2;

    _gridMap->setPosition(Vec2(offsetX, offsetY));
    _gridMap->setScale(scale);
    this->addChild(_gridMap, 0);
}
```

### 10.2.2 背景贴图重复填充

```cpp
// 使用草地贴图平铺背景
auto tileBg = Sprite::create("grass/grass_0000.png");
if (tileBg) {
    auto texture = tileBg->getTexture();
    if (texture) {
        // 设置纹理重复模式
        Texture2D::TexParams params = { 
            GL_LINEAR, GL_LINEAR, 
            GL_REPEAT, GL_REPEAT 
        };
        texture->setTexParameters(params);
    }
    
    tileBg->setAnchorPoint(Vec2::ZERO);
    tileBg->setPosition(Vec2::ZERO);
    tileBg->setTextureRect(Rect(0, 0, mapWidth, mapHeight));
    tileBg->setOpacity(200);
    _gridMap->addChild(tileBg, -2);
}
```

### 10.2.3 部署范围提示绘制

```cpp
void BattleScene::setupDeployRangeHint() {
    if (!_gridMap) return;

    int deployWidth = DEPLOY_MAX_X - DEPLOY_MIN_X + 1;
    int deployHeight = DEPLOY_MAX_Y - DEPLOY_MIN_Y + 1;

    float cellSize = _gridMap->getCellSize();
    
    // 创建半透明绿色区域
    auto hintLayer = LayerColor::create(
        Color4B(80, 130, 90, 70),  // 淡绿色
        deployWidth * cellSize,
        deployHeight * cellSize
    );
    
    hintLayer->setAnchorPoint(Vec2(0.0f, 0.0f));
    hintLayer->setPosition(Vec2(
        DEPLOY_MIN_X * cellSize,
        DEPLOY_MIN_Y * cellSize
    ));
    
    _gridMap->addChild(hintLayer, -1);
}
```

---

## 10.3 关卡初始化与敌方布局

### 10.3.1 攻击关卡设计

```cpp
void BattleScene::createLevel1() {
    int towerLevel = resolveAttackTowerLevel(_levelId);
    
    // 创建敌方基地
    createEnemyBase(26, 12, towerLevel);

    // 创建防御塔
    createDefenseTower(16, 8, kTowerArrow, towerLevel);
    createDefenseTower(16, 18, kTowerArrow, towerLevel);
    createDefenseTower(18, 12, kTowerBoom, towerLevel);

    // 创建陷阱
    for (int x = 10; x <= 14; ++x) {
        createSpikeTrap(x, 13);
    }
    createSnapTrap(12, 10);
}
```

### 10.3.2 敌方基地创建

```cpp
void BattleScene::createEnemyBase(int gridX, int gridY, int level) {
    auto* manager = BuildingManager::getInstance();
    
    int baseId = manager->getEnemyBaseId();
    const auto* baseConfig = manager->getProductionConfig(baseId);
    int baseWidth = baseConfig ? baseConfig->width : 4;
    int baseHeight = baseConfig ? baseConfig->length : 4;

    auto base = manager->createProductionBuilding(baseId, level);
    if (base) {
        _buildingLayer->addChild(base);

        // 定位到格子中心
        float cellSize = _gridMap->getCellSize();
        float centerX = (gridX + baseWidth * 0.5f) * cellSize;
        float centerY = (gridY + baseHeight * 0.5f) * cellSize;
        base->setPosition(Vec2(centerX, centerY));

        // 缩放适配
        scaleBuildingToFit(base, baseWidth, baseHeight, cellSize);

        // 标记网格占用
        _gridMap->occupyCell(gridX, gridY, baseWidth, baseHeight, base);

        // 记录为敌方建筑
        _enemyBuildings.push_back(base);
        base->retain();
        _totalBuildingCount++;
        
        // 标记主基地
        _enemyBase = base;
        _enemyBaseDestroyed = false;
    }
}
```

---

## 10.4 UI 系统

### 10.4.1 顶部信息条

```cpp
void BattleScene::initUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _uiLayer = Node::create();
    this->addChild(_uiLayer, 100);

    // 顶部背景
    auto topBg = LayerColor::create(
        Color4B(30, 30, 30, 220),
        visibleSize.width,
        UI_TOP_HEIGHT
    );
    topBg->setPosition(origin.x, 
        origin.y + visibleSize.height - UI_TOP_HEIGHT);
    _uiLayer->addChild(topBg);

    float topY = origin.y + visibleSize.height - UI_TOP_HEIGHT / 2;

    // 计时器
    _timerLabel = Label::createWithTTF("3:00", BATTLE_FONT, 18);
    _timerLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, topY));
    _uiLayer->addChild(_timerLabel);

    // 进度显示
    _progressLabel = Label::createWithTTF("0%", BATTLE_FONT, 14);
    _progressLabel->setPosition(Vec2(origin.x + visibleSize.width - 60, topY));
    _progressLabel->setColor(Color3B::YELLOW);
    _uiLayer->addChild(_progressLabel);
}
```

### 10.4.2 暂停控制系统

```cpp
void BattleScene::setupPauseControls(float topY) {
    // 暂停按钮
    _pauseButton = Button::create();
    _pauseButton->setTitleText("Pause");
    _pauseButton->addClickEventListener([this](Ref*) {
        setPausedState(!_battlePaused);
    });
    _uiLayer->addChild(_pauseButton);

    // 暂停遮罩
    _pauseOverlay = LayerColor::create(
        Color4B(0, 0, 0, 140),
        visibleSize.width, visibleSize.height
    );
    _pauseOverlay->setVisible(false);
    _uiLayer->addChild(_pauseOverlay, 50);

    // 点击遮罩恢复
    auto swallow = EventListenerTouchOneByOne::create();
    swallow->setSwallowTouches(true);
    swallow->onTouchBegan = [this](Touch*, Event*) {
        return _battlePaused;
    };
    swallow->onTouchEnded = [this](Touch*, Event*) {
        setPausedState(false);
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(
        swallow, _pauseOverlay);
}

void BattleScene::setPausedState(bool paused) {
    if (_battleEnded && paused) return;
    _battlePaused = paused;

    if (_gridMap) {
        paused ? _gridMap->pause() : _gridMap->resume();
    }

    _pauseOverlay->setVisible(paused);
    _pauseButton->setTitleText(paused ? "Resume" : "Pause");
}
```

---

## 10.5 士兵部署流程

### 10.5.1 触摸检测与范围限制

```cpp
bool BattleScene::onTouchBegan(Touch* touch, Event* event) {
    // 回放模式禁止输入
    if (_isReplay || _battlePaused) return false;

    Vec2 touchPos = touch->getLocation();

    // 检查 UI 区域
    if (touchPos.y < UI_BOTTOM_HEIGHT || 
        touchPos.y > visibleSize.height - UI_TOP_HEIGHT) {
        return false;
    }

    // 转换到地图坐标
    Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
    Vec2 gridPos = _gridMap->worldToGrid(localPos);
    int gridX = static_cast<int>(gridPos.x);
    int gridY = static_cast<int>(gridPos.y);

    // 检查部署范围
    if (!isDeployGridAllowed(gridX, gridY)) {
        return false;
    }

    // 部署士兵
    if (_selectedUnitId != -1) {
        deploySoldier(_selectedUnitId, localPos);
    }

    return true;
}

bool BattleScene::isDeployGridAllowed(int gridX, int gridY) const {
    return gridX >= DEPLOY_MIN_X && gridX <= DEPLOY_MAX_X &&
           gridY >= DEPLOY_MIN_Y && gridY <= DEPLOY_MAX_Y;
}
```

### 10.5.2 部署特效生成

```cpp
void BattleScene::spawnDeployEffect(const Vec2& position) {
    // 创建冲击环
    auto ring = DrawNode::create();
    ring->drawCircle(Vec2::ZERO, 14.0f, 0.0f, 20, false, 
                     Color4F(0.8f, 1.0f, 0.8f, 0.8f));
    ring->setPosition(position);
    _gridMap->addChild(ring, 30);

    // 动画：放大 + 淡出
    auto scale = ScaleTo::create(0.25f, 1.6f);
    auto fade = FadeTo::create(0.25f, 0);
    ring->runAction(Sequence::create(
        Spawn::create(scale, fade, nullptr), 
        RemoveSelf::create(), 
        nullptr
    ));
}
```

---

## 10.6 战斗逻辑更新

### 10.6.1 updateBattle(float dt) 主循环

```cpp
void BattleScene::update(float dt) {
    if (_battleEnded || _battlePaused) return;

    // 更新战斗时间
    _battleTime += dt;

    // 更新计时器
    float remaining = std::max(0.0f, BATTLE_TIME_LIMIT - _battleTime);
    _timerLabel->setString(formatTimeText(remaining));

    // 回放模式：按时间戳部署
    if (_isReplay) {
        updateReplayPlayback();
    }

    // 更新战斗
    updateBattle(dt);

    // 检查结束条件
    checkBattleEnd();
}

void BattleScene::updateBattle(float dt) {
    // 防守模式：生成敌军
    if (_battleMode == BattleMode::Defense) {
        updateDefenseSpawns();
    }

    // 清理死亡士兵
    for (auto& soldier : _soldiers) {
        if (soldier && !soldier->getParent()) {
            _deadSoldierCount++;
            soldier->release();
            soldier = nullptr;
        }
    }

    // 清理被摧毁的建筑
    int destroyed = 0;
    for (auto& building : _enemyBuildings) {
        if (building && !building->getParent()) {
            if (building == _enemyBase) {
                _enemyBaseDestroyed = true;
            }
            building->release();
            building = nullptr;
        }
        if (!building) destroyed++;
    }
    _destroyedBuildingCount = destroyed;

    // 更新进度
    int progress = _totalBuildingCount > 0 ?
        (destroyed * 100 / _totalBuildingCount) : 0;
    _progressLabel->setString(StringUtils::format("%d%%", progress));
}
```

---

## 10.7 战斗结束判定

### 10.7.1 星级计算算法

```cpp
int BattleScene::calculateStarCount() const {
    if (_battleMode == BattleMode::Defense) {
        if (_enemyBaseDestroyed) return 0;  // 基地被摧毁
        
        float destroyedRatio = (float)_destroyedBuildingCount / 
                               _totalBuildingCount;
        if (destroyedRatio <= 0.2f) return 3;
        if (destroyedRatio <= 0.5f) return 2;
        return 1;
    }

    // 攻击模式：根据阵亡比例评星
    if (_totalDeployedCount <= 0) return 1;
    
    float ratio = (float)_deadSoldierCount / _totalDeployedCount;
    if (ratio <= 0.34f) return 3;  // 阵亡 < 34%
    if (ratio <= 0.67f) return 2;  // 阵亡 < 67%
    return 1;
}
```

### 10.7.2 奖励计算公式

```cpp
void BattleScene::onBattleWin() {
    _battleEnded = true;

    // 计算奖励系数
    float destroyedRatio = (float)_destroyedBuildingCount / 
                           _totalBuildingCount;
    float remainingRatio = std::max(0.0f, 
        BATTLE_TIME_LIMIT - _battleTime) / BATTLE_TIME_LIMIT;

    float rewardFactor = 0.75f + destroyedRatio * 0.35f + 
                         remainingRatio * 0.15f;
    if (_enemyBaseDestroyed) rewardFactor += 0.1f;
    rewardFactor = std::min(1.6f, rewardFactor);

    // 基础奖励
    int baseCoin = 180 + _levelId * 120;
    int baseDiamond = 4 + _levelId * 2;

    // 最终奖励
    int rewardCoin = static_cast<int>(baseCoin * rewardFactor);
    int rewardDiamond = static_cast<int>(baseDiamond * rewardFactor);

    Core::getInstance()->addResource(ResourceType::COIN, rewardCoin);
    Core::getInstance()->addResource(ResourceType::DIAMOND, rewardDiamond);

    // 记录星级
    int stars = calculateStarCount();
    Core::getInstance()->setLevelStars(_levelId, stars);

    showBattleResult(true, stars);
}
```

---

## 10.8 悬浮信息系统

### 10.8.1 建筑信息面板

```cpp
void BattleScene::showBuildingInfo(Node* building) {
    std::string name, attackText, rangeText;
    int level; float hp, maxHp;

    if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
        name = defence->getName();
        level = defence->getLevel() + 1;
        hp = defence->getCurrentHP();
        maxHp = defence->getCurrentMaxHP();
        attackText = StringUtils::format("%.0f", defence->getCurrentATK());
        rangeText = StringUtils::format("%.0f", defence->getCurrentATK_RANGE());
    }

    // 构建信息文本
    std::string infoText = StringUtils::format(
        "Name: %s\nLevel: %d\nHP: %.0f / %.0f\nATK: %s\nRange: %s",
        name.c_str(), level, hp, maxHp, 
        attackText.c_str(), rangeText.c_str()
    );

    _hoverInfoLabel->setString(infoText);
    _hoverInfoPanel->setVisible(true);
    
    updateHoverOverlays(building);
}
```

### 10.8.2 攻击范围圆绘制

```cpp
void BattleScene::updateHoverOverlays(Node* building) {
    _hoverRangeNode->clear();

    if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
        float range = defence->getCurrentATK_RANGE();
        Vec2 center = building->getPosition();

        // 绘制虚线圆
        NodeUtils::drawDashedCircle(_hoverRangeNode, center, range,
            Color4F(0.85f, 0.85f, 0.85f, 0.8f));
        _hoverRangeNode->setVisible(true);
    }
}
```

---

## 本章小结

1. **BattleScene** 是战斗系统的核心，管理所有战斗逻辑
2. **多种创建模式**：普通关卡、回放、快照
3. **网格地图** 自适应缩放以适配不同屏幕
4. **部署系统** 限制玩家只能在指定区域部署
5. **星级系统** 根据阵亡比例评定
6. **悬浮信息** 提供实时建筑状态查看

---

## 练习题

1. 添加新关卡：设计第 13 关的敌方布局
2. 实现加速按钮：2倍速战斗
3. 添加战斗统计：显示总伤害输出

---

**下一章**：[第11章 UI 组件系统](第11章-UI组件系统.md)
