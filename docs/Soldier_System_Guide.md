# 士兵类系统说明文档

## 1. 系统架构概览

```
┌─────────────────┐     ┌──────────────────┐     ┌────────────────┐
│ units_config.json│────▶│   UnitManager    │────▶│    Soldier     │
│   (配置数据)     │     │  (配置加载/管理)  │     │  (运行时实例)   │
└─────────────────┘     └──────────────────┘     └────────────────┘
                               │
                               ▼
                        ┌──────────────┐
                        │  UnitConfig  │
                        │  (数据结构)   │
                        └──────────────┘
```

## 2. 核心文件说明

| 文件 | 职责 |
|------|------|
| `UnitData.h` | 定义 `UnitConfig` 结构体和 `TargetPriority` 枚举 |
| `UnitManager.h/cpp` | 单例管理器，负责加载JSON、缓存配置、生成士兵实例 |
| `Soldier.h/cpp` | 士兵运行时类，继承自 `cocos2d::Node` |
| `units_config.json` | 所有士兵的配置数据文件 |

## 3. 创建新士兵的完整流程

### 步骤1: 在JSON中添加配置

打开 `Resources/res/units_config.json`，在 `units` 数组中添加新单位：

```json
{
    "id": 103,                    // 唯一ID
    "name": "Archer",             // 名称
    "spriteFrameName": "archer.png",  // 精灵图片
    "HP": [40, 45, 50, 55, 60],   // 每级血量
    "SPEED": [80, 80, 80, 80, 80], // 每级移速
    "DP": [0, 0, 0, 0, 0],        // 每级防御
    "ATK": [30, 35, 40, 48, 56],  // 每级攻击
    "RANGE": [300, 300, 320, 340, 360], // 每级攻击范围
    "aiType": 0,                  // AI类型: 0=ANY, 1=RESOURCE, 2=DEFENSE
    "ISREMOTE": true,             // 是否远程
    "ISFLY": false,               // 是否飞行
    "MAXLEVEL": 4,                // 最大等级(从0开始)
    "COST_COIN": 0,               // 金币消耗
    "COST_ELIXIR": 150,           // 圣水消耗
    "COST_POPULATION": 1,         // 人口消耗
    "TRAIN_TIME": 25              // 训练时间(秒)
}
```

### 步骤2: 游戏启动时加载配置

```cpp
// 在游戏初始化时调用(如 AppDelegate 或 GameScene::init)
UnitManager::getInstance()->loadConfig("res/units_config.json");
```

### 步骤3: 生成士兵实例

```cpp
// 参数: 单位ID, 位置, 等级(可选,默认0)
auto goblin = UnitManager::getInstance()->spawnSoldier(101, Vec2(100, 200), 2);
if (goblin) {
    this->addChild(goblin);
}

auto barbarian = UnitManager::getInstance()->spawnSoldier(102, Vec2(150, 200));
this->addChild(barbarian);
```

## 4. UnitConfig 字段说明

```cpp
struct UnitConfig {
    // === 基础信息 ===
    int id;                      // 唯一标识符
    std::string name;            // 显示名称
    std::string spriteFrameName; // 精灵帧名称

    // === 等级数据(数组,索引=等级) ===
    std::vector<float> HP;       // 血量
    std::vector<float> SPEED;    // 移动速度
    std::vector<float> DP;       // 防御力
    std::vector<float> ATK;      // 攻击力
    std::vector<float> RANGE;    // 攻击范围

    // === AI行为 ===
    TargetPriority aiType;       // 攻击优先级

    // === 特性标记 ===
    bool ISREMOTE;               // 远程单位
    bool ISFLY;                  // 飞行单位

    // === 资源相关 ===
    int MAXLEVEL;                // 最大等级
    int COST_COIN;               // 金币消耗
    int COST_ELIXIR;             // 圣水消耗
    int COST_POPULATION;         // 人口消耗
    int TRAIN_TIME;              // 训练时间(秒)
};
```

## 5. AI类型说明 (TargetPriority)

| 值 | 枚举 | 说明 | 典型单位 |
|----|------|------|----------|
| 0 | ANY | 攻击最近的任意建筑 | 野蛮人 |
| 1 | RESOURCE | 优先攻击资源建筑 | 哥布林 |
| 2 | DEFENSE | 优先攻击防御建筑 | 气球兵 |

## 6. 设计模式说明

### 6.1 享元模式 (Flyweight)
- `UnitConfig` 作为共享的内在状态存储在 `UnitManager::_configCache`
- `Soldier` 实例只持有指向配置的指针，不复制数据
- 节省内存：100个哥布林共享同一份配置数据

### 6.2 工厂模式 (Factory)
- `UnitManager::spawnSoldier()` 作为工厂方法
- 隐藏创建细节，统一生成逻辑

### 6.3 单例模式 (Singleton)
- `UnitManager::getInstance()` 全局唯一访问点
- 确保配置只加载一次

## 7. 运行时数据 vs 配置数据

```
配置数据 (UnitConfig)          运行时数据 (Soldier)
─────────────────────          ────────────────────
HP[level] = 最大血量     ──▶   _currentHP = 当前血量
SPEED[level] = 移速            _level = 当前等级
ATK[level] = 攻击力            _target = 当前目标
...                            _bodySprite = 显示精灵
```

## 8. 扩展建议

### 添加新的单位属性
1. 在 `UnitData.h` 的 `UnitConfig` 中添加字段
2. 在 `UnitManager::parseUnitConfig()` 中添加解析逻辑
3. 在 JSON 配置中添加对应字段

### 添加新的AI类型
1. 在 `UnitData.h` 的 `TargetPriority` 枚举中添加新值
2. 在 `UnitManager::parseAIType()` 中添加映射
3. 在 `Soldier::findTarget()` 中实现对应逻辑
