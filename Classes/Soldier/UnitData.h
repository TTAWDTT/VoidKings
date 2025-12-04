// UnitData.h
#include "cocos2d.h"
#include <vector>

// 定义攻击偏好类型
enum class TargetPriority {
    ANY = 0,        // 任意建筑 (野蛮人)
    RESOURCE = 1,   // 资源建筑 (哥布林)
    DEFENSE = 2     // 防御塔 (胖子)
};

// 纯数据类,用来存储兵种的属性
struct UnitConfig {
	// 基本信息
    int id;                     // 方便管理1001
    std::string name;           // 如"Goblin"
    std::string spriteFrameName;// 如"goblin01.png" 
    
	// 基础属性 - 修改为按等级的vector数组
    std::vector<float> HP;      // 每个等级的血量
    std::vector<float> SPEED;   // 每个等级的移动速度
    std::vector<float> DP;      // 每个等级的防御力
    std::vector<float> ATK;     // 每个等级的攻击力
    std::vector<float> RANGE;   // 每个等级的攻击范围
    
    TargetPriority aiType;      // 如：2 - 这个字段决定了它是哥布林
    
    // 特殊兵种
    bool ISREMOTE;
    bool ISFLY;

    // 最大等级
	int MAXLEVEL;

    // 资源消耗
	int COST_COIN;
	int COST_ELIXIR;
    int COST_POPULATION;
	int TRAIN_TIME; // 训练时间,单位秒
};