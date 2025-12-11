// BuildingManager.cpp
#include "BuildingManager.h"

USING_NS_CC;

BuildingManager* BuildingManager::_instance = nullptr;

BuildingManager::BuildingManager() : _gridMap(nullptr) {
}

BuildingManager::~BuildingManager() {
}

BuildingManager* BuildingManager::getInstance() {
    if (!_instance) {
        _instance = new BuildingManager();
    }
    return _instance;
}

void BuildingManager::loadConfigs() {
    // TODO: Load from JSON files when available
    // For now, create default configurations
}

DefenceBuilding* BuildingManager::createDefenceBuilding(int buildingId, int level) {
    auto it = _defenceConfigs.find(buildingId);
    if (it != _defenceConfigs.end()) {
        return DefenceBuilding::create(&it->second, level);
    }
    return nullptr;
}

ProductionBuilding* BuildingManager::createProductionBuilding(int buildingId, int level) {
    auto it = _productionConfigs.find(buildingId);
    if (it != _productionConfigs.end()) {
        return ProductionBuilding::create(&it->second, level);
    }
    return nullptr;
}

StorageBuilding* BuildingManager::createStorageBuilding(int buildingId, int level) {
    auto it = _storageConfigs.find(buildingId);
    if (it != _storageConfigs.end()) {
        return StorageBuilding::create(&it->second, level);
    }
    return nullptr;
}

bool BuildingManager::placeBuilding(Node* building, int gridX, int gridY, int width, int height) {
    if (!_gridMap) return false;

    if (_gridMap->canPlaceBuilding(gridX, gridY, width, height)) {
        _gridMap->occupyCell(gridX, gridY, width, height, building);
        Vec2 worldPos = _gridMap->gridToWorld(gridX + width / 2, gridY + height / 2);
        building->setPosition(worldPos);
        return true;
    }
    return false;
}

void BuildingManager::destroyBuilding(Node* building, int gridX, int gridY, int width, int height) {
    if (!_gridMap) return;
    _gridMap->freeCell(gridX, gridY, width, height);
}
