// StorageBuilding.cpp
// 存储建筑类实现文件
// 功能: 实现资源存储容量管理

#include "StorageBuilding.h"
#include "../Core/ResourceManager.h"

// ==================== 创建和初始化 ====================

StorageBuilding* StorageBuilding::create(BuildingType type, Faction faction) {
    StorageBuilding* building = new (std::nothrow) StorageBuilding();
    if (building && building->initWithType(type, faction)) {
        building->autorelease();
        return building;
    }
    CC_SAFE_DELETE(building);
    return nullptr;
}

StorageBuilding::StorageBuilding()
    : _storageType(ResourceType::GOLD)
    , _capacity(1000)
    , _previousCapacity(0)
    , _lossRateOnDeath(0.3f)
{
}

StorageBuilding::~StorageBuilding() {
}

bool StorageBuilding::initWithType(BuildingType type, Faction faction) {
    if (!Building::initWithType(type, faction)) {
        return false;
    }
    
    // 初始化存储属性
    initStorageAttributes();
    
    return true;
}

// ==================== 存储属性初始化 ====================

void StorageBuilding::initStorageAttributes() {
    switch (_buildingType) {
        case BuildingType::GOLD_STORAGE:
            _storageType = ResourceType::GOLD;
            _capacity = 1500;
            _lossRateOnDeath = 0.3f;
            break;
            
        case BuildingType::ELIXIR_STORAGE:
            _storageType = ResourceType::ELIXIR;
            _capacity = 1500;
            _lossRateOnDeath = 0.3f;
            break;
            
        default:
            break;
    }
    
    // 根据等级调整容量
    float levelMultiplier = 1.0f + (_level - 1) * 0.25f;
    _capacity = (int)(_capacity * levelMultiplier);
    _previousCapacity = _capacity;
}

// ==================== 建造和升级完成 ====================

void StorageBuilding::finishBuilding() {
    Building::finishBuilding();
    
    // 增加全局存储容量
    updateGlobalCapacity();
}

void StorageBuilding::finishUpgrade() {
    // 保存升级前的容量
    _previousCapacity = _capacity;
    
    // 调用父类升级
    Building::finishUpgrade();
    
    // 重新计算容量
    float levelMultiplier = 1.0f + (_level - 1) * 0.25f;
    _capacity = (int)(1500 * levelMultiplier);  // 使用基础值重新计算
    
    // 更新全局存储容量
    updateGlobalCapacity();
}

void StorageBuilding::updateGlobalCapacity() {
    // 计算容量增量
    int capacityIncrease = _capacity - _previousCapacity;
    
    // 更新全局最大存储量
    int currentMax = ResourceManager::getInstance()->getMaxResource(_storageType);
    ResourceManager::getInstance()->setMaxResource(_storageType, currentMax + capacityIncrease);
    
    _previousCapacity = _capacity;
}

// ==================== 伤害处理 ====================

int StorageBuilding::takeDamage(int damage) {
    int result = Building::takeDamage(damage);
    
    // 如果被摧毁，减少全局存储容量
    if (_state == BuildingState::DESTROYED) {
        int currentMax = ResourceManager::getInstance()->getMaxResource(_storageType);
        int newMax = std::max(0, currentMax - _capacity);
        ResourceManager::getInstance()->setMaxResource(_storageType, newMax);
        
        // 损失部分资源
        int currentResource = ResourceManager::getInstance()->getResource(_storageType);
        int loss = (int)(currentResource * _lossRateOnDeath);
        ResourceManager::getInstance()->consumeResource(_storageType, loss);
    }
    
    return result;
}
