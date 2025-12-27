// UnitManager.h
#ifndef __UNIT_MANAGER_H__
#define __UNIT_MANAGER_H__

#include "cocos2d.h"
#include "UnitData.h"
#include "Soldier.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include <map>
#include <string>

class UnitManager {
public:
    static UnitManager* getInstance();

    // 加载配置 (比如在游戏启动时调用)
    bool loadConfig(const std::string& jsonFile);

    // 造兵工厂方法
    Soldier* spawnSoldier(int unitId, cocos2d::Vec2 position, int level = 0);

    // 获取配置
    const UnitConfig* getConfig(int unitId) const;

    // 检查配置是否存在
    bool hasConfig(int unitId) const;

    // 获取所有已加载的兵种ID
    std::vector<int> getAllUnitIds() const;

    // 已训练兵种数量
    const std::map<int, int>& getTrainedUnits() const;
    int getUnitCount(int unitId) const;
    void addTrainedUnit(int unitId, int count = 1);
    bool consumeTrainedUnit(int unitId, int count = 1);
    void resetTrainedUnits();
    void setTrainedUnits(const std::map<int, int>& units);

    // 兵种等级管理
    int getUnitLevel(int unitId) const;
    void setUnitLevel(int unitId, int level);
    const std::map<int, int>& getUnitLevels() const;
    void setUnitLevels(const std::map<int, int>& levels);

    void resetState();

private:
    UnitManager() = default;
    ~UnitManager() = default;

    // 解析单个单位配置
    bool parseUnitConfig(const rapidjson::Value& unitJson, UnitConfig& config);

    // 解析浮点数数组
    std::vector<float> parseFloatArray(const rapidjson::Value& arr);

    // 解析AI类型
    TargetPriority parseAIType(int aiType);

    // ID -> Config 的映射表
    std::map<int, UnitConfig> _configCache;

    // 兵种训练与等级数据（跨场景持久化）
    std::map<int, int> _trainedUnits;
    std::map<int, int> _unitLevels;

    void initInventoryForUnits();

    // 单例实例
    static UnitManager* _instance;
};

#endif // __UNIT_MANAGER_H__
