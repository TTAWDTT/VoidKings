/**
 * @file EffectUtils.cpp
 * @brief 轻量级视觉效果工具实现
 */

#include "Utils/EffectUtils.h"

namespace EffectUtils {
namespace {
constexpr int kHitFlashTag = 22001;
}

void playHitFlash(cocos2d::Sprite* sprite) {
    if (!sprite) {
        return;
    }

    sprite->stopActionByTag(kHitFlashTag);

    auto flash = cocos2d::TintTo::create(0.05f, 255, 230, 200);
    auto reset = cocos2d::TintTo::create(0.08f, 255, 255, 255);
    auto sequence = cocos2d::Sequence::create(flash, reset, nullptr);
    sequence->setTag(kHitFlashTag);
    sprite->runAction(sequence);
}
} // namespace EffectUtils
