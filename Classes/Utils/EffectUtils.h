/**
 * @file EffectUtils.h
 * @brief 轻量级视觉效果工具
 */

#ifndef __EFFECT_UTILS_H__
#define __EFFECT_UTILS_H__

#include "cocos2d.h"

namespace EffectUtils {
    /**
     * @brief 受击闪烁效果
     * @param sprite 目标精灵
     */
    void playHitFlash(cocos2d::Sprite* sprite);
}

#endif // __EFFECT_UTILS_H__
