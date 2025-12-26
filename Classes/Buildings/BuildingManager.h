// BuildingManager.h
#ifndef __BUILDING_MANAGER_H__
#define __BUILDING_MANAGER_H__

#include "cocos2d.h"
#include "Buildings/BuildingCatalog.h"
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
    const DefenceBuildingConfig* getDefenceConfig(int buildingId) const;
    const ProductionBuildingConfig* getProductionConfig(int buildingId) const;
    const StorageBuildingConfig* getStorageConfig(int buildingId) const;
    const std::vector<BuildingOption>& getBuildOptions() const { return _buildOptions; }
    int getMainBaseId() const { return _mainBaseId; }
    int getBarracksId() const { return _barracksId; }
    int getEnemyBaseId() const { return _enemyBaseId; }
    int getBattleTowerConfigId(int towerType) const;

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
    std::vector<BuildingOption> _buildOptions;
    std::map<int, int> _battleTowerConfigIds;
    int _mainBaseId = 0;
    int _barracksId = 0;
    int _enemyBaseId = 0;
    bool _configsLoaded = false;
};

#endif // __BUILDING_MANAGER_H__
