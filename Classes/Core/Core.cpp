#include "Core.h"
#include <algorithm>
#include <cstdio>

USING_NS_CC;

Core* Core::s_instance = nullptr;

Core* Core::getInstance()
{
    if (!s_instance)
    {
        s_instance = new (std::nothrow) Core();
        if (s_instance && s_instance->init())
        {
            s_instance->retain();
        }
        else
        {
            CC_SAFE_DELETE(s_instance);
        }
    }
    return s_instance;
}

void Core::destroyInstance()
{
    CC_SAFE_RELEASE_NULL(s_instance);
}

Core::Core()
{
}

Core::~Core()
{
}

bool Core::init()
{
    _resourceMap.clear();
    _resourceMap[ResourceType::COIN] = 1000;
    _resourceMap[ResourceType::DIAMOND] = 50;
    _levelStars.clear();
    _totalEarnedGold = 0;
    _totalEarnedDiamond = 0;
    return true;
}

int Core::getResource(ResourceType type) const
{
    auto it = _resourceMap.find(type);
    return it == _resourceMap.end() ? 0 : it->second;
}

void Core::setResource(ResourceType type, int amount)
{
    if (amount < 0) amount = 0;
    _resourceMap[type] = amount;
}

void Core::addResource(ResourceType type, int delta)
{
    int current = getResource(type);
    setResource(type, current + delta);

    // 只记录正向获得的历史总量
    if (delta > 0) {
        if (type == ResourceType::COIN) {
            _totalEarnedGold += delta;
        }
        else if (type == ResourceType::DIAMOND) {
            _totalEarnedDiamond += delta;
        }
    }
}

bool Core::consumeResource(ResourceType type, int amount)
{
    if (amount <= 0) return false;
    int current = getResource(type);
    if (current < amount)
    {
        return false;
    }
    setResource(type, current - amount);
    return true;
}

Sprite* Core::createResourceSprite(ResourceType type)
{
    std::string frameName = getDefaultFrameName(type);
    auto sprite = Sprite::create(frameName);
    if (sprite)
    {
        playResourceAnimation(sprite, type);
    }
    return sprite;
}

void Core::playResourceAnimation(Sprite* sprite, ResourceType type, float delay, bool loop)
{
    if (!sprite) return;

    auto anim = buildResourceAnimation(type, delay);
    if (!anim) return;

    auto animate = Animate::create(anim);
    sprite->stopAllActions();
    if (loop)
    {
        sprite->runAction(RepeatForever::create(animate));
    }
    else
    {
        sprite->runAction(animate);
    }
}

std::string Core::getDefaultFrameName(ResourceType type) const
{
    switch (type)
    {
        case ResourceType::COIN:
            return "source/coin/coin_0001.png";
        case ResourceType::DIAMOND:
        default:
            return "source/diamond/sprite_0000.png";
    }
}

Animation* Core::buildResourceAnimation(ResourceType type, float delay) const
{
    Vector<SpriteFrame*> frames;
    if (type == ResourceType::COIN)
    {
        for (int i = 1; i <= 5; ++i)
        {
            char buffer[32];
            sprintf(buffer, "source/coin/coin_%04d.png", i);
            auto texture = Director::getInstance()->getTextureCache()->addImage(buffer);
            if (texture) {
                Size size = texture->getContentSize();
                auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, size.width, size.height));
                if (frame) frames.pushBack(frame);
            }
        }
        if (delay <= 0.0f)
        {
            delay = 0.1f;
        }
    }
    else
    {
        for (int i = 0; i <= 3; ++i)
        {
            char buffer[32];
            sprintf(buffer, "source/diamond/sprite_%04d.png", i);
            auto texture = Director::getInstance()->getTextureCache()->addImage(buffer);
            if (texture) {
                Size size = texture->getContentSize();
                auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, size.width, size.height));
                if (frame) frames.pushBack(frame);
            }
        }
        if (delay <= 0.0f)
        {
            delay = 0.15f;
        }
    }

    if (frames.empty())
    {
        return nullptr;
    }

    return Animation::createWithSpriteFrames(frames, delay);
}

long long Core::getTotalEarned(ResourceType type) const
{
    if (type == ResourceType::COIN) {
        return _totalEarnedGold;
    }
    if (type == ResourceType::DIAMOND) {
        return _totalEarnedDiamond;
    }
    return 0;
}

int Core::getBaseLevel() const
{
    // 基地等级阈值（金币/钻石同时满足）
    static const int kGoldThresholds[] = { 0, 1000, 3000, 8000, 15000, 30000 };
    static const int kDiamondThresholds[] = { 0, 50, 150, 350, 700, 1200 };
    const int goldSize = static_cast<int>(sizeof(kGoldThresholds) / sizeof(kGoldThresholds[0]));
    const int diamondSize = static_cast<int>(sizeof(kDiamondThresholds) / sizeof(kDiamondThresholds[0]));

    int goldLevel = 0;
    for (int i = 0; i < goldSize; ++i) {
        if (_totalEarnedGold >= kGoldThresholds[i]) {
            goldLevel = i;
        }
    }

    int diamondLevel = 0;
    for (int i = 0; i < diamondSize; ++i) {
        if (_totalEarnedDiamond >= kDiamondThresholds[i]) {
            diamondLevel = i;
        }
    }

    return std::min(goldLevel, diamondLevel);
}

float Core::getBaseProduceSpeedMultiplier() const
{
    int level = getBaseLevel();
    // 每级提升产出速度
    return 1.0f + static_cast<float>(level) * 0.15f;
}

int Core::getLevelStars(int levelId) const
{
    auto it = _levelStars.find(levelId);
    return it == _levelStars.end() ? 0 : it->second;
}

void Core::setLevelStars(int levelId, int stars)
{
    if (levelId <= 0) {
        return;
    }
    if (stars < 0) {
        stars = 0;
    }
    if (stars > 3) {
        stars = 3;
    }

    auto it = _levelStars.find(levelId);
    if (it == _levelStars.end()) {
        _levelStars[levelId] = stars;
        return;
    }
    if (stars > it->second) {
        it->second = stars;
    }
}

bool Core::isLevelCompleted(int levelId) const
{
    return getLevelStars(levelId) > 0;
}
