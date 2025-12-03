// BuildingLimits.h
// 建筑数量上限与解锁规则

#ifndef __BUILDING_LIMITS_H__
#define __BUILDING_LIMITS_H__

#include "../Core/GameDefines.h"
#include <map>
#include <vector>

// 负责根据大本营等级返回每种建筑的可建造总数上限
class BuildingLimits {
public:
    // 初始化默认规则（幂等）
    static void initDefaults();

    // 获取在给定大本营等级下某建筑的总上限（包含初始赠送的建筑）
    static int getMaxCount(BuildingType type, int townHallLevel);

private:
    static bool s_inited;
    static std::map<BuildingType, std::vector<int>> s_limitsByTH; // TH 等级从 1 开始的上限表
};

#endif // __BUILDING_LIMITS_H__
