# 第8章 UnitManager 兵种管理器

> **本章目标**：理解 UnitManager 的职责，掌握配置加载、训练队列管理、等级系统的实现。

---

## 8.1 配置加载与解析

### 8.1.1 JSON 结构设计

`Resources/res/units_config.json` 存储所有兵种配置：

```json
{
  "units": [
    {
      "id": 1001,
      "name": "Spearman",
      "HP": [50, 60, 75, 90, 110],
      "ATK": [8, 10, 13, 16, 20],
      "ATK_SPEED": [1.0, 0.95, 0.9, 0.85, 0.8],
      "ATK_RANGE": [30, 30, 30, 30, 30],
      "SPEED": [60, 65, 70, 75, 80],
      "COST_COIN": 20,
      "COST_DIAMOND": 0,
      "MAXLEVEL": 4,
      "spriteFrameName": "unit/spearman/spearman.png",
      "spriteBaseName": "unit/spearman/spearman",
      "anim_idle": "idle",
      "anim_idle_frames": 4,
      "anim_idle_delay": 0.15,
      "anim_walk": "walk",
      "anim_walk_frames": 6,
      "anim_walk_delay": 0.1,
      "anim_attack": "attack",
      "anim_attack_frames": 4,
      "anim_attack_delay": 0.08,
      "anim_death": "death",
      "anim_death_frames": 3,
      "anim_death_delay": 0.12,
      "isRanged": false,
      "isFlying": false,
      "aiType": "ANY"
    }
  ]
}
```

### 8.1.2 parseUnitConfig() 实现

```cpp
// Classes/Soldier/UnitManager.cpp
bool UnitManager::loadConfig(const std::string& path) {
    std::string jsonData = FileUtils::getInstance()->getStringFromFile(path);
    
    if (jsonData.empty()) {
        CCLOG("[UnitManager] 配置文件不存在: %s", path.c_str());
        return false;
    }
    
    // 处理 BOM
    if (jsonData.size() >= 3 &&
        static_cast<unsigned char>(jsonData[0]) == 0xEF) {
        jsonData = jsonData.substr(3);
    }
    
    rapidjson::Document doc;
    doc.Parse(jsonData.c_str());
    
    if (doc.HasParseError()) {
        CCLOG("[UnitManager] JSON 解析错误");
        return false;
    }
    
    if (!doc.HasMember("units") || !doc["units"].IsArray()) {
        CCLOG("[UnitManager] 缺少 units 数组");
        return false;
    }
    
    _configs.clear();
    
    for (auto& item : doc["units"].GetArray()) {
        UnitConfig config;
        parseUnitConfig(item, config);
        _configs[config.id] = config;
    }
    
    CCLOG("[UnitManager] 加载了 %zu 个兵种配置", _configs.size());
    return true;
}

void UnitManager::parseUnitConfig(const rapidjson::Value& item, 
                                   UnitConfig& config) {
    config.id = item["id"].GetInt();
    config.name = item["name"].GetString();
    config.MAXLEVEL = item["MAXLEVEL"].GetInt();
    config.COST_COIN = item["COST_COIN"].GetInt();
    config.COST_DIAMOND = item["COST_DIAMOND"].GetInt();
    
    // 解析数组属性
    parseFloatArray(item, "HP", config.HP);
    parseFloatArray(item, "ATK", config.ATK);
    parseFloatArray(item, "ATK_SPEED", config.ATK_SPEED);
    parseFloatArray(item, "ATK_RANGE", config.ATK_RANGE);
    parseFloatArray(item, "SPEED", config.SPEED);
    
    // 动画配置
    config.spriteFrameName = readString(item, "spriteFrameName", "");
    config.spriteBaseName = readString(item, "spriteBaseName", "");
    config.anim_idle = readString(item, "anim_idle", "idle");
    config.anim_idle_frames = readInt(item, "anim_idle_frames", 1);
    config.anim_idle_delay = readFloat(item, "anim_idle_delay", 0.1f);
    // ... 其他动画字段 ...
    
    // 类型标志
    config.isRanged = readBool(item, "isRanged", false);
    config.isFlying = readBool(item, "isFlying", false);
    
    // AI 类型
    std::string aiTypeStr = readString(item, "aiType", "ANY");
    config.aiType = parseAIType(aiTypeStr);
}
```

### 8.1.3 数组属性解析

```cpp
void UnitManager::parseFloatArray(const rapidjson::Value& item,
                                   const char* key,
                                   std::vector<float>& out) {
    out.clear();
    
    if (!item.HasMember(key) || !item[key].IsArray()) {
        return;
    }
    
    for (auto& v : item[key].GetArray()) {
        if (v.IsNumber()) {
            out.push_back(static_cast<float>(v.GetDouble()));
        }
    }
}
```

### 8.1.4 AI 类型映射

```cpp
AITargetType UnitManager::parseAIType(const std::string& str) {
    if (str == "DEFENCE" || str == "defence") {
        return AITargetType::DEFENCE;
    }
    if (str == "RESOURCE" || str == "resource") {
        return AITargetType::RESOURCE;
    }
    if (str == "BASE" || str == "base") {
        return AITargetType::BASE;
    }
    return AITargetType::ANY;
}
```

---

## 8.2 兵种训练队列管理

### 8.2.1 std::map<int, int> 存储结构

```cpp
// Classes/Soldier/UnitManager.h
class UnitManager {
private:
    // 训练队列：unitId → 已训练数量
    std::map<int, int> _trainedUnits;
    
    // 兵种等级：unitId → 当前等级
    std::map<int, int> _unitLevels;
    
public:
    // 训练接口
    bool trainUnit(int unitId);
    bool consumeUnit(int unitId);
    int getTrainedCount(int unitId) const;
    
    // 等级接口
    int getUnitLevel(int unitId) const;
    bool upgradeUnit(int unitId);
};
```

### 8.2.2 训练/消费 API

```cpp
// Classes/Soldier/UnitManager.cpp
bool UnitManager::trainUnit(int unitId) {
    // 1. 检查配置是否存在
    const UnitConfig* config = getConfig(unitId);
    if (!config) {
        CCLOG("[UnitManager] 未知兵种: %d", unitId);
        return false;
    }
    
    // 2. 计算训练费用（应用等级折扣）
    float costMultiplier = Core::getInstance()->getTrainingCostMultiplier();
    int coinCost = static_cast<int>(config->COST_COIN * costMultiplier);
    int diamondCost = static_cast<int>(config->COST_DIAMOND * costMultiplier);
    
    // 3. 检查并扣除资源
    if (coinCost > 0 && 
        !Core::getInstance()->consumeResource(ResourceType::COIN, coinCost)) {
        CCLOG("[UnitManager] 金币不足");
        return false;
    }
    
    if (diamondCost > 0 && 
        !Core::getInstance()->consumeResource(ResourceType::DIAMOND, diamondCost)) {
        // 退还已扣除的金币
        Core::getInstance()->addResource(ResourceType::COIN, coinCost);
        CCLOG("[UnitManager] 钻石不足");
        return false;
    }
    
    // 4. 增加训练数量
    _trainedUnits[unitId]++;
    
    CCLOG("[UnitManager] 训练完成: %s, 当前数量: %d",
          config->name.c_str(), _trainedUnits[unitId]);
    
    return true;
}

bool UnitManager::consumeUnit(int unitId) {
    auto it = _trainedUnits.find(unitId);
    if (it == _trainedUnits.end() || it->second <= 0) {
        return false;
    }
    
    it->second--;
    return true;
}

int UnitManager::getTrainedCount(int unitId) const {
    auto it = _trainedUnits.find(unitId);
    return (it != _trainedUnits.end()) ? it->second : 0;
}
```

### 8.2.3 跨场景状态持久化

训练数据在场景切换时保持：

```cpp
// UnitManager 是单例，数据自动保持
// 但需要在存档时序列化

std::map<int, int> UnitManager::getTrainedUnitsData() const {
    return _trainedUnits;
}

void UnitManager::setTrainedUnitsData(const std::map<int, int>& data) {
    _trainedUnits = data;
}
```

---

## 8.3 兵种等级系统

### 8.3.1 等级数据存储

```cpp
int UnitManager::getUnitLevel(int unitId) const {
    auto it = _unitLevels.find(unitId);
    return (it != _unitLevels.end()) ? it->second : 0;
}

void UnitManager::setUnitLevel(int unitId, int level) {
    const UnitConfig* config = getConfig(unitId);
    if (!config) return;
    
    // 限制在有效范围内
    level = std::max(0, std::min(level, config->MAXLEVEL));
    _unitLevels[unitId] = level;
}
```

### 8.3.2 等级上限限制

```cpp
bool UnitManager::upgradeUnit(int unitId) {
    const UnitConfig* config = getConfig(unitId);
    if (!config) {
        CCLOG("[UnitManager] 未知兵种: %d", unitId);
        return false;
    }
    
    int currentLevel = getUnitLevel(unitId);
    
    // 检查是否已达最高等级
    if (currentLevel >= config->MAXLEVEL) {
        CCLOG("[UnitManager] 已达最高等级");
        return false;
    }
    
    // 计算升级费用（钻石）
    int upgradeCost = calculateUpgradeCost(unitId, currentLevel);
    
    // 扣除费用
    if (!Core::getInstance()->consumeResource(
            ResourceType::DIAMOND, upgradeCost)) {
        CCLOG("[UnitManager] 钻石不足");
        return false;
    }
    
    // 升级
    _unitLevels[unitId] = currentLevel + 1;
    
    CCLOG("[UnitManager] 升级成功: %s Lv.%d → Lv.%d",
          config->name.c_str(), currentLevel + 1, currentLevel + 2);
    
    return true;
}

int UnitManager::calculateUpgradeCost(int unitId, int currentLevel) const {
    // 升级费用：基础 50 钻石 × 等级倍率
    int baseCost = 50;
    return baseCost * (currentLevel + 1);
}
```

### 8.3.3 存档恢复逻辑

```cpp
void UnitManager::restoreState(const std::map<int, int>& trained,
                                const std::map<int, int>& levels) {
    _trainedUnits = trained;
    _unitLevels = levels;
    
    CCLOG("[UnitManager] 状态已恢复: %zu 种兵种, %zu 个等级记录",
          _trainedUnits.size(), _unitLevels.size());
}

void UnitManager::resetState() {
    _trainedUnits.clear();
    _unitLevels.clear();
}
```

---

## 8.4 士兵生成工厂

### 8.4.1 spawnSoldier() 源码

```cpp
Soldier* UnitManager::spawnSoldier(int unitId, const Vec2& position, 
                                    int level) {
    // 1. 获取配置
    const UnitConfig* config = getConfig(unitId);
    if (!config) {
        CCLOG("[UnitManager] 无法生成士兵：未知 ID %d", unitId);
        return nullptr;
    }
    
    // 2. 确定等级
    int resolvedLevel = level;
    if (resolvedLevel < 0) {
        resolvedLevel = getUnitLevel(unitId);
    }
    resolvedLevel = std::min(resolvedLevel, config->MAXLEVEL);
    
    // 3. 创建士兵
    Soldier* soldier = Soldier::create(config, resolvedLevel);
    if (!soldier) {
        CCLOG("[UnitManager] Soldier::create 失败");
        return nullptr;
    }
    
    // 4. 设置位置
    soldier->setPosition(position);
    
    CCLOG("[UnitManager] 生成士兵: %s Lv.%d at (%.0f, %.0f)",
          config->name.c_str(), resolvedLevel + 1,
          position.x, position.y);
    
    return soldier;
}
```

### 8.4.2 位置设置与返回

注意：`spawnSoldier` 只创建士兵，不添加到场景：

```cpp
// 调用方负责添加到场景
Soldier* soldier = UnitManager::getInstance()->spawnSoldier(
    1001, Vec2(100, 200), -1);

if (soldier) {
    _soldierLayer->addChild(soldier);
    _soldiers.push_back(soldier);
    soldier->retain();  // 手动保持引用
}
```

---

## 数据流图

```
┌──────────────────────────────────────────────────────────────┐
│                      UnitManager                              │
├──────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐       │
│  │ _configs    │    │_trainedUnits│    │ _unitLevels │       │
│  │ 配置模板    │    │ 训练队列    │    │ 等级数据    │       │
│  └──────┬──────┘    └──────┬──────┘    └──────┬──────┘       │
│         │                  │                  │               │
│         └──────────────────┼──────────────────┘               │
│                            │                                  │
│                            ▼                                  │
│                    ┌───────────────┐                         │
│                    │ spawnSoldier()│                         │
│                    └───────┬───────┘                         │
│                            │                                  │
└────────────────────────────┼──────────────────────────────────┘
                             │
                             ▼
                      ┌─────────────┐
                      │   Soldier   │
                      │   实例      │
                      └─────────────┘
```

---

## 获取所有兵种 ID

```cpp
std::vector<int> UnitManager::getAllUnitIds() const {
    std::vector<int> ids;
    ids.reserve(_configs.size());
    
    for (const auto& pair : _configs) {
        ids.push_back(pair.first);
    }
    
    // 按 ID 排序
    std::sort(ids.begin(), ids.end());
    
    return ids;
}
```

---

## 本章小结

1. **UnitManager** 是兵种系统的核心管理器
2. **JSON 配置** 定义所有兵种属性，支持热更新
3. **训练队列** 使用 `std::map<int, int>` 存储
4. **等级系统** 独立于训练数量，支持升级
5. **spawnSoldier()** 是士兵创建的唯一入口

---

## 练习题

1. 添加训练时间机制：训练需要等待一定时间
2. 实现批量训练功能：一次训练多个相同单位
3. 添加兵种解锁机制：某些兵种需要达到条件才能训练

---

**下一章**：[第9章 子弹系统实现](第09章-子弹系统实现.md)
