#include "cocos2d.h"
#include <vector>

// 纯数据类，用来存储其他建筑的属性
struct OtherBuildingConfig {
    int id;                                  // 方便管理
    std::string name;                        // 如"ARCHER_TOWER"
    std::string spriteFrameName;             // 如"archer_tower.png" 
    TargetPriority aiType;                   // 如：弓箭塔 - 这个字段决定了它是弓箭塔
    // 战斗属性
    std::vector<float> HP;                   // 每个等级的生命值  
    std::vector<float> DP;                   // 每个等级的抗性（0~1）
    // 基地属性
    int length;                             // 建筑占地长度（格子数）
    int width;                              // 建筑占地宽度（格子数）
    std::vector<float> BUILD_TIME;           // 每个等级的建造时间（秒）
    std::vector<int> COST_GOLD;              // 每个等级的建造花费（金）
    std::vector<int> COST_ELIXIR;            // 每个等级的建造花费（圣水）
    int MAX_LEVEL;                           // 最大等级
};