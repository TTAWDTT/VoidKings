#include "Utils/AnimationUtils.h"

namespace AnimationUtils {

cocos2d::Animation* buildAnimationFromFrames(const std::string& baseName,
                                             const std::string& animKey,
                                             int frameCount,
                                             float delay,
                                             bool useCache) {
    if (frameCount <= 0) return nullptr;

    std::string cacheKey = baseName + "_" + animKey;
    if (useCache) {
        auto cached = cocos2d::AnimationCache::getInstance()->getAnimation(cacheKey);
        if (cached) return cached;
    }

    cocos2d::Vector<cocos2d::SpriteFrame*> frames;
    for (int i = 1; i <= frameCount; ++i) {
        std::string frameName = baseName + "_" + animKey + "_" + std::to_string(i) + ".png";
        cocos2d::SpriteFrame* frame = cocos2d::SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
        if (!frame) {
            auto texture = cocos2d::Director::getInstance()->getTextureCache()->addImage(frameName);
            if (texture) {
                auto size = texture->getContentSize();
                frame = cocos2d::SpriteFrame::createWithTexture(texture, cocos2d::Rect(0, 0, size.width, size.height));
                if (frame && useCache) {
                    cocos2d::SpriteFrameCache::getInstance()->addSpriteFrame(frame, frameName);
                }
            }
        }
        // 没有缓存帧则跳过
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
