// ResourceManager.cpp
// 资源管理器实现文件
// 功能: 实现资源管理器的所有方法

#include "ResourceManager.h"

// 单例实例初始化
ResourceManager* ResourceManager::_instance = nullptr;

// ==================== 单例管理 ====================

ResourceManager* ResourceManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new ResourceManager();
    }
    return _instance;
}

void ResourceManager::destroyInstance() {
    if (_instance != nullptr) {
        delete _instance;
        _instance = nullptr;
    }
}

// ==================== 构造与析构 ====================

ResourceManager::ResourceManager() 
    : _currentPopulation(0)
    , _maxPopulation(20)
    , _totalWorkers(2)
    , _busyWorkers(0)
{
    initDefaultResources();
}

ResourceManager::~ResourceManager() {
    _callbacks.clear();
}

// ==================== 初始化 ====================

void ResourceManager::initDefaultResources() {
    // 初始化资源数量
    _resources[ResourceType::GOLD] = 500;
    _resources[ResourceType::ELIXIR] = 500;
    _resources[ResourceType::WORKER] = 2;
    _resources[ResourceType::POPULATION] = 0;
    
    // 初始化资源上限
    _maxResources[ResourceType::GOLD] = 5000;
    _maxResources[ResourceType::ELIXIR] = 5000;
    _maxResources[ResourceType::WORKER] = 5;
    _maxResources[ResourceType::POPULATION] = 20;
    
    // 初始化人口
    _currentPopulation = 0;
    _maxPopulation = 20;
    
    // 初始化工人
    _totalWorkers = 2;
    _busyWorkers = 0;
}

void ResourceManager::reset() {
    initDefaultResources();
    // 通知所有回调资源已重置
    for (auto& callback : _callbacks) {
        callback(ResourceType::GOLD, 0, _resources[ResourceType::GOLD]);
        callback(ResourceType::ELIXIR, 0, _resources[ResourceType::ELIXIR]);
    }
}

// ==================== 资源操作接口 ====================

int ResourceManager::getResource(ResourceType type) const {
    auto it = _resources.find(type);
    if (it != _resources.end()) {
        return it->second;
    }
    return 0;
}

int ResourceManager::getMaxResource(ResourceType type) const {
    auto it = _maxResources.find(type);
    if (it != _maxResources.end()) {
        return it->second;
    }
    return 0;
}

void ResourceManager::setResource(ResourceType type, int amount) {
    int oldValue = _resources[type];
    int maxValue = _maxResources[type];
    _resources[type] = std::min(amount, maxValue);
    _resources[type] = std::max(_resources[type], 0);
    
    if (oldValue != _resources[type]) {
        notifyResourceChange(type, oldValue, _resources[type]);
    }
}

void ResourceManager::setMaxResource(ResourceType type, int maxAmount) {
    _maxResources[type] = maxAmount;
    // 如果当前资源超过新上限，则调整
    if (_resources[type] > maxAmount) {
        setResource(type, maxAmount);
    }
}

int ResourceManager::addResource(ResourceType type, int amount) {
    if (amount <= 0) return 0;
    
    int oldValue = _resources[type];
    int maxValue = _maxResources[type];
    int newValue = std::min(oldValue + amount, maxValue);
    _resources[type] = newValue;
    
    int actualAdded = newValue - oldValue;
    if (actualAdded > 0) {
        notifyResourceChange(type, oldValue, newValue);
    }
    
    return actualAdded;
}

bool ResourceManager::consumeResource(ResourceType type, int amount) {
    if (amount <= 0) return true;
    if (!hasEnoughResource(type, amount)) return false;
    
    int oldValue = _resources[type];
    _resources[type] -= amount;
    notifyResourceChange(type, oldValue, _resources[type]);
    
    return true;
}

bool ResourceManager::hasEnoughResource(ResourceType type, int amount) const {
    return getResource(type) >= amount;
}

bool ResourceManager::hasEnoughResources(int gold, int elixir) const {
    return hasEnoughResource(ResourceType::GOLD, gold) &&
           hasEnoughResource(ResourceType::ELIXIR, elixir);
}

bool ResourceManager::consumeResources(int gold, int elixir) {
    if (!hasEnoughResources(gold, elixir)) return false;
    
    consumeResource(ResourceType::GOLD, gold);
    consumeResource(ResourceType::ELIXIR, elixir);
    return true;
}

// ==================== 人口管理 ====================

int ResourceManager::getCurrentPopulation() const {
    return _currentPopulation;
}

int ResourceManager::getMaxPopulation() const {
    return _maxPopulation;
}

void ResourceManager::addMaxPopulation(int amount) {
    _maxPopulation += amount;
}

bool ResourceManager::usePopulation(int amount) {
    if (_currentPopulation + amount > _maxPopulation) {
        return false;
    }
    _currentPopulation += amount;
    return true;
}

void ResourceManager::releasePopulation(int amount) {
    _currentPopulation = std::max(0, _currentPopulation - amount);
}

// ==================== 工人管理 ====================

int ResourceManager::getAvailableWorkers() const {
    return _totalWorkers - _busyWorkers;
}

int ResourceManager::getTotalWorkers() const {
    return _totalWorkers;
}

bool ResourceManager::useWorker() {
    if (_busyWorkers >= _totalWorkers) {
        return false;
    }
    _busyWorkers++;
    return true;
}

void ResourceManager::releaseWorker() {
    if (_busyWorkers > 0) {
        _busyWorkers--;
    }
}

void ResourceManager::addWorker() {
    if (_totalWorkers < _maxResources[ResourceType::WORKER]) {
        _totalWorkers++;
    }
}

// ==================== 回调管理 ====================

void ResourceManager::registerCallback(ResourceCallback callback) {
    _callbacks.push_back(callback);
}

void ResourceManager::clearCallbacks() {
    _callbacks.clear();
}

void ResourceManager::notifyResourceChange(ResourceType type, int oldValue, int newValue) {
    for (auto& callback : _callbacks) {
        callback(type, oldValue, newValue);
    }
}

// ==================== 数据操作 ====================

ResourceData ResourceManager::getAllResources() const {
    ResourceData data;
    data.gold = getResource(ResourceType::GOLD);
    data.elixir = getResource(ResourceType::ELIXIR);
    data.workers = getAvailableWorkers();
    data.population = _currentPopulation;
    data.maxPopulation = _maxPopulation;
    return data;
}
