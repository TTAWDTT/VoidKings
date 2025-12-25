#ifndef __STORAGE_BUILDING_DATA_H__
#define __STORAGE_BUILDING_DATA_H__

#include "cocos2d.h"
#include "Soldier/UnitData.h"
#include <vector>

struct StorageBuildingConfig {
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
    std::vector<int> ADD_STORAGE_ELIXIR_CAPACITY;
    std::vector<int> ADD_STORAGE_GOLD_CAPACITY;
    int MAXLEVEL;
    
    std::string anim_idle = "idle";
    int anim_idle_frames = 1;
    float anim_idle_delay = 0.1f;
};

#endif // __STORAGE_BUILDING_DATA_H__
