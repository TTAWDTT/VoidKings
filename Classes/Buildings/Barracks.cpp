// Barracks.cpp
// 兵营类实现文件
// 功能: 实现兵种训练和队列管理

#include "Barracks.h"
#include "../Core/ResourceManager.h"

// ==================== 创建和初始化 ====================

Barracks* Barracks::create(Faction faction) {
    Barracks* barracks = new (std::nothrow) Barracks();
    if (barracks && barracks->initWithType(BuildingType::BARRACKS, faction)) {
        barracks->autorelease();
        return barracks;
    }
    CC_SAFE_DELETE(barracks);
    return nullptr;
}

Barracks::Barracks()
    : _queueCapacity(5)
    , _trainingProgress(0)
    , _trainingTimer(0)
    , _currentTrainTime(0)
{
}

Barracks::~Barracks() {
    _availableUnits.clear();
    _trainingQueue.clear();
    _trainedUnits.clear();
}

bool Barracks::initWithType(BuildingType type, Faction faction) {
    if (!Building::initWithType(type, faction)) {
        return false;
    }
    
    // 初始化兵营属性
    initBarracksAttributes();
    
    return true;
}

// ==================== 兵营属性初始化 ====================

void Barracks::initBarracksAttributes() {
    // 根据等级解锁兵种
    _availableUnits.clear();
    
    // 等级1: 野蛮人
    _availableUnits.push_back(UnitType::BARBARIAN);
    
    // 等级2: 弓箭手
    if (_level >= 2) {
        _availableUnits.push_back(UnitType::ARCHER);
    }
    
    // 等级3: 巨人
    if (_level >= 3) {
        _availableUnits.push_back(UnitType::GIANT);
    }
    
    // 等级4: 哥布林
    if (_level >= 4) {
        _availableUnits.push_back(UnitType::GOBLIN);
    }
    
    // 等级5: 法师
    if (_level >= 5) {
        _availableUnits.push_back(UnitType::WIZARD);
    }
    
    // 队列容量随等级增加
    _queueCapacity = 5 + _level;
}

// ==================== 兵种数据 ====================

int Barracks::getTrainTime(UnitType type) const {
    switch (type) {
        case UnitType::BARBARIAN: return 20;   // 20秒
        case UnitType::ARCHER:    return 25;   // 25秒
        case UnitType::GIANT:     return 120;  // 2分钟
        case UnitType::GOBLIN:    return 30;   // 30秒
        case UnitType::WIZARD:    return 180;  // 3分钟
        case UnitType::HEALER:    return 300;  // 5分钟
        default: return 30;
    }
}

std::pair<int, int> Barracks::getTrainCost(UnitType type) const {
    // 返回(金币, 圣水)
    switch (type) {
        case UnitType::BARBARIAN: return {25, 0};
        case UnitType::ARCHER:    return {50, 0};
        case UnitType::GIANT:     return {250, 0};
        case UnitType::GOBLIN:    return {25, 0};
        case UnitType::WIZARD:    return {0, 1500};
        case UnitType::HEALER:    return {0, 2500};
        default: return {50, 0};
    }
}

int Barracks::getUnitPopulation(UnitType type) const {
    switch (type) {
        case UnitType::BARBARIAN: return 1;
        case UnitType::ARCHER:    return 1;
        case UnitType::GIANT:     return 5;
        case UnitType::GOBLIN:    return 1;
        case UnitType::WIZARD:    return 4;
        case UnitType::HEALER:    return 14;
        default: return 1;
    }
}

// ==================== 训练接口 ====================

std::vector<UnitType> Barracks::getAvailableUnits() const {
    return _availableUnits;
}

bool Barracks::trainUnit(UnitType type) {
    // 检查兵营状态
    if (_state != BuildingState::NORMAL) return false;
    
    // 检查队列容量
    if ((int)_trainingQueue.size() >= _queueCapacity) return false;
    
    // 检查兵种是否可训练
    bool available = false;
    for (auto& unit : _availableUnits) {
        if (unit == type) {
            available = true;
            break;
        }
    }
    if (!available) return false;
    
    // 检查人口
    int pop = getUnitPopulation(type);
    if (!ResourceManager::getInstance()->usePopulation(pop)) {
        return false;
    }
    
    // 检查并消耗资源
    auto cost = getTrainCost(type);
    if (!ResourceManager::getInstance()->consumeResources(cost.first, cost.second)) {
        ResourceManager::getInstance()->releasePopulation(pop);
        return false;
    }
    
    // 加入训练队列
    _trainingQueue.push_back(type);
    
    // 如果这是第一个，开始训练
    if (_trainingQueue.size() == 1) {
        _currentTrainTime = (float)getTrainTime(type);
        _trainingTimer = 0;
        _trainingProgress = 0;
    }
    
    return true;
}

bool Barracks::cancelTraining(int index) {
    if (index < 0 || index >= (int)_trainingQueue.size()) return false;
    
    UnitType type = _trainingQueue[index];
    
    // 返还资源
    auto cost = getTrainCost(type);
    float refundRate = (index == 0) ? (1.0f - _trainingProgress) * 0.5f : 0.5f;
    ResourceManager::getInstance()->addResource(ResourceType::GOLD, (int)(cost.first * refundRate));
    ResourceManager::getInstance()->addResource(ResourceType::ELIXIR, (int)(cost.second * refundRate));
    
    // 释放人口
    ResourceManager::getInstance()->releasePopulation(getUnitPopulation(type));
    
    // 从队列移除
    _trainingQueue.erase(_trainingQueue.begin() + index);
    
    // 如果取消的是当前正在训练的，重置进度
    if (index == 0 && !_trainingQueue.empty()) {
        _currentTrainTime = (float)getTrainTime(_trainingQueue[0]);
        _trainingTimer = 0;
        _trainingProgress = 0;
    }
    
    return true;
}

bool Barracks::deployUnit(UnitType type) {
    for (auto it = _trainedUnits.begin(); it != _trainedUnits.end(); ++it) {
        if (*it == type) {
            _trainedUnits.erase(it);
            return true;
        }
    }
    return false;
}

void Barracks::completeTraining() {
    if (_trainingQueue.empty()) return;
    
    UnitType completedType = _trainingQueue.front();
    _trainingQueue.erase(_trainingQueue.begin());
    
    // 添加到已训练列表
    _trainedUnits.push_back(completedType);
    
    // 如果队列还有，开始下一个训练
    if (!_trainingQueue.empty()) {
        _currentTrainTime = (float)getTrainTime(_trainingQueue[0]);
        _trainingTimer = 0;
        _trainingProgress = 0;
    } else {
        _trainingProgress = 0;
        _trainingTimer = 0;
    }
}

// ==================== 帧更新 ====================

void Barracks::update(float dt) {
    Building::update(dt);
    
    // 只有正常状态才训练
    if (_state != BuildingState::NORMAL) return;
    
    // 更新训练进度
    if (!_trainingQueue.empty()) {
        _trainingTimer += dt;
        _trainingProgress = _trainingTimer / _currentTrainTime;
        
        if (_trainingProgress >= 1.0f) {
            _trainingProgress = 1.0f;
            completeTraining();
        }
    }
}
