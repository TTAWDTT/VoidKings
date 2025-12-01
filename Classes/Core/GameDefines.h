// GameDefines.h
// 游戏全局定义和常量
// 功能: 定义游戏中使用的所有枚举类型、常量和全局配置
// 编码: UTF-8 with BOM (为VS兼容性)

#ifndef __GAME_DEFINES_H__
#define __GAME_DEFINES_H__

#include "cocos2d.h"
#include <string>
#include <map>
#include <vector>

USING_NS_CC;

// ==================== 游戏常量定义 ====================

// 设计分辨率
const float DESIGN_WIDTH = 1280.0f;
const float DESIGN_HEIGHT = 720.0f;

// 地图格子大小
const int GRID_SIZE = 32;
const int MAP_WIDTH = 40;  // 地图宽度(格子数)
const int MAP_HEIGHT = 30; // 地图高度(格子数)

// 游戏更新间隔
const float GAME_UPDATE_INTERVAL = 1.0f / 60.0f;

// Z-Order层级定义
enum ZOrder {
    Z_BACKGROUND = 0,      // 背景层
    Z_GROUND = 10,         // 地面层
    Z_BUILDING = 20,       // 建筑层
    Z_UNIT = 30,           // 单位层
    Z_PROJECTILE = 40,     // 投射物层
    Z_EFFECT = 50,         // 特效层
    Z_UI = 100,            // UI层
    Z_POPUP = 200          // 弹窗层
};

// ==================== 资源类型枚举 ====================
enum class ResourceType {
    GOLD = 0,      // 金币 - 用于建筑和兵种训练
    ELIXIR,        // 圣水 - 用于药水和高级兵种
    WORKER,        // 工人 - 用于建筑建造和升级
    POPULATION     // 人口 - 兵种和药水占用
};

// ==================== 建筑类型枚举 ====================
enum class BuildingType {
    // 核心建筑
    TOWN_HALL = 0,     // 大本营

    // 防御建筑
    CANNON,            // 加农炮
    ARCHER_TOWER,      // 箭塔
    MORTAR,            // 迫击炮

    // 资源建筑
    GOLD_MINE,         // 金矿
    ELIXIR_COLLECTOR,  // 圣水收集器

    // 存储建筑
    GOLD_STORAGE,      // 金币存储
    ELIXIR_STORAGE,    // 圣水存储

    // 军事建筑
    BARRACKS,          // 兵营
    SPELL_FACTORY,     // 法术工厂

    // 防护建筑
    WALL,              // 城墙

    COUNT              // 建筑类型总数
};

// ==================== 兵种类型枚举 ====================
enum class UnitType {
    BARBARIAN = 0,     // 野蛮人 - 近战单位
    ARCHER,            // 弓箭手 - 远程单位
    GIANT,             // 巨人 - 肉盾单位
    GOBLIN,            // 哥布林 - 资源掠夺者
    WIZARD,            // 法师 - 范围攻击
    HEALER,            // 治疗者 - 空中支援

    COUNT              // 兵种类型总数
};

// ==================== 药水/法术类型枚举 ====================
enum class PotionType {
    HEAL = 0,          // 治疗药水
    RAGE,              // 狂暴药水
    LIGHTNING,         // 闪电法术
    FREEZE,            // 冰冻法术

    COUNT              // 药水类型总数
};

// ==================== 弹道类型枚举 ====================
enum class ProjectileType {
    STRAIGHT = 0,      // 直线弹道
    PARABOLIC,         // 抛物线弹道
    TRACKING           // 追踪弹道
};

// ==================== 伤害类型枚举 ====================
enum class DamageType {
    SINGLE = 0,        // 单体伤害
    AREA               // 范围伤害
};

// ==================== 目标优先级枚举 ====================
enum class TargetPriority {
    NEAREST = 0,       // 最近目标
    LOWEST_HP,         // 最低血量
    DEFENSE_FIRST,     // 优先防御建筑
    RESOURCE_FIRST     // 优先资源建筑
};

// ==================== 阵营枚举 ====================
enum class Faction {
    PLAYER = 0,        // 玩家方
    ENEMY              // 敌方
};

// ==================== 游戏状态枚举 ====================
enum class GameState {
    MENU = 0,          // 主菜单
    BASE_BUILDING,     // 基地建设
    BATTLE,            // 战斗中
    PAUSED             // 暂停
};

// ==================== 单位状态枚举 ====================
enum class UnitState {
    IDLE = 0,          // 待机
    MOVING,            // 移动中
    ATTACKING,         // 攻击中
    DEAD               // 死亡
};

// ==================== 建筑状态枚举 ====================
enum class BuildingState {
    NORMAL = 0,        // 正常
    BUILDING,          // 建造中
    UPGRADING,         // 升级中
    DESTROYED          // 被摧毁
};

// ==================== 配置数据结构 ====================

// 资源数据结构
struct ResourceData {
    int gold = 0;
    int elixir = 0;
    int workers = 0;
    int population = 0;
    int maxPopulation = 0;
};

// 建筑配置数据
struct BuildingConfig {
    BuildingType type;
    std::string name;
    std::string texturePath;
    int gridWidth;
    int gridHeight;
    int maxLevel;
    std::vector<int> hpByLevel;
    std::vector<int> goldCostByLevel;
    std::vector<int> elixirCostByLevel;
    std::vector<int> buildTimeByLevel;
};

// 兵种配置数据
struct UnitConfig {
    UnitType type;
    std::string name;
    std::string texturePath;
    int hp;
    int damage;
    float attackRange;
    float attackSpeed;
    float moveSpeed;
    int goldCost;
    int elixirCost;
    int population;
    int trainTime;
    TargetPriority priority;
};

// 药水配置数据
struct PotionConfig {
    PotionType type;
    std::string name;
    std::string texturePath;
    float radius;
    float duration;
    float effectValue;
    int goldCost;
    int elixirCost;
    int population;
    int craftTime;
};

#endif // __GAME_DEFINES_H__
