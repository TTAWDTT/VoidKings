#include "Core.h"
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
