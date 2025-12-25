#ifndef __PRODUCTION_BUILDING_DATA_H__
#define __PRODUCTION_BUILDING_DATA_H__

#include "cocos2d.h"
#include "Soldier/UnitData.h"
#include <vector>

struct ProductionBuildingConfig {
    int id;
    std::string name;
    std::string spriteFrameName;
    TargetPriority aiType;
    
    std::vector<float> HP;
    std::vector<float> DP;
    
    int length;
    int width;
    std::vector<float> BUILD_TIME;
    std::vector<int> COST_GOLD;
    std::vector<int> COST_ELIXIR;
    std::vector<int> PRODUCE_ELIXIR;
    std::vector<int> STORAGE_ELIXIR_CAPACITY;
    std::vector<int> PRODUCE_GOLD;
    std::vector<int> STORAGE_GOLD_CAPACITY;
    int MAXLEVEL;
    
    std::string anim_idle = "idle";
    int anim_idle_frames = 1;
    float anim_idle_delay = 0.1f;
    std::string anim_produce = "produce";
    int anim_produce_frames = 1;
    float anim_produce_delay = 0.1f;
};

#endif // __PRODUCTION_BUILDING_DATA_H__
