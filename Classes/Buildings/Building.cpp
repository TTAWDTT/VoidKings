// Building.cpp
// 建筑基类实现文件
// 功能: 实现建筑基类的所有方法

#include "Building.h"
#include "../Core/ResourceManager.h"

// 线性增长：HP 默认增长系数（沿用 GameDefines 中 LEVEL_HP_MULTIPLIER）
float Building::s_hpGrowthK = LEVEL_HP_MULTIPLIER;

// ==================== 创建和初始化 ====================

Building* Building::create(BuildingType type, Faction faction) {
    Building* building = new (std::nothrow) Building();
    if (building && building->initWithType(type, faction)) {
        building->autorelease();
        return building;
    }
    CC_SAFE_DELETE(building);
    return nullptr;
}

Building::Building()
    : _buildingType(BuildingType::TOWN_HALL)
    , _faction(Faction::PLAYER)
    , _state(BuildingState::NORMAL)
    , _buildingName("Building")
    , _currentHP(100)
    , _maxHP(100)
    , _level(1)
    , _maxLevel(10)
    , _gridWidth(2)
    , _gridHeight(2)
    , _gridPosition(Vec2::ZERO)
    , _buildTime(10)
    , _buildProgress(0)
    , _goldCost(100)
    , _elixirCost(0)
    , _refundRate(0.5f)
    , _sprite(nullptr)
    , _shadowSprite(nullptr)
    , _healthBar(nullptr)
    , _progressBar(nullptr)
    , _selectionIndicator(nullptr)
    , _isSelected(false)
    , _clickCallback(nullptr)
    , _destroyCallback(nullptr)
    , _progressValue(0)
    , _baseMaxHP(0)
{
}

Building::~Building() {
}

bool Building::init() {
    if (!Node::init()) {
        return false;
    }
    return true;
}

bool Building::initWithType(BuildingType type, Faction faction) {
    if (!init()) {
        return false;
    }
    
    _buildingType = type;
    _faction = faction;
    
    // 初始化建筑属性
    initAttributes();
    
    // 初始化精灵
    initSprite();
    
    // 创建血条
    createHealthBar();
    
    // 添加触摸监听
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(Building::onTouchBegan, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    // 启用帧更新
    scheduleUpdate();
    
    return true;
}

// ==================== 属性初始化 ====================

void Building::initAttributes() {
    // 根据建筑类型设置属性
    switch (_buildingType) {
        //市政厅（基地）
        case BuildingType::TOWN_HALL:
            _buildingName = "Town Hall";
            _gridWidth = 4;
            _gridHeight = 4;
            _maxHP = 1500;
            _goldCost = 0;
            _buildTime = 0;
            _maxLevel = 5;
            _progressValue = 40;
            break;
        //大炮    
        case BuildingType::CANNON:
            _buildingName = "Cannon";
            _gridWidth = 2;
            _gridHeight = 2;
            _maxHP = 400;
            _goldCost = 250;
            _buildTime = 60;
            _maxLevel = 5;
            _progressValue = 5;
            break;
        //弓箭塔    
        case BuildingType::ARCHER_TOWER:
            _buildingName = "Archer Tower";
            _gridWidth = 2;
            _gridHeight = 2;
            _maxHP = 380;
            _goldCost = 500;
            _buildTime = 120;
            _maxLevel = 5;
            _progressValue = 5;
            break;
        //迫击炮
        case BuildingType::MORTAR:
            _buildingName = "Mortar";
            _gridWidth = 2;
            _gridHeight = 2;
            _maxHP = 400;
            _goldCost = 5000;
            _buildTime = 300;
            _maxLevel = 5;  
            _progressValue = 5;
            break;
        //金矿    
        case BuildingType::GOLD_MINE:
            _buildingName = "Gold Mine";
            _gridWidth = 3;
            _gridHeight = 3;
            _maxHP = 400;
            _elixirCost = 150;
            _buildTime = 60;
            _maxLevel = 5;
            _progressValue = 10;
            break;
        //圣水收集器    
        case BuildingType::ELIXIR_COLLECTOR:
            _buildingName = "Elixir Collector";
            _gridWidth = 3;
            _gridHeight = 3;
            _maxHP = 400;
            _goldCost = 150;
            _buildTime = 60;
            _maxLevel = 5;
            _progressValue = 10;
            break;
        //金库    
        case BuildingType::GOLD_STORAGE:
            _buildingName = "Gold Storage";
            _gridWidth = 3;
            _gridHeight = 3;
            _maxHP = 600;
            _elixirCost = 300;
            _buildTime = 120;
            _maxLevel = 5;
            _progressValue = 10;
            break;
        //圣水库    
        case BuildingType::ELIXIR_STORAGE:
            _buildingName = "Elixir Storage";
            _gridWidth = 3;
            _gridHeight = 3;
            _maxHP = 600;
            _goldCost = 300;
            _buildTime = 120;
            _maxLevel = 5;
            _progressValue = 10;
            break;
        //兵营    
        case BuildingType::BARRACKS:
            _buildingName = "Barracks";
            _gridWidth = 3;
            _gridHeight = 3;
            _maxHP = 500;
            _elixirCost = 200;
            _buildTime = 180;
            _maxLevel = 5;
            _progressValue = 5;
            break;
        //法术工厂
        case BuildingType::SPELL_FACTORY:
            _buildingName = "Spell Factory";
            _gridWidth = 3;
            _gridHeight = 3;
            _maxHP = 500;
            _goldCost = 200;
            _buildTime = 240;
            _maxLevel = 5;
            _progressValue = 5;
            break;
        //城墙    
        case BuildingType::WALL:
            _buildingName = "Wall";
            _gridWidth = 1;
            _gridHeight = 1;
            _maxHP = 300;
            _goldCost = 50;
            _buildTime = 1;
            _maxLevel = 5;
            _progressValue = 0;
            break;
            
        default:
            break;
    }
    
    // 记录基础最大血量
    _baseMaxHP = _maxHP;
    // 应用线性增长：maxHP = base * (1 + k * (level-1))
    _maxHP = (int)(_baseMaxHP * (1.0f + s_hpGrowthK * (_level - 1)));
    // 设置当前血量为最大血量
    _currentHP = _maxHP;
}

void Building::initSprite() {
    // 根据建筑类型选择纹理
    std::string texturePath = "textures/buildings/";
    
    switch (_buildingType) {
        case BuildingType::TOWN_HALL:
            texturePath += "townhall.png";
            break;
        case BuildingType::CANNON:
            texturePath += "cannon.png";
            break;
        case BuildingType::ARCHER_TOWER:
            texturePath += "archer_tower.png";
            break;
        case BuildingType::MORTAR:
            texturePath += "mortar.png";
            break;
        case BuildingType::GOLD_MINE:
            texturePath += "gold_mine.png";
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            texturePath += "elixir_collector.png";
            break;
        case BuildingType::GOLD_STORAGE:
            texturePath += "gold_storage.png";
            break;
        case BuildingType::ELIXIR_STORAGE:
            texturePath += "elixir_storage.png";
            break;
        case BuildingType::BARRACKS:
            texturePath += "barracks.png";
            break;
        case BuildingType::SPELL_FACTORY:
            texturePath += "spell_factory.png";
            break;
        case BuildingType::WALL:
            texturePath += "wall.png";
            break;
        default:
            texturePath += "default.png";
            break;
    }
    
    // 尝试加载纹理，如果失败则使用占位图形
    _sprite = Sprite::create(texturePath);
    if (!_sprite) {
        // 创建占位矩形
        auto drawNode = DrawNode::create();
        float width = _gridWidth * GRID_SIZE;
        float height = _gridHeight * GRID_SIZE;
        
        // 根据建筑类型设置颜色
        Color4F color;
        switch (_buildingType) {
            case BuildingType::TOWN_HALL:
                color = Color4F(0.8f, 0.6f, 0.2f, 1.0f);  // 金色
                break;
            case BuildingType::CANNON:
            case BuildingType::ARCHER_TOWER:
            case BuildingType::MORTAR:
                color = Color4F(0.6f, 0.2f, 0.2f, 1.0f);  // 红色
                break;
            case BuildingType::GOLD_MINE:
            case BuildingType::GOLD_STORAGE:
                color = Color4F(1.0f, 0.84f, 0.0f, 1.0f); // 金色
                break;
            case BuildingType::ELIXIR_COLLECTOR:
            case BuildingType::ELIXIR_STORAGE:
                color = Color4F(0.6f, 0.2f, 0.8f, 1.0f);  // 紫色
                break;
            case BuildingType::BARRACKS:
            case BuildingType::SPELL_FACTORY:
                color = Color4F(0.2f, 0.6f, 0.2f, 1.0f);  // 绿色
                break;
            case BuildingType::WALL:
                color = Color4F(0.5f, 0.5f, 0.5f, 1.0f);  // 灰色
                break;
            default:
                color = Color4F(0.4f, 0.4f, 0.4f, 1.0f);
                break;
        }
        
        // 绘制建筑矩形
        Vec2 vertices[] = {
            Vec2(-width/2, -height/2),
            Vec2(width/2, -height/2),
            Vec2(width/2, height/2),
            Vec2(-width/2, height/2)
        };
        drawNode->drawPolygon(vertices, 4, color, 2, Color4F::BLACK);
        
        // 添加建筑名称标签
        auto label = Label::createWithSystemFont(_buildingName, "Arial", 12);
        label->setPosition(Vec2(0, 0));
        label->setColor(Color3B::WHITE);
        drawNode->addChild(label);
        
        this->addChild(drawNode, 1);
        
        // 设置内容大小
        this->setContentSize(Size(width, height));
    } else {
        // 调整精灵大小以匹配格子
        float width = _gridWidth * GRID_SIZE;
        float height = _gridHeight * GRID_SIZE;
        _sprite->setContentSize(Size(width, height));
        this->addChild(_sprite, 1);
        this->setContentSize(Size(width, height));
    }
    
    // 创建选中指示器
    _selectionIndicator = Sprite::create();
    if (!_selectionIndicator) {
        auto selectNode = DrawNode::create();
        float width = _gridWidth * GRID_SIZE;
        float height = _gridHeight * GRID_SIZE;
        Vec2 vertices[] = {
            Vec2(-width/2 - 2, -height/2 - 2),
            Vec2(width/2 + 2, -height/2 - 2),
            Vec2(width/2 + 2, height/2 + 2),
            Vec2(-width/2 - 2, height/2 + 2)
        };
        selectNode->drawPolygon(vertices, 4, Color4F(0, 0, 0, 0), 3, Color4F::GREEN);
        selectNode->setVisible(false);
        this->addChild(selectNode, 0);
        _selectionIndicator = (Sprite*)selectNode;
    }
}

// ==================== 血条管理 ====================

void Building::createHealthBar() {
    // 创建血条背景
    float width = _gridWidth * GRID_SIZE;
    float barWidth = width * 0.8f;
    float barHeight = 6;
    
    _healthBar = Node::create();
    
    // 背景条
    auto bgBar = DrawNode::create();
    bgBar->drawSolidRect(Vec2(-barWidth/2, -barHeight/2), 
                         Vec2(barWidth/2, barHeight/2), 
                         Color4F(0.3f, 0.3f, 0.3f, 0.8f));
    bgBar->setName("bgBar");
    _healthBar->addChild(bgBar);
    
    // 血量条
    auto hpBar = DrawNode::create();
    hpBar->drawSolidRect(Vec2(-barWidth/2 + 1, -barHeight/2 + 1), 
                        Vec2(barWidth/2 - 1, barHeight/2 - 1), 
                        Color4F(0.2f, 0.8f, 0.2f, 1.0f));
    hpBar->setName("hpBar");
    _healthBar->addChild(hpBar);
    
    // 设置血条位置在建筑上方
    float height = _gridHeight * GRID_SIZE;
    _healthBar->setPosition(Vec2(0, height/2 + 10));
    _healthBar->setVisible(false);  // 默认隐藏，受伤时显示
    
    this->addChild(_healthBar, 100);
}

void Building::updateHealthBar() {
    if (!_healthBar) return;
    
    // 计算血量比例
    float hpRatio = (float)_currentHP / (float)_maxHP;
    hpRatio = std::max(0.0f, std::min(1.0f, hpRatio));
    
    // 更新血条颜色和长度
    auto hpBar = dynamic_cast<DrawNode*>(_healthBar->getChildByName("hpBar"));
    if (hpBar) {
        hpBar->clear();
        
        float width = _gridWidth * GRID_SIZE;
        float barWidth = width * 0.8f;
        float barHeight = 6;
        
        // 根据血量比例设置颜色
        Color4F hpColor;
        if (hpRatio > 0.6f) {
            hpColor = Color4F(0.2f, 0.8f, 0.2f, 1.0f);  // 绿色
        } else if (hpRatio > 0.3f) {
            hpColor = Color4F(0.8f, 0.8f, 0.2f, 1.0f);  // 黄色
        } else {
            hpColor = Color4F(0.8f, 0.2f, 0.2f, 1.0f);  // 红色
        }
        
        // 绘制血条
        float actualWidth = (barWidth - 2) * hpRatio;
        hpBar->drawSolidRect(Vec2(-barWidth/2 + 1, -barHeight/2 + 1), 
                            Vec2(-barWidth/2 + 1 + actualWidth, barHeight/2 - 1), 
                            hpColor);
    }
    
    // 显示血条
    _healthBar->setVisible(true);
}

// ==================== 进度条管理 ====================

void Building::createProgressBar() {
    if (_progressBar) {
        _progressBar->removeFromParent();
    }
    
    float width = _gridWidth * GRID_SIZE;
    float barWidth = width * 0.8f;
    float barHeight = 8;
    
    _progressBar = Node::create();
    
    // 背景条
    auto bgBar = DrawNode::create();
    bgBar->drawSolidRect(Vec2(-barWidth/2, -barHeight/2), 
                         Vec2(barWidth/2, barHeight/2), 
                         Color4F(0.2f, 0.2f, 0.2f, 0.8f));
    _progressBar->addChild(bgBar);
    
    // 进度条
    auto progBar = DrawNode::create();
    progBar->setName("progBar");
    _progressBar->addChild(progBar);
    
    // 设置位置
    float height = _gridHeight * GRID_SIZE;
    _progressBar->setPosition(Vec2(0, height/2 + 20));
    
    this->addChild(_progressBar, 101);
}

void Building::updateProgressBar(float progress) {
    if (!_progressBar) return;
    
    auto progBar = dynamic_cast<DrawNode*>(_progressBar->getChildByName("progBar"));
    if (progBar) {
        progBar->clear();
        
        float width = _gridWidth * GRID_SIZE;
        float barWidth = width * 0.8f;
        float barHeight = 8;
        
        float actualWidth = (barWidth - 2) * progress;
        progBar->drawSolidRect(Vec2(-barWidth/2 + 1, -barHeight/2 + 1), 
                              Vec2(-barWidth/2 + 1 + actualWidth, barHeight/2 - 1), 
                              Color4F(0.2f, 0.6f, 1.0f, 1.0f));
    }
}

// ==================== 位置管理 ====================

void Building::setGridPosition(const Vec2& gridPos) {
    _gridPosition = gridPos;
    // 转换为实际坐标
    float x = gridPos.x * GRID_SIZE + (_gridWidth * GRID_SIZE) / 2;
    float y = gridPos.y * GRID_SIZE + (_gridHeight * GRID_SIZE) / 2;
    this->setPosition(Vec2(x, y));
}

void Building::setGridPosition(int gridX, int gridY) {
    setGridPosition(Vec2(gridX, gridY));
}

// ==================== 伤害和修复 ====================

int Building::takeDamage(int damage) {
    if (_state == BuildingState::DESTROYED) return 0;
    
    _currentHP -= damage;
    
    if (_currentHP <= 0) {
        _currentHP = 0;
        _state = BuildingState::DESTROYED;
        playDestroyAnimation();
        onDestroyed();
    }
    
    updateHealthBar();
    return _currentHP;
}

void Building::repair(int amount) {
    if (_state == BuildingState::DESTROYED) return;
    
    _currentHP += amount;
    if (_currentHP > _maxHP) {
        _currentHP = _maxHP;
    }
    
    updateHealthBar();
}

// ==================== 建造和升级 ====================

void Building::startBuilding() {
    _state = BuildingState::BUILDING;
    _buildProgress = 0;
    createProgressBar();
    playBuildAnimation();
}

void Building::finishBuilding() {
    _state = BuildingState::NORMAL;
    _buildProgress = 1.0f;
    
    if (_progressBar) {
        _progressBar->removeFromParent();
        _progressBar = nullptr;
    }
    
    // 释放工人
    ResourceManager::getInstance()->releaseWorker();
}

bool Building::startUpgrade() {
    if (_level >= _maxLevel) return false;
    if (_state != BuildingState::NORMAL) return false;
    
    // 检查资源是否足够
    auto cost = getUpgradeCost();
    if (!ResourceManager::getInstance()->hasEnoughResources(cost.first, cost.second)) {
        return false;
    }
    
    // 检查是否有空闲工人
    if (!ResourceManager::getInstance()->useWorker()) {
        return false;
    }
    
    // 消耗资源
    ResourceManager::getInstance()->consumeResources(cost.first, cost.second);
    
    _state = BuildingState::UPGRADING;
    _buildProgress = 0;
    createProgressBar();
    playUpgradeAnimation();
    
    return true;
}

void Building::finishUpgrade() {
    _level++;
    _state = BuildingState::NORMAL;
    _buildProgress = 1.0f;
    
    // 按线性曲线更新最大血量
    _maxHP = (int)(_baseMaxHP * (1.0f + s_hpGrowthK * (_level - 1)));
    _currentHP = _maxHP;
    
    if (_progressBar) {
        _progressBar->removeFromParent();
        _progressBar = nullptr;
    }
    
    // 释放工人
    ResourceManager::getInstance()->releaseWorker();
}

// ==================== 曲线配置接口 ====================

void Building::setHpGrowthFactor(float k) {
    s_hpGrowthK = k;
}

float Building::getHpGrowthFactor() {
    return s_hpGrowthK;
}

void Building::cancelConstruction() {
    if (_state != BuildingState::BUILDING && _state != BuildingState::UPGRADING) {
        return;
    }
    
    // 返还部分资源
    auto cost = (_state == BuildingState::BUILDING) ? getBuildCost() : getUpgradeCost();
    int refundGold = (int)(cost.first * _refundRate * (1 - _buildProgress));
    int refundElixir = (int)(cost.second * _refundRate * (1 - _buildProgress));
    
    ResourceManager::getInstance()->addResource(ResourceType::GOLD, refundGold);
    ResourceManager::getInstance()->addResource(ResourceType::ELIXIR, refundElixir);
    
    // 释放工人
    ResourceManager::getInstance()->releaseWorker();
    
    if (_state == BuildingState::BUILDING) {
        // 建造中取消，移除建筑
        this->removeFromParent();
    } else {
        // 升级中取消，恢复正常状态
        _state = BuildingState::NORMAL;
        _buildProgress = 0;
        if (_progressBar) {
            _progressBar->removeFromParent();
            _progressBar = nullptr;
        }
    }
}

std::pair<int, int> Building::demolish() {
    int refundGold = (int)(_goldCost * _refundRate);
    int refundElixir = (int)(_elixirCost * _refundRate);
    
    _state = BuildingState::DESTROYED;
    playDestroyAnimation();
    
    return std::make_pair(refundGold, refundElixir);
}

// ==================== 资源获取 ====================

std::pair<int, int> Building::getUpgradeCost() const {
    // 升级费用按等级增长
    float multiplier = 1.0f + (_level * 0.5f);
    return std::make_pair((int)(_goldCost * multiplier), (int)(_elixirCost * multiplier));
}

int Building::getUpgradeTime() const {
    // 升级时间按等级增长
    return (int)(_buildTime * (1.0f + _level * 0.3f));
}

std::pair<int, int> Building::getBuildCost() const {
    return std::make_pair(_goldCost, _elixirCost);
}

int Building::getBuildTime() const {
    return _buildTime;
}

// ==================== 动画效果 ====================

void Building::playBuildAnimation() {
    // 建造动画：渐显效果
    this->setOpacity(0);
    this->runAction(FadeIn::create(0.5f));
}

void Building::playDestroyAnimation() {
    // 摧毁动画：缩放+渐隐
    auto scale = ScaleTo::create(0.3f, 0.5f);
    auto fade = FadeOut::create(0.3f);
    auto spawn = Spawn::create(scale, fade, nullptr);
    auto callback = CallFunc::create([this]() {
        this->removeFromParent();
    });
    this->runAction(Sequence::create(spawn, callback, nullptr));
}

void Building::playUpgradeAnimation() {
    // 升级动画：闪烁效果
    auto blink = Blink::create(1.0f, 3);
    this->runAction(blink);
}

// ==================== 选中效果 ====================

void Building::setSelected(bool selected) {
    _isSelected = selected;
    if (_selectionIndicator) {
        _selectionIndicator->setVisible(selected);
    }
}

void Building::showRange(bool show) {
    // 基类不显示攻击范围，由防御建筑子类实现
}

// ==================== 回调处理 ====================

void Building::onDestroyed() {
    if (_destroyCallback) {
        _destroyCallback(this);
    }
}

bool Building::onTouchBegan(Touch* touch, Event* event) {
    Vec2 touchLocation = touch->getLocation();
    Vec2 localLocation = this->convertToNodeSpace(touchLocation);
    
    // 检查是否点击在建筑范围内
    Size size = this->getContentSize();
    Rect rect = Rect(-size.width/2, -size.height/2, size.width, size.height);
    
    if (rect.containsPoint(localLocation)) {
        if (_clickCallback) {
            _clickCallback(this);
        }
        return true;
    }
    return false;
}

// ==================== 帧更新 ====================

void Building::update(float dt) {
    // 更新建造/升级进度
    if (_state == BuildingState::BUILDING || _state == BuildingState::UPGRADING) {
        int totalTime = (_state == BuildingState::BUILDING) ? _buildTime : getUpgradeTime();
        _buildProgress += dt / totalTime;
        
        if (_buildProgress >= 1.0f) {
            _buildProgress = 1.0f;
            if (_state == BuildingState::BUILDING) {
                finishBuilding();
            } else {
                finishUpgrade();
            }
        } else {
            updateProgressBar(_buildProgress);
        }
    }
}
