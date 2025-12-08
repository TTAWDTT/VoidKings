// UnitData.h
#include "cocos2d.h"
#include <vector>

// 定义攻击偏好类型
enum class TargetPriority {
    ANY = 0,        // 任意建筑 (野蛮人)
    RESOURCE = 1,   // 资源优先 (哥布林)
    DEFENSE = 2     // 防御塔 (气球)
};

// 方向枚举
// 注意：只需要向右方向(RIGHT)的动画资源，向左方向通过图片翻转实现
// 帧命名规范：{spriteFrameName}_{anim_key}_{frame_number}.png
// 例如：goblin_walk_1.png, goblin_walk_2.png
enum class Direction {
    LEFT = 0,       // 左
    RIGHT = 1       // 右
};

// 兵种配置,用于存储各种兵的数据
struct UnitConfig {
// 基础信息
    int id;                     // 兵种编号如1001
    std::string name;           // 如"Goblin"
    std::string spriteFrameName;// 如"goblin01.png" 
    
// 属性数据 - 修改为按等级的vector数组
    std::vector<float> HP;      // 每个等级的血量
    std::vector<float> SPEED;   // 每个等级的移动速度
    std::vector<float> DP;      // 每个等级的防御力
    std::vector<float> ATK;     // 每个等级的攻击力
    std::vector<float> RANGE;   // 每个等级的攻击范围
    
    TargetPriority aiType;      // 如：2 - 这个字段决定这个是哥布林
    
    // 其他属性
    bool ISREMOTE;
    bool ISFLY;

    // 最大等级
int MAXLEVEL;

    // 资源消耗
int COST_COIN;
int COST_ELIXIR;
    int COST_POPULATION;
int TRAIN_TIME; // 训练时间,单位秒
    // 动画配置：从精灵图集中读取帧序
    // 帧命名规范：{spriteFrameName}_{anim_key}_{frame_number}.png
    // 例如：goblin_walk_1.png, goblin_walk_2.png, ...
    std::string anim_walk = "walk";        // 移动动画关键字
    int anim_walk_frames = 6;              // 移动动画帧数
    float anim_walk_delay = 0.08f;         // 每帧延迟（秒）
    
    std::string anim_attack = "attack";    // 攻击动画关键字
    int anim_attack_frames = 8;            // 攻击动画帧数
    float anim_attack_delay = 0.06f;       // 每帧延迟（秒）
    
    std::string anim_idle = "idle";        // 待机动画关键字（可选）
    int anim_idle_frames = 4;              // 待机动画帧数
    float anim_idle_delay = 0.15f;         // 每帧延迟（秒）

    std::string anim_dead = "dead";
    int anim_dead_frames = 4;              // 死亡动画帧数
    float anim_dead_delay = 0.06f;         // 每帧延迟（秒）
};