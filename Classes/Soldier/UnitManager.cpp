// UnitManager.cpp (实现部分)
void UnitManager::loadConfig(const std::string& jsonFile) {
    // 假设解析 JSON
    // ... 解析过程 ...

    // 模拟存入数据：这里定义了哥布林
    UnitConfig goblinData;
    goblinData.id = 101;
    goblinData.name = "Goblin";
    goblinData.moveSpeed = 150.0f; // 跑得快
    goblinData.aiType = TargetPriority::RESOURCE; // 爱抢钱
    goblinData.spriteFrameName = "goblin.png";
    _configCache[101] = goblinData;

    // 模拟存入数据：这里定义了野蛮人
    UnitConfig barbData;
    barbData.id = 102;
    barbData.name = "Barbarian";
    barbData.moveSpeed = 100.0f;
    barbData.aiType = TargetPriority::ANY; // 见谁打谁
    barbData.spriteFrameName = "barbarian.png";
    _configCache[102] = barbData;
}

Soldier* UnitManager::spawnSoldier(int unitId, cocos2d::Vec2 position) {
    // 1. 查找配置
    if (_configCache.find(unitId) == _configCache.end()) return nullptr;

    const UnitConfig* cfg = &_configCache[unitId];

    // 2. 使用通用类 + 特定配置创建对象
    auto soldier = Soldier::create(cfg);
    soldier->setPosition(position);

    return soldier;
}