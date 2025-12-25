// BuildingManager.h
#ifndef __BUILDING_MANAGER_H__
#define __BUILDING_MANAGER_H__

#include "cocos2d.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Buildings/StorageBuilding.h"
#include "Map/GridMap.h"
#include <map>

class BuildingManager {
public:
    static BuildingManager* getInstance();

    void setGridMap(GridMap* gridMap) { _gridMap = gridMap; }
    GridMap* getGridMap() const { return _gridMap; }

    // Load configurations
    void loadConfigs();

    // Factory methods
    DefenceBuilding* createDefenceBuilding(int buildingId, int level = 0);
    ProductionBuilding* createProductionBuilding(int buildingId, int level = 0);
    StorageBuilding* createStorageBuilding(int buildingId, int level = 0);

    // Building operations
    bool placeBuilding(cocos2d::Node* building, int gridX, int gridY, int width, int height);
    void destroyBuilding(cocos2d::Node* building, int gridX, int gridY, int width, int height);

private:
    BuildingManager();
    ~BuildingManager();

    static BuildingManager* _instance;
    GridMap* _gridMap;

    std::map<int, DefenceBuildingConfig> _defenceConfigs;
    std::map<int, ProductionBuildingConfig> _productionConfigs;
    std::map<int, StorageBuildingConfig> _storageConfigs;
};

#endif // __BUILDING_MANAGER_H__
