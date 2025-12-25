// UnitData.h
#ifndef __UNIT_DATA_H__
#define __UNIT_DATA_H__

#include "cocos2d.h"
#include <vector>

// 目标优先级
enum class TargetPriority {
    ANY = 0,        // 任意目标（默认）
    RESOURCE = 1,   // 优先资源建筑（生产/仓库）
    DEFENSE = 2     // 优先防御建筑
};

// 朝向枚举
// 默认使用朝右(RIGHT)帧资源，朝左可通过翻转精灵实现
// 帧命名规则：{spriteFrameName}_{anim_key}_{frame_number}.png
// 例如 goblin_walk_1.png, goblin_walk_2.png
enum class Direction {
    LEFT = 0,       // 左
    RIGHT = 1       // 右
};

// 兵种配置（通常来自 JSON）
struct UnitConfig {
// 基础信息
    int id;                     // 兵种ID，例如 1001
    std::string name;           // 名称，如 "Goblin"
    std::string spriteFrameName;// 贴图前缀，如 "goblin" 或 "unit/xxx/archer"
    
// 等级数据（按等级索引）
    std::vector<float> HP;      // 生命值
    std::vector<float> SPEED;   // 移动速度
    std::vector<float> DP;      // 防御/减伤
    std::vector<float> ATK;     // 攻击力
    std::vector<float> RANGE;   // 攻击范围
    
    TargetPriority aiType;      // 目标优先级（0/1/2）
    
    // 兵种属性
    bool ISREMOTE;              // 远程单位
    bool ISFLY;                 // 飞行单位

    // 等级上限
    int MAXLEVEL;

    // 训练消耗
    int COST_COIN;
    int COST_ELIXIR;
    int COST_POPULATION;
    int TRAIN_TIME; // 训练时间（秒）

    // 动画配置
    // 帧命名：{spriteFrameName}_{anim_key}_{frame_number}.png
    // 例如 goblin_walk_1.png, goblin_walk_2.png, ...
    std::string anim_walk = "walk";        // 行走动画名
    int anim_walk_frames = 6;              // 行走帧数
    float anim_walk_delay = 0.08f;         // 行走帧间隔
    
    std::string anim_attack = "attack";    // 攻击动画名
    int anim_attack_frames = 8;            // 攻击帧数
    float anim_attack_delay = 0.06f;       // 攻击帧间隔
    
    std::string anim_idle = "idle";        // 待机动画名
    int anim_idle_frames = 4;              // 待机帧数
    float anim_idle_delay = 0.15f;         // 待机帧间隔

    std::string anim_dead = "dead";        // 死亡动画名
    int anim_dead_frames = 4;              // 死亡帧数
    float anim_dead_delay = 0.06f;         // 死亡帧间隔
};

#endif // __UNIT_DATA_H__
