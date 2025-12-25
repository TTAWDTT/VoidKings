#ifndef __DEFENSE_BUILDING_DATA_H__
#define __DEFENSE_BUILDING_DATA_H__

#include "cocos2d.h"
#include "Soldier/UnitData.h"
#include <vector>

struct DefenceBuildingConfig {
    int id;
    std::string name;
    std::string spriteFrameName;
    TargetPriority aiType;
    
    std::vector<float> HP;
    std::vector<float> DP;
    std::vector<float> ATK;
    std::vector<float> ATK_RANGE;
    std::vector<float> ATK_SPEED;
    bool SKY_ABLE;
    bool GROUND_ABLE;
    
    int length;
    int width;
    std::vector<float> BUILD_TIME;
    std::vector<int> COST_GOLD;
    std::vector<int> COST_ELIXIR;
    int MAXLEVEL;
    
    std::string anim_idle = "idle";
    int anim_idle_frames = 1;
    float anim_idle_delay = 0.1f;
    std::string anim_attack = "attack";
    int anim_attack_frames = 1;
    float anim_attack_delay = 0.1f;

    std::string bulletSpriteFrameName;
    float bulletSpeed = 0.0f;
    bool bulletIsAOE = false;
    float bulletAOERange = 0.0f;
};

#endif // __DEFENSE_BUILDING_DATA_H__
