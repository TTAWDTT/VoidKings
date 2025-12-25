#ifndef __NODE_UTILS_H__
#define __NODE_UTILS_H__

#include "cocos2d.h"
#include <cmath>

namespace NodeUtils {

inline const cocos2d::Sprite* findBodySprite(const cocos2d::Node* building) {
    if (!building) {
        return nullptr;
    }
    if (auto* named = dynamic_cast<const cocos2d::Sprite*>(building->getChildByName("bodySprite"))) {
        return named;
    }
    const cocos2d::Sprite* largest = nullptr;
    float largestArea = 0.0f;
    for (auto* child : building->getChildren()) {
        auto* sprite = dynamic_cast<const cocos2d::Sprite*>(child);
        if (!sprite) {
            continue;
        }
        if (sprite->getName() == "healthBar") {
            continue;
        }
        cocos2d::Size size = sprite->getContentSize();
        float area = size.width * size.height;
        if (!largest || area > largestArea) {
            largest = sprite;
            largestArea = area;
        }
    }
    return largest;
}

inline cocos2d::Sprite* findBodySprite(cocos2d::Node* building) {
    return const_cast<cocos2d::Sprite*>(findBodySprite(static_cast<const cocos2d::Node*>(building)));
}

inline bool hitTestBuilding(cocos2d::Node* building, const cocos2d::Vec2& worldPos) {
    if (!building) {
        return false;
    }
    const cocos2d::Sprite* bodySprite = findBodySprite(building);
    if (bodySprite) {
        cocos2d::Vec2 localPos = building->convertToNodeSpace(worldPos);
        return bodySprite->getBoundingBox().containsPoint(localPos);
    }
    auto* parent = building->getParent();
    if (parent) {
        cocos2d::Vec2 localPos = parent->convertToNodeSpace(worldPos);
        return building->getBoundingBox().containsPoint(localPos);
    }
    return false;
}

inline void drawDashedCircle(cocos2d::DrawNode* node,
                             const cocos2d::Vec2& center,
                             float radius,
                             const cocos2d::Color4F& color) {
    if (!node || radius <= 0.0f) {
        return;
    }
    node->clear();
    const int segments = 48;
    constexpr float kPi = 3.1415926f;
    const float step = 2.0f * kPi / segments;
    for (int i = 0; i < segments; ++i) {
        if (i % 2 != 0) {
            continue;
        }
        float angle1 = step * i;
        float angle2 = step * (i + 1);
        cocos2d::Vec2 p1(center.x + radius * std::cos(angle1), center.y + radius * std::sin(angle1));
        cocos2d::Vec2 p2(center.x + radius * std::cos(angle2), center.y + radius * std::sin(angle2));
        node->drawLine(p1, p2, color);
    }
}

} // namespace NodeUtils

#endif // __NODE_UTILS_H__
