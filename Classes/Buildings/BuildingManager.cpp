// BuildingManager.cpp
#include "BuildingManager.h"
#include "json/document.h"
#include "json/rapidjson.h"
#include <string>

USING_NS_CC;

namespace {
void stripUtf8Bom(std::string& text) {
    if (text.size() >= 3 &&
        static_cast<unsigned char>(text[0]) == 0xEF &&
        static_cast<unsigned char>(text[1]) == 0xBB &&
        static_cast<unsigned char>(text[2]) == 0xBF) {
        text.erase(0, 3);
    }
}

std::vector<float> parseFloatArray(const rapidjson::Value& arr) {
    std::vector<float> result;
    if (!arr.IsArray()) {
        return result;
    }
    result.reserve(arr.Size());
    for (auto& v : arr.GetArray()) {
        if (v.IsNumber()) {
            result.push_back(static_cast<float>(v.GetDouble()));
        }
    }
    return result;
}

std::vector<int> parseIntArray(const rapidjson::Value& arr) {
    std::vector<int> result;
    if (!arr.IsArray()) {
        return result;
    }
    result.reserve(arr.Size());
    for (auto& v : arr.GetArray()) {
        if (v.IsInt()) {
            result.push_back(v.GetInt());
        } else if (v.IsNumber()) {
            result.push_back(static_cast<int>(v.GetDouble()));
        }
    }
    return result;
}

int readInt(const rapidjson::Value& obj, const char* key, int fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) {
        return fallback;
    }
    if (it->value.IsInt()) {
        return it->value.GetInt();
    }
    if (it->value.IsNumber()) {
        return static_cast<int>(it->value.GetDouble());
    }
    return fallback;
}

float readFloat(const rapidjson::Value& obj, const char* key, float fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) {
        return fallback;
    }
    if (it->value.IsNumber()) {
        return static_cast<float>(it->value.GetDouble());
    }
    return fallback;
}

bool readBool(const rapidjson::Value& obj, const char* key, bool fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) {
        return fallback;
    }
    if (it->value.IsBool()) {
        return it->value.GetBool();
    }
    return fallback;
}

std::string readString(const rapidjson::Value& obj, const char* key, const std::string& fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) {
        return fallback;
    }
    if (it->value.IsString()) {
        return it->value.GetString();
    }
    return fallback;
}

int readMaxLevel(const rapidjson::Value& obj, int fallback) {
    if (obj.HasMember("MAXLEVEL") && obj["MAXLEVEL"].IsInt()) {
        return obj["MAXLEVEL"].GetInt();
    }
    if (obj.HasMember("MAX_LEVEL") && obj["MAX_LEVEL"].IsInt()) {
        return obj["MAX_LEVEL"].GetInt();
    }
    return fallback;
}

TargetPriority readTargetPriority(const rapidjson::Value& obj, TargetPriority fallback) {
    if (!obj.HasMember("aiType")) {
        return fallback;
    }
    const auto& value = obj["aiType"];
    if (value.IsInt()) {
        int aiType = value.GetInt();
        if (aiType == 1) {
            return TargetPriority::RESOURCE;
        }
        if (aiType == 2) {
            return TargetPriority::DEFENSE;
        }
    }
    return fallback;
}

BuildingCategory parseCategory(const std::string& raw) {
    if (raw == "defence" || raw == "defense") {
        return BuildingCategory::Defence;
    }
    if (raw == "production") {
        return BuildingCategory::Production;
    }
    if (raw == "storage") {
        return BuildingCategory::Storage;
    }
    if (raw == "trap") {
        return BuildingCategory::Trap;
    }
    return BuildingCategory::Unknown;
}
} // namespace

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
    if (_configsLoaded) {
        return;
    }
    _configsLoaded = true;

    _defenceConfigs.clear();
    _productionConfigs.clear();
    _storageConfigs.clear();
    _buildOptions.clear();
    _battleTowerConfigIds.clear();
    _mainBaseId = 0;
    _barracksId = 0;
    _enemyBaseId = 0;

    const char* configPath = "res/buildings_config.json";
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(configPath);
    std::string jsonData = FileUtils::getInstance()->getStringFromFile(fullPath);
    if (jsonData.empty()) {
        CCLOG("BuildingManager: Failed to load config file: %s", configPath);
        return;
    }
    stripUtf8Bom(jsonData);

    rapidjson::Document doc;
    doc.Parse(jsonData.c_str());
    if (doc.HasParseError()) {
        CCLOG("BuildingManager: JSON parse error at offset %zu", doc.GetErrorOffset());
        return;
    }

    if (doc.HasMember("defenceBuildings") && doc["defenceBuildings"].IsArray()) {
        for (const auto& item : doc["defenceBuildings"].GetArray()) {
            if (!item.IsObject()) {
                continue;
            }
            DefenceBuildingConfig config{};
            config.aiType = readTargetPriority(item, TargetPriority::ANY);
            config.id = readInt(item, "id", 0);
            if (config.id <= 0) {
                continue;
            }
            config.name = readString(item, "name", "");
            config.spriteFrameName = readString(item, "spriteFrameName", "");
            if (config.spriteFrameName.empty()) {
                config.spriteFrameName = readString(item, "spritePath", "");
            }
            if (item.HasMember("HP")) config.HP = parseFloatArray(item["HP"]);
            if (item.HasMember("DP")) config.DP = parseFloatArray(item["DP"]);
            if (item.HasMember("ATK")) config.ATK = parseFloatArray(item["ATK"]);
            if (item.HasMember("ATK_RANGE")) config.ATK_RANGE = parseFloatArray(item["ATK_RANGE"]);
            if (item.HasMember("ATK_SPEED")) config.ATK_SPEED = parseFloatArray(item["ATK_SPEED"]);
            config.SKY_ABLE = readBool(item, "SKY_ABLE", false);
            config.GROUND_ABLE = readBool(item, "GROUND_ABLE", true);
            config.length = readInt(item, "length", 0);
            config.width = readInt(item, "width", 0);
            if (item.HasMember("BUILD_TIME")) config.BUILD_TIME = parseFloatArray(item["BUILD_TIME"]);
            if (item.HasMember("COST_GOLD")) config.COST_GOLD = parseIntArray(item["COST_GOLD"]);
            if (item.HasMember("COST_ELIXIR")) config.COST_ELIXIR = parseIntArray(item["COST_ELIXIR"]);
            int maxLevelFallback = config.HP.empty() ? 0 : static_cast<int>(config.HP.size()) - 1;
            config.MAXLEVEL = readMaxLevel(item, maxLevelFallback);

            config.anim_idle = readString(item, "anim_idle", config.anim_idle);
            config.anim_idle_frames = readInt(item, "anim_idle_frames", config.anim_idle_frames);
            config.anim_idle_delay = readFloat(item, "anim_idle_delay", config.anim_idle_delay);
            config.anim_attack = readString(item, "anim_attack", config.anim_attack);
            config.anim_attack_frames = readInt(item, "anim_attack_frames", config.anim_attack_frames);
            config.anim_attack_delay = readFloat(item, "anim_attack_delay", config.anim_attack_delay);

            config.bulletSpriteFrameName = readString(item, "bulletSpriteFrameName", "");
            if (config.bulletSpriteFrameName.empty()) {
                config.bulletSpriteFrameName = readString(item, "bulletSpritePath", "");
            }
            config.bulletSpeed = readFloat(item, "bulletSpeed", 0.0f);
            config.bulletIsAOE = readBool(item, "bulletIsAOE", false);
            config.bulletAOERange = readFloat(item, "bulletAOERange", 0.0f);

            _defenceConfigs[config.id] = config;
        }
    }

    if (doc.HasMember("productionBuildings") && doc["productionBuildings"].IsArray()) {
        for (const auto& item : doc["productionBuildings"].GetArray()) {
            if (!item.IsObject()) {
                continue;
            }
            ProductionBuildingConfig config{};
            config.aiType = readTargetPriority(item, TargetPriority::ANY);
            config.id = readInt(item, "id", 0);
            if (config.id <= 0) {
                continue;
            }
            config.name = readString(item, "name", "");
            config.spriteFrameName = readString(item, "spriteFrameName", "");
            if (config.spriteFrameName.empty()) {
                config.spriteFrameName = readString(item, "spritePath", "");
            }
            if (item.HasMember("HP")) config.HP = parseFloatArray(item["HP"]);
            if (item.HasMember("DP")) config.DP = parseFloatArray(item["DP"]);
            config.length = readInt(item, "length", 0);
            config.width = readInt(item, "width", 0);
            if (item.HasMember("BUILD_TIME")) config.BUILD_TIME = parseFloatArray(item["BUILD_TIME"]);
            if (item.HasMember("COST_GOLD")) config.COST_GOLD = parseIntArray(item["COST_GOLD"]);
            if (item.HasMember("COST_ELIXIR")) config.COST_ELIXIR = parseIntArray(item["COST_ELIXIR"]);
            if (item.HasMember("PRODUCE_ELIXIR")) config.PRODUCE_ELIXIR = parseIntArray(item["PRODUCE_ELIXIR"]);
            if (item.HasMember("STORAGE_ELIXIR_CAPACITY")) config.STORAGE_ELIXIR_CAPACITY = parseIntArray(item["STORAGE_ELIXIR_CAPACITY"]);
            if (item.HasMember("PRODUCE_GOLD")) config.PRODUCE_GOLD = parseIntArray(item["PRODUCE_GOLD"]);
            if (item.HasMember("STORAGE_GOLD_CAPACITY")) config.STORAGE_GOLD_CAPACITY = parseIntArray(item["STORAGE_GOLD_CAPACITY"]);
            int maxLevelFallback = config.HP.empty() ? 0 : static_cast<int>(config.HP.size()) - 1;
            config.MAXLEVEL = readMaxLevel(item, maxLevelFallback);

            config.anim_idle = readString(item, "anim_idle", config.anim_idle);
            config.anim_idle_frames = readInt(item, "anim_idle_frames", config.anim_idle_frames);
            config.anim_idle_delay = readFloat(item, "anim_idle_delay", config.anim_idle_delay);
            config.anim_produce = readString(item, "anim_produce", config.anim_produce);
            config.anim_produce_frames = readInt(item, "anim_produce_frames", config.anim_produce_frames);
            config.anim_produce_delay = readFloat(item, "anim_produce_delay", config.anim_produce_delay);

            if (readBool(item, "isMainBase", false)) {
                _mainBaseId = config.id;
            }
            if (readBool(item, "isBarracks", false)) {
                _barracksId = config.id;
            }
            if (readBool(item, "isEnemyBase", false)) {
                _enemyBaseId = config.id;
            }

            _productionConfigs[config.id] = config;
        }
    }

    if (doc.HasMember("storageBuildings") && doc["storageBuildings"].IsArray()) {
        for (const auto& item : doc["storageBuildings"].GetArray()) {
            if (!item.IsObject()) {
                continue;
            }
            StorageBuildingConfig config{};
            config.aiType = readTargetPriority(item, TargetPriority::ANY);
            config.id = readInt(item, "id", 0);
            if (config.id <= 0) {
                continue;
            }
            config.name = readString(item, "name", "");
            config.spriteFrameName = readString(item, "spriteFrameName", "");
            if (config.spriteFrameName.empty()) {
                config.spriteFrameName = readString(item, "spritePath", "");
            }
            if (item.HasMember("HP")) config.HP = parseFloatArray(item["HP"]);
            if (item.HasMember("DP")) config.DP = parseFloatArray(item["DP"]);
            config.length = readInt(item, "length", 0);
            config.width = readInt(item, "width", 0);
            if (item.HasMember("BUILD_TIME")) config.BUILD_TIME = parseFloatArray(item["BUILD_TIME"]);
            if (item.HasMember("COST_GOLD")) config.COST_GOLD = parseIntArray(item["COST_GOLD"]);
            if (item.HasMember("COST_ELIXIR")) config.COST_ELIXIR = parseIntArray(item["COST_ELIXIR"]);
            if (item.HasMember("ADD_STORAGE_ELIXIR_CAPACITY")) {
                config.ADD_STORAGE_ELIXIR_CAPACITY = parseIntArray(item["ADD_STORAGE_ELIXIR_CAPACITY"]);
            }
            if (item.HasMember("ADD_STORAGE_GOLD_CAPACITY")) {
                config.ADD_STORAGE_GOLD_CAPACITY = parseIntArray(item["ADD_STORAGE_GOLD_CAPACITY"]);
            }
            int maxLevelFallback = config.HP.empty() ? 0 : static_cast<int>(config.HP.size()) - 1;
            config.MAXLEVEL = readMaxLevel(item, maxLevelFallback);

            config.anim_idle = readString(item, "anim_idle", config.anim_idle);
            config.anim_idle_frames = readInt(item, "anim_idle_frames", config.anim_idle_frames);
            config.anim_idle_delay = readFloat(item, "anim_idle_delay", config.anim_idle_delay);

            _storageConfigs[config.id] = config;
        }
    }

    if (doc.HasMember("shopOptions") && doc["shopOptions"].IsArray()) {
        for (const auto& item : doc["shopOptions"].GetArray()) {
            if (!item.IsObject()) {
                continue;
            }
            BuildingOption option;
            option.type = readInt(item, "type", 0);
            option.configId = readInt(item, "configId", option.type);
            option.name = readString(item, "name", "");
            option.cost = readInt(item, "cost", 0);
            option.gridWidth = readInt(item, "gridWidth", 0);
            option.gridHeight = readInt(item, "gridHeight", 0);
            option.spritePath = readString(item, "spritePath", "");
            option.canBuild = readBool(item, "canBuild", true);
            option.category = parseCategory(readString(item, "category", ""));

            if (option.type <= 0) {
                continue;
            }
            _buildOptions.push_back(option);
        }
    }

    if (doc.HasMember("battleTowers") && doc["battleTowers"].IsArray()) {
        for (const auto& item : doc["battleTowers"].GetArray()) {
            if (!item.IsObject()) {
                continue;
            }
            int towerType = readInt(item, "towerType", 0);
            int configId = readInt(item, "configId", 0);
            if (towerType > 0 && configId > 0) {
                _battleTowerConfigIds[towerType] = configId;
            }
        }
    }

    if (_mainBaseId == 0) {
        for (const auto& pair : _productionConfigs) {
            if (pair.second.name == "Base") {
                _mainBaseId = pair.first;
                break;
            }
        }
    }
    if (_barracksId == 0) {
        for (const auto& pair : _productionConfigs) {
            if (pair.second.name == "SoldierBuilder") {
                _barracksId = pair.first;
                break;
            }
        }
    }
    if (_enemyBaseId == 0) {
        for (const auto& pair : _productionConfigs) {
            if (pair.second.name == "EnemyBase") {
                _enemyBaseId = pair.first;
                break;
            }
        }
    }
}

DefenceBuilding* BuildingManager::createDefenceBuilding(int buildingId, int level) {
    loadConfigs();
    const DefenceBuildingConfig* config = getDefenceConfig(buildingId);
    return config ? DefenceBuilding::create(config, level) : nullptr;
}

ProductionBuilding* BuildingManager::createProductionBuilding(int buildingId, int level) {
    loadConfigs();
    const ProductionBuildingConfig* config = getProductionConfig(buildingId);
    return config ? ProductionBuilding::create(config, level) : nullptr;
}

StorageBuilding* BuildingManager::createStorageBuilding(int buildingId, int level) {
    loadConfigs();
    const StorageBuildingConfig* config = getStorageConfig(buildingId);
    return config ? StorageBuilding::create(config, level) : nullptr;
}

const DefenceBuildingConfig* BuildingManager::getDefenceConfig(int buildingId) const {
    auto it = _defenceConfigs.find(buildingId);
    if (it != _defenceConfigs.end()) {
        return &it->second;
    }
    return nullptr;
}

const ProductionBuildingConfig* BuildingManager::getProductionConfig(int buildingId) const {
    auto it = _productionConfigs.find(buildingId);
    if (it != _productionConfigs.end()) {
        return &it->second;
    }
    return nullptr;
}

const StorageBuildingConfig* BuildingManager::getStorageConfig(int buildingId) const {
    auto it = _storageConfigs.find(buildingId);
    if (it != _storageConfigs.end()) {
        return &it->second;
    }
    return nullptr;
}

int BuildingManager::getBattleTowerConfigId(int towerType) const {
    auto it = _battleTowerConfigIds.find(towerType);
    if (it != _battleTowerConfigIds.end()) {
        return it->second;
    }
    return 0;
}

bool BuildingManager::placeBuilding(Node* building, int gridX, int gridY, int width, int height) {
    if (!_gridMap) return false;

    if (_gridMap->canPlaceBuilding(gridX, gridY, width, height)) {
        _gridMap->occupyCell(gridX, gridY, width, height, building);
        // 计算建筑中心位置：左下角格子位置 + 建筑尺寸的一半
        // 使用浮点数计算避免整数除法导致的偏移
        float cellSize = _gridMap->getCellSize();
        float centerX = (gridX + width * 0.5f) * cellSize;
        float centerY = (gridY + height * 0.5f) * cellSize;
        building->setPosition(Vec2(centerX, centerY));
        return true;
    }
    return false;
}

void BuildingManager::destroyBuilding(Node* building, int gridX, int gridY, int width, int height) {
    if (!_gridMap) return;
    _gridMap->freeCell(gridX, gridY, width, height);
}
