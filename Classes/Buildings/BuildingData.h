#include "cocos2d.h"
#include <vector>

// 定义自身类型（以便兵种通过索敌偏好筛选）
enum class BuildingType {
    ANY = 0,        // 其他建筑 (基地，兵营，药水生产厂)
    RESOURCE = 1,   // 资源建筑 (金矿，Building圣水收集器，金库，圣水罐)
    DEFENSE = 2     // 防御塔 (弓箭塔，迫击炮，城墙，烟花发射器)
    CANNOTSEE = 3   // 不可见目标（地雷）
};

// 纯数据类，用来存储建筑的属性
struct BuildingConfig {
    int id;                                  // 方便管理
    std::string name;                        // 如"ARCHER_TOWER"
    std::string spriteFrameName;             // 如"archer_tower.png" 
    TargetPriority aiType;                   // 如：弓箭塔 - 这个字段决定了它是弓箭塔
    // 战斗属性
    std::vector<float> HP;                   // 每个等级的生命值  
    std::vector<float> DP;                   // 每个等级的抗性（0~1）
    std::vector<float> ATK;                  // 每个等级的攻击力（每次伤害）（0表示不攻击）
    std::vector<float> ATK_RANGE;            // 每个等级的攻击范围（0表示不攻击）
    std::vector<float> ATK_SPEED;            // 每个等级的攻击速度（每秒攻击次数）（0表示不攻击）
    bool SKY_ABLE;              // 是否能攻击空中单位
    // 建造属性
    std::vector<float> BUILD_TIME;           // 每个等级的建造时间（秒）
    std::vector<int> COST_GOLD;              // 每个等级的建造花费（金）
    std::vector<int> COST_ELIXIR;            // 每个等级的建造花费（圣水）
    int MAX_LEVEL;                           // 最大等级
};