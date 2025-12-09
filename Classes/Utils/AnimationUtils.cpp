#include "Utils/AnimationUtils.h"

namespace AnimationUtils {

cocos2d::Animation* buildAnimationFromFrames(const std::string& baseName,
                                             const std::string& animKey,
                                             int frameCount,
                                             float delay,
                                             bool useCache) {
    std::string cacheKey = baseName + "_" + animKey;
    if (useCache) {
        auto cached = cocos2d::AnimationCache::getInstance()->getAnimation(cacheKey);
        if (cached) return cached;
    }

    cocos2d::Vector<cocos2d::SpriteFrame*> frames;
    for (int i = 1; i <= frameCount; ++i) {
        std::string frameName = baseName + "_" + animKey + "_" + std::to_string(i) + ".png";
        auto frame = cocos2d::SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
        if (frame) frames.pushBack(frame);
    }
    if (frames.empty()) return nullptr;

    auto anim = cocos2d::Animation::createWithSpriteFrames(frames, delay);
    if (useCache) {
        cocos2d::AnimationCache::getInstance()->addAnimation(anim, cacheKey);
    }
    return anim;
}

} // namespace AnimationUtils
