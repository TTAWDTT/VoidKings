// Core.h
#ifndef __CORE_H__
#define __CORE_H__

#include "cocos2d.h"
#include <unordered_map>

enum class ResourceType {
    COIN,
    DIAMOND
};

class Core : public cocos2d::Ref {
public:
    static Core* getInstance();
    static void destroyInstance();

    int getResource(ResourceType type) const;
    void setResource(ResourceType type, int amount);
    void addResource(ResourceType type, int delta);
    bool consumeResource(ResourceType type, int amount);

    cocos2d::Sprite* createResourceSprite(ResourceType type);
    void playResourceAnimation(cocos2d::Sprite* sprite, ResourceType type, float delay = 0.1f, bool loop = true);

    // 资源历史累计
    long long getTotalEarned(ResourceType type) const;

    // 基地等级（由历史资源总量决定）
    int getBaseLevel() const;
    float getBaseProduceSpeedMultiplier() const;

    // 关卡星级记录
    int getLevelStars(int levelId) const;
    void setLevelStars(int levelId, int stars);
    bool isLevelCompleted(int levelId) const;

protected:
    Core();
    virtual ~Core();

    bool init();

private:
    struct ResourceTypeHash {
        std::size_t operator()(ResourceType type) const {
            return static_cast<std::size_t>(type);
        }
    };

    std::string getDefaultFrameName(ResourceType type) const;
    cocos2d::Animation* buildResourceAnimation(ResourceType type, float delay) const;

    static Core* s_instance;
    std::unordered_map<ResourceType, int, ResourceTypeHash> _resourceMap;
    long long _totalEarnedGold = 0;
    long long _totalEarnedDiamond = 0;
    std::unordered_map<int, int> _levelStars;
};

#endif // __CORE_H__
