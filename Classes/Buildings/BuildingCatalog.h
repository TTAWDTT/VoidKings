#ifndef __BUILDING_CATALOG_H__
#define __BUILDING_CATALOG_H__

#include <string>

enum class BuildingCategory {
    Defence,
    Production,
    Storage,
    Trap,
    Unknown
};

struct BuildingOption {
    int type = 0;
    int configId = 0;
    BuildingCategory category = BuildingCategory::Unknown;
    std::string name;
    int cost = 0;
    int gridWidth = 0;
    int gridHeight = 0;
    std::string spritePath;
    bool canBuild = true;
};

#endif // __BUILDING_CATALOG_H__
