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
};

#endif // __CORE_H__
