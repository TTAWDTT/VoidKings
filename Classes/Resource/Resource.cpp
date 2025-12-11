// Resource.cpp
#include "Resource.h"
#include "Utils/AnimationUtils.h"

USING_NS_CC;

Resource* Resource::create(ResourceType type, int amount) {
    Resource* pRet = new(std::nothrow) Resource();
    if (pRet && pRet->init(type, amount)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool Resource::init(ResourceType type, int amount) {
    if (!Node::init()) return false;

    _type = type;
    _amount = amount;

    // Create initial sprite based on type
    std::string frameName;
    if (type == ResourceType::COIN) {
        frameName = "coin_0001.png";
    } else {
        frameName = "sprite_0000.png"; // diamond
    }

    _sprite = Sprite::createWithSpriteFrameName(frameName);
    if (_sprite) {
        this->addChild(_sprite);
        playIdleAnimation();
    }

    return true;
}

void Resource::playIdleAnimation() {
    if (!_sprite) return;

    Animation* anim = nullptr;
    if (_type == ResourceType::COIN) {
        // Create coin animation from coin_0001.png to coin_0005.png
        Vector<SpriteFrame*> frames;
        for (int i = 1; i <= 5; ++i) {
            char buffer[32];
            sprintf(buffer, "coin_%04d.png", i);
            auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(buffer);
            if (frame) frames.pushBack(frame);
        }
        if (!frames.empty()) {
            anim = Animation::createWithSpriteFrames(frames, 0.1f);
        }
    } else {
        // Create diamond animation from sprite_0000.png to sprite_0003.png
        Vector<SpriteFrame*> frames;
        for (int i = 0; i <= 3; ++i) {
            char buffer[32];
            sprintf(buffer, "sprite_%04d.png", i);
            auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(buffer);
            if (frame) frames.pushBack(frame);
        }
        if (!frames.empty()) {
            anim = Animation::createWithSpriteFrames(frames, 0.15f);
        }
    }

    if (anim) {
        auto animate = Animate::create(anim);
        auto repeat = RepeatForever::create(animate);
        _sprite->runAction(repeat);
    }
}
