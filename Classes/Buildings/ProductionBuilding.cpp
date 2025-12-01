// ProductionBuilding.cpp
// 生产建筑类实现文件
// 功能: 实现资源生产和收集功能

#include "ProductionBuilding.h"
#include "../Core/ResourceManager.h"

// ==================== 创建和初始化 ====================

ProductionBuilding* ProductionBuilding::create(BuildingType type, Faction faction) {
    ProductionBuilding* building = new (std::nothrow) ProductionBuilding();
    if (building && building->initWithType(type, faction)) {
        building->autorelease();
        return building;
    }
    CC_SAFE_DELETE(building);
    return nullptr;
}

ProductionBuilding::ProductionBuilding()
    : _productionType(ResourceType::GOLD)
    , _productionRate(1.0f)
    , _productionTimer(0)
    , _currentStorage(0)
    , _maxStorage(500)
    , _lossRateOnDeath(0.5f)
    , _resourceIndicator(nullptr)
    , _resourceLabel(nullptr)
{
}

ProductionBuilding::~ProductionBuilding() {
}

bool ProductionBuilding::initWithType(BuildingType type, Faction faction) {
    if (!Building::initWithType(type, faction)) {
        return false;
    }
    
    // 初始化生产属性
    initProductionAttributes();
    
    // 创建资源指示器
    createResourceIndicator();
    
    return true;
}

// ==================== 生产属性初始化 ====================

void ProductionBuilding::initProductionAttributes() {
    switch (_buildingType) {
        case BuildingType::GOLD_MINE:
            _productionType = ResourceType::GOLD;
            _productionRate = 0.5f;  // 每秒0.5金币
            _maxStorage = 500;
            _lossRateOnDeath = 0.5f;
            break;
            
        case BuildingType::ELIXIR_COLLECTOR:
            _productionType = ResourceType::ELIXIR;
            _productionRate = 0.5f;  // 每秒0.5圣水
            _maxStorage = 500;
            _lossRateOnDeath = 0.5f;
            break;
            
        default:
            break;
    }
    
    // 根据等级调整产量和存储
    float levelMultiplier = 1.0f + (_level - 1) * 0.15f;
    _productionRate *= levelMultiplier;
    _maxStorage = (int)(_maxStorage * levelMultiplier);
}

// ==================== 资源指示器 ====================

void ProductionBuilding::createResourceIndicator() {
    _resourceIndicator = Node::create();
    
    // 创建资源图标背景
    auto bg = DrawNode::create();
    bg->drawSolidCircle(Vec2::ZERO, 12, 0, 20, 
                        _productionType == ResourceType::GOLD ? 
                        Color4F(1.0f, 0.84f, 0.0f, 0.8f) : 
                        Color4F(0.6f, 0.2f, 0.8f, 0.8f));
    _resourceIndicator->addChild(bg);
    
    // 创建数量标签
    _resourceLabel = Label::createWithSystemFont("0", "Arial", 10);
    _resourceLabel->setColor(Color3B::WHITE);
    _resourceLabel->setPosition(Vec2(0, -20));
    _resourceIndicator->addChild(_resourceLabel);
    
    // 设置位置
    float height = _gridHeight * GRID_SIZE;
    _resourceIndicator->setPosition(Vec2(0, height/2 + 25));
    _resourceIndicator->setVisible(false);
    
    this->addChild(_resourceIndicator, 99);
}

void ProductionBuilding::updateResourceIndicator() {
    if (!_resourceIndicator || !_resourceLabel) return;
    
    // 更新数量显示
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", _currentStorage);
    _resourceLabel->setString(buf);
    
    // 有资源时显示指示器
    _resourceIndicator->setVisible(_currentStorage > 0);
    
    // 根据填充比例调整大小
    float ratio = getStorageRatio();
    float scale = 0.8f + ratio * 0.4f;
    _resourceIndicator->setScale(scale);
}

// ==================== 资源生产和收集 ====================

int ProductionBuilding::collectResource() {
    if (_currentStorage <= 0) return 0;
    
    int collected = _currentStorage;
    
    // 添加到玩家资源
    int actualAdded = ResourceManager::getInstance()->addResource(_productionType, collected);
    
    // 清空储量
    _currentStorage = 0;
    updateResourceIndicator();
    
    // 播放收集动画
    auto scale1 = ScaleTo::create(0.1f, 1.1f);
    auto scale2 = ScaleTo::create(0.1f, 1.0f);
    this->runAction(Sequence::create(scale1, scale2, nullptr));
    
    return actualAdded;
}

int ProductionBuilding::takeDamage(int damage) {
    int result = Building::takeDamage(damage);
    
    // 如果被摧毁，损失部分资源
    if (_state == BuildingState::DESTROYED && _currentStorage > 0) {
        int lost = (int)(_currentStorage * _lossRateOnDeath);
        _currentStorage -= lost;
        updateResourceIndicator();
    }
    
    return result;
}

// ==================== 帧更新 ====================

void ProductionBuilding::update(float dt) {
    Building::update(dt);
    
    // 只有正常状态才生产资源
    if (_state != BuildingState::NORMAL) return;
    
    // 更新生产计时器
    _productionTimer += dt;
    
    // 每秒生产一次
    if (_productionTimer >= 1.0f) {
        _productionTimer -= 1.0f;
        
        // 生产资源(不超过最大储量)
        if (_currentStorage < _maxStorage) {
            int production = (int)_productionRate;
            if (production < 1) {
                // 对于产量小于1的情况，使用随机
                if (rand() % 100 < (int)(_productionRate * 100)) {
                    production = 1;
                } else {
                    production = 0;
                }
            }
            _currentStorage = std::min(_currentStorage + production, _maxStorage);
            updateResourceIndicator();
        }
    }
}
