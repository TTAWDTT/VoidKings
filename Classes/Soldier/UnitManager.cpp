// UnitManager.cpp
#include "UnitManager.h"

// 单例实例
UnitManager* UnitManager::_instance = nullptr;

UnitManager* UnitManager::getInstance() {
    if (!_instance) {
        _instance = new UnitManager();
    }
    return _instance;
}


/*
注意：
1.存入缓存后，后续创建Soldier时直接从_configCache中读取，读取时需要使用ID
2.相似实现进行的时候可以抄，但是要注意变量名和结构体字段名的对应关系
3.该部分，包括加载配置和解析配置，均需要使用rapidjson库，注意在头文件中include，关于该库的细节我（@TTAWDTT）会在wiki中说明
*/

// 从json文件中加载配置，输入为文件路径，返回是否成功
bool UnitManager::loadConfig(const std::string& jsonFile) {
    // 1. 读取JSON文件
    std::string fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(jsonFile);
    std::string jsonData = cocos2d::FileUtils::getInstance()->getStringFromFile(fullPath);
    
    if (jsonData.empty()) {
        cocos2d::log("UnitManager: Failed to load config file: %s", jsonFile.c_str());
        return false;
    }

    // 2. 解析JSON -> 树结构
    rapidjson::Document doc;
    doc.Parse(jsonData.c_str());
    
    if (doc.HasParseError()) {
        cocos2d::log("UnitManager: JSON parse error at offset %zu", doc.GetErrorOffset());
        return false;
    }

    // 3. 检查是否有units数组 -> 解析后的顶层必须是units数组
    if (!doc.HasMember("units") || !doc["units"].IsArray()) {
        cocos2d::log("UnitManager: Missing 'units' array in config");
        return false;
    }

    // 4. 遍历解析每个单位配置
    const rapidjson::Value& units = doc["units"]; // 取出顶层unit数组便于便利
    for (rapidjson::SizeType i = 0; i < units.Size(); i++) {
        UnitConfig config;
		if (parseUnitConfig(units[i], config)) { // 将units[i]的数据解析到config结构体中
            _configCache[config.id] = config; // 解析后存入缓存，方便创建时调用
            cocos2d::log("UnitManager: Loaded unit [%d] %s", config.id, config.name.c_str());
        }
    }

    cocos2d::log("UnitManager: Successfully loaded %zu units", _configCache.size());
    return true;
}

// 将units[i]的数据解析到config结构体中（带默认值，孟哥设计的时候可以修改一下数值）
// 后续根据默认值来设计config中各个种类的数据差异
bool UnitManager::parseUnitConfig(const rapidjson::Value& unitJson, UnitConfig& config) {
    // 必需字段检查
    if (!unitJson.HasMember("id") || !unitJson["id"].IsInt()) {
        cocos2d::log("UnitManager: Unit missing 'id' field");
        return false;
    }

    // 基础信息
    config.id = unitJson["id"].GetInt();
    config.name = unitJson.HasMember("name") ? unitJson["name"].GetString() : "Unknown";
    config.spriteFrameName = unitJson.HasMember("spriteFrameName") ? unitJson["spriteFrameName"].GetString() : "";

    // 等级相关数据 - 数组形式
    if (unitJson.HasMember("HP") && unitJson["HP"].IsArray()) {
        config.HP = parseFloatArray(unitJson["HP"]);
    }
    if (unitJson.HasMember("SPEED") && unitJson["SPEED"].IsArray()) {
        config.SPEED = parseFloatArray(unitJson["SPEED"]);
    }
    if (unitJson.HasMember("DP") && unitJson["DP"].IsArray()) {
        config.DP = parseFloatArray(unitJson["DP"]);
    }
    if (unitJson.HasMember("ATK") && unitJson["ATK"].IsArray()) {
        config.ATK = parseFloatArray(unitJson["ATK"]);
    }
    if (unitJson.HasMember("RANGE") && unitJson["RANGE"].IsArray()) {
        config.RANGE = parseFloatArray(unitJson["RANGE"]);
    }

    // AI类型
    int aiTypeValue = unitJson.HasMember("aiType") ? unitJson["aiType"].GetInt() : 0;
    config.aiType = parseAIType(aiTypeValue);

    // 布尔属性
    config.ISREMOTE = unitJson.HasMember("ISREMOTE") ? unitJson["ISREMOTE"].GetBool() : false;
    config.ISFLY = unitJson.HasMember("ISFLY") ? unitJson["ISFLY"].GetBool() : false;

    // 等级上限
    config.MAXLEVEL = unitJson.HasMember("MAXLEVEL") ? unitJson["MAXLEVEL"].GetInt() : 1;

    // 资源消耗
    config.COST_COIN = unitJson.HasMember("COST_COIN") ? unitJson["COST_COIN"].GetInt() : 0;
    config.COST_ELIXIR = unitJson.HasMember("COST_ELIXIR") ? unitJson["COST_ELIXIR"].GetInt() : 0;
    config.COST_POPULATION = unitJson.HasMember("COST_POPULATION") ? unitJson["COST_POPULATION"].GetInt() : 1;
    config.TRAIN_TIME = unitJson.HasMember("TRAIN_TIME") ? unitJson["TRAIN_TIME"].GetInt() : 30;

    return true;
}

std::vector<float> UnitManager::parseFloatArray(const rapidjson::Value& arr) {
    std::vector<float> result;
    for (rapidjson::SizeType i = 0; i < arr.Size(); i++) {
        if (arr[i].IsNumber()) {
            result.push_back(static_cast<float>(arr[i].GetDouble()));
        }
    }
    return result;
}

TargetPriority UnitManager::parseAIType(int aiType) {
    switch (aiType) {
        case 1: return TargetPriority::RESOURCE;
        case 2: return TargetPriority::DEFENSE;
        default: return TargetPriority::ANY;
    }
}

const UnitConfig* UnitManager::getConfig(int unitId) const {
    auto it = _configCache.find(unitId);
    if (it != _configCache.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool UnitManager::hasConfig(int unitId) const {
    return _configCache.find(unitId) != _configCache.end();
}

Soldier* UnitManager::spawnSoldier(int unitId, cocos2d::Vec2 position, int level) {
    // 1. 查找配置
    const UnitConfig* cfg = getConfig(unitId);
    if (!cfg) {
        cocos2d::log("UnitManager: Unit config not found for id: %d", unitId);
        return nullptr;
    }

    // 2. 使用通用类 + 特定配置创建士兵
    auto soldier = Soldier::create(cfg, level);
    if (soldier) {
        soldier->setPosition(position);
    }

    return soldier;
}