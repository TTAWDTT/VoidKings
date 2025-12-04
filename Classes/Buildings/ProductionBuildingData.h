#include "cocos2d.h"
#include <vector>

// 纯数据类，用来存储生产建筑的属性
struct ProductionBuildingConfig {
    int id;                                  // 方便管理
    std::string name;                        // 如"ARCHER_TOWER"
    std::string spriteFrameName;             // 如"archer_tower.png" 
    TargetPriority aiType;                   // 如：弓箭塔 - 这个字段决定了它是弓箭塔
    // 基地属性
    int length;                             // 建筑占地长度（格子数）
    int width;                              // 建筑占地宽度（格子数）
    std::vector<float> BUILD_TIME;           // 每个等级的建造时间（秒）
    std::vector<int> COST_GOLD;              // 每个等级的建造花费（金）
    std::vector<int> COST_ELIXIR;            // 每个等级的建造花费（圣水）
    std::vector<int> PRODUCE_ELIXIR;         // 每个等级的圣水产量（每分钟产量，0表示不产圣水）
    std::vector<int> STORAGE_ELIXIR_CAPACITY;// 每个等级的圣水生产储存上限（0表示不储存圣水）
    std::vector<int> PRODUCE_GOLD;           // 每个等级的金钱产量（每分钟产量，0表示不产金钱）
    std::vector<int> STORAGE_GOLD_CAPACITY;  // 每个等级的金钱生产储存上限（0表示不储存金钱）
    int MAX_LEVEL;                           // 最大等级
};