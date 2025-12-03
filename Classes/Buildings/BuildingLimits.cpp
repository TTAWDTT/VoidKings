// BuildingLimits.cpp
// 实现建筑数量上限与解锁规则

#include "BuildingLimits.h"

bool BuildingLimits::s_inited = false;
std::map<BuildingType, std::vector<int>> BuildingLimits::s_limitsByTH;

void BuildingLimits::initDefaults() {
    if (s_inited) return;
    s_inited = true;

    // 统一仅配置 TH 1..5（当前项目建筑的最大等级也是 5）
    // 向量下标：0 未用，1..5 为对应 TH 等级的总上限

    auto pad = [](std::initializer_list<int> v){
        std::vector<int> r(6, 0); // 索引 0 空
        int i = 1;
        for (int x : v) { if (i <= 5) r[i++] = x; }
        // 若给定不足 5 个，用最后一个值填充
        for (; i <= 5; ++i) r[i] = r[i-1];
        return r;
    };

    s_limitsByTH[BuildingType::TOWN_HALL]       = pad({1,1,1,1,1});
    s_limitsByTH[BuildingType::CANNON]          = pad({0,1,2,3,4});
    s_limitsByTH[BuildingType::ARCHER_TOWER]    = pad({0,0,1,2,3});
    s_limitsByTH[BuildingType::MORTAR]          = pad({0,0,0,1,1});

    s_limitsByTH[BuildingType::GOLD_MINE]       = pad({1,2,3,3,4});
    s_limitsByTH[BuildingType::ELIXIR_COLLECTOR]= pad({1,2,3,3,4});

    s_limitsByTH[BuildingType::GOLD_STORAGE]    = pad({1,1,2,2,3});
    s_limitsByTH[BuildingType::ELIXIR_STORAGE]  = pad({1,1,2,2,3});

    s_limitsByTH[BuildingType::BARRACKS]        = pad({1,1,2,2,3});
    s_limitsByTH[BuildingType::SPELL_FACTORY]   = pad({0,0,0,1,1});

    // 墙体按片段计数，给一个较高的上限
    s_limitsByTH[BuildingType::WALL]            = pad({20,50,75,100,125});
}

int BuildingLimits::getMaxCount(BuildingType type, int townHallLevel) {
    initDefaults();
    if (townHallLevel < 1) townHallLevel = 1;
    if (townHallLevel > 5) townHallLevel = 5;
    auto it = s_limitsByTH.find(type);
    if (it == s_limitsByTH.end()) return 0;
    const auto &vec = it->second;
    if ((int)vec.size() <= townHallLevel) return 0;
    return vec[townHallLevel];
}
