// Resource.h
#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include "cocos2d.h"

enum class ResourceType {
    COIN,
    DIAMOND
};

class Resource : public cocos2d::Node {
public:
    static Resource* create(ResourceType type, int amount);
    virtual bool init(ResourceType type, int amount);

    ResourceType getType() const { return _type; }
    int getAmount() const { return _amount; }

private:
    ResourceType _type;
    int _amount;
    cocos2d::Sprite* _sprite;
    
    void playIdleAnimation();
};

#endif // __RESOURCE_H__
