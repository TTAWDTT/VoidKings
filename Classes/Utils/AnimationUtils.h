// AnimationUtils.h
#pragma once

#include "cocos2d.h"
#include <string>

namespace AnimationUtils {

// 从 SpriteFrameCache 构建 Animation
// 帧命名规范: {baseName}_{animKey}_{frameNumber}.png (从1开始)
// 返回值: autorelease 的 cocos2d::Animation*, 调用方可直接使用或加入 AnimationCache
cocos2d::Animation* buildAnimationFromFrames(const std::string& baseName,
                                             const std::string& animKey,
                                             int frameCount,
                                             float delay,
                                             bool useCache = true);

} // namespace AnimationUtils
