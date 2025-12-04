// UnitManager.h
class UnitManager {
public:
    static UnitManager* getInstance();

    // 加载配置 (比如在游戏启动时调用)
    void loadConfig(const std::string& jsonFile);

    // 造兵工厂方法
    Soldier* spawnSoldier(int unitId, cocos2d::Vec2 position);

private:
    // ID -> Config 的映射表
    std::map<int, UnitConfig> _configCache;
};
