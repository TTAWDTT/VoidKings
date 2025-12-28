# 附录B：配置文件规范

> **本附录**：详细说明 VoidKings 中所有配置文件的字段定义。

---

## buildings_config.json 完整字段说明

### 防御建筑 (defenceBuildings)

```json
{
  "id": 1001,                    // 唯一标识符
  "name": "ArrowTower",          // 建筑名称（内部使用）
  "displayName": "箭塔",          // 显示名称（可选）
  
  // 属性数组（索引对应等级 0,1,2...）
  "HP": [100, 120, 150, 180, 220],
  "ATK": [15, 20, 25, 32, 40],
  "ATK_SPEED": [1.0, 0.95, 0.9, 0.85, 0.8],  // 攻击间隔（秒）
  "ATK_RANGE": [150, 160, 170, 180, 200],    // 攻击范围（像素）
  
  // 尺寸
  "width": 3,                    // 占地宽度（格子）
  "length": 3,                   // 占地长度（格子）
  
  // 等级
  "MAXLEVEL": 4,                 // 最大等级索引（实际等级 = 索引+1）
  
  // 攻击目标
  "GROUND_ABLE": true,           // 可攻击地面单位
  "SKY_ABLE": true,              // 可攻击空中单位
  
  // 贴图
  "spritePath": "buildings/ArrowTower/arrow_tower.png",
  "bulletSprite": "buildings/ArrowTower/arrow.png",
  "bulletSpeed": 300,            // 子弹飞行速度
  
  // 动画（可选）
  "anim_idle": "idle",
  "anim_idle_frames": 4,
  "anim_idle_delay": 0.15,
  "anim_attack": "attack",
  "anim_attack_frames": 3,
  "anim_attack_delay": 0.1,
  
  // 特殊属性（可选）
  "aoeRadius": 0,                // AOE 半径（0 表示单体）
  "slowEffect": false,           // 减速效果
  "slowDuration": 0,
  "slowPercent": 0
}
```

### 生产建筑 (productionBuildings)

```json
{
  "id": 3001,
  "name": "MainBase",
  "displayName": "主基地",
  
  "HP": [500, 600, 750, 900],
  "width": 4,
  "length": 4,
  "MAXLEVEL": 3,
  
  // 生产属性
  "PRODUCE_GOLD": [0, 0, 0, 0],     // 每次生产金币数
  "PRODUCE_ELIXIR": [0, 0, 0, 0],   // 每次生产圣水数
  "PRODUCE_INTERVAL": 10.0,         // 生产间隔（秒）
  "STORAGE_CAPACITY": [0, 0, 0, 0], // 存储容量
  
  // 特殊标记
  "isMainBase": true,
  "isBarracks": false,
  
  "spritePath": "buildings/MainBase/main_base.png"
}
```

### 存储建筑 (storageBuildings)

```json
{
  "id": 4001,
  "name": "GoldStorage",
  "displayName": "金库",
  
  "HP": [200, 250, 300],
  "width": 3,
  "length": 3,
  "MAXLEVEL": 2,
  
  "STORAGE_GOLD": [500, 1000, 2000],
  "STORAGE_ELIXIR": [0, 0, 0],
  
  "spritePath": "buildings/GoldStorage/gold_storage.png"
}
```

---

## units_config.json 完整字段说明

```json
{
  "id": 1001,                      // 唯一标识符
  "name": "Spearman",              // 内部名称
  "displayName": "枪兵",            // 显示名称
  
  // 属性数组
  "HP": [50, 60, 75, 90, 110],
  "ATK": [8, 10, 13, 16, 20],
  "ATK_SPEED": [1.0, 0.95, 0.9, 0.85, 0.8],  // 攻击间隔
  "ATK_RANGE": [30, 30, 30, 30, 30],          // 攻击范围
  "SPEED": [60, 65, 70, 75, 80],              // 移动速度
  
  // 费用
  "COST_COIN": 20,
  "COST_DIAMOND": 0,
  "UPGRADE_COST": [50, 100, 200, 400],        // 升级费用（钻石）
  
  // 等级
  "MAXLEVEL": 4,
  
  // 贴图
  "spriteFrameName": "unit/spearman/spearman.png",
  "spriteBaseName": "unit/spearman/spearman",
  
  // 动画配置
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
  
  // 类型标记
  "isRanged": false,               // 是否远程
  "isFlying": false,               // 是否飞行
  
  // AI 类型
  "aiType": "ANY",                 // ANY / DEFENCE / RESOURCE / BASE
  
  // 特殊技能（可选）
  "healAmount": [0, 0, 0, 0, 0],   // 治疗量（治疗兵）
  "splashRadius": 0,               // 溅射半径
  "stunDuration": 0                // 眩晕时长
}
```

---

## 存档 JSON Schema

```json
{
  "version": 1,
  
  "meta": {
    "timestamp": 1703750400,       // Unix 时间戳
    "summary": "Lv.3 | 5000G | 12 Buildings",
    "playTime": 3600               // 游戏时长（秒）
  },
  
  "core": {
    "coin": 5000,
    "diamond": 150,
    "baseLevel": 3,
    "barracksLevel": 2,
    
    "levelStars": [
      { "id": 1, "stars": 3 },
      { "id": 2, "stars": 2 }
    ],
    
    "statistics": {
      "totalKills": 500,
      "totalDeaths": 100,
      "battlesWon": 15,
      "battlesLost": 5
    }
  },
  
  "units": {
    "trained": [
      { "id": 1001, "count": 5 },
      { "id": 1002, "count": 3 }
    ],
    
    "levels": [
      { "id": 1001, "level": 2 },
      { "id": 1002, "level": 1 }
    ]
  },
  
  "base": {
    "buildings": [
      {
        "configId": 1001,
        "gridX": 10,
        "gridY": 12,
        "level": 2,
        "isDefence": true
      }
    ]
  }
}
```

---

## 回放 JSON Schema

```json
{
  "version": 1,
  "levelId": 5,
  "defenseMode": false,
  "battleSpeed": 1.0,
  "timestamp": 1703750400,
  "allowDefaultUnits": true,
  
  "resultWin": true,
  "resultStars": 3,
  
  "deployableUnits": [
    { "id": 1001, "count": 10 },
    { "id": 1002, "count": 5 }
  ],
  
  "events": [
    { "t": 0.5, "u": 1001, "x": 5, "y": 10, "l": 2 },
    { "t": 1.2, "u": 1001, "x": 6, "y": 10, "l": 2 },
    { "t": 2.0, "u": 1002, "x": 5, "y": 12, "l": 1 }
  ]
}
```

---

## 基地快照 JSON Schema

```json
{
  "version": 1,
  "configHash": "12345678901234567890",
  "baseLevel": 3,
  "barracksLevel": 2,
  
  "baseAnchor": { "x": 26, "y": 14 },
  "barracksAnchor": { "x": 20, "y": 8 },
  
  "buildings": [
    {
      "configId": 1001,
      "gridX": 10,
      "gridY": 12,
      "level": 2,
      "isDefence": true
    }
  ]
}
```

---

## 配置验证

```cpp
bool validateUnitConfig(const UnitConfig& config) {
    if (config.id <= 0) return false;
    if (config.name.empty()) return false;
    if (config.HP.empty()) return false;
    if (config.MAXLEVEL < 0) return false;
    if (config.MAXLEVEL >= static_cast<int>(config.HP.size())) return false;
    return true;
}

bool validateDefenceConfig(const DefenceConfig& config) {
    if (config.id <= 0) return false;
    if (config.name.empty()) return false;
    if (config.HP.empty() || config.ATK.empty()) return false;
    if (config.width <= 0 || config.length <= 0) return false;
    return true;
}
```
