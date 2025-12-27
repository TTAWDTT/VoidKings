#ifndef __GAME_SETTINGS_H__
#define __GAME_SETTINGS_H__

#include "base/CCUserDefault.h"
#include "cocos2d.h"

namespace GameSettings {
constexpr const char* kShowFpsKey = "ui_show_fps";
constexpr const char* kShowGridKey = "ui_show_grid";
constexpr const char* kBattleSpeedKey = "battle_speed";

inline bool getShowFps() {
    return cocos2d::UserDefault::getInstance()->getBoolForKey(kShowFpsKey, true);
}

inline void setShowFps(bool value) {
    auto* defaults = cocos2d::UserDefault::getInstance();
    defaults->setBoolForKey(kShowFpsKey, value);
    defaults->flush();
    cocos2d::Director::getInstance()->setDisplayStats(value);
}

inline void applyShowFps() {
    cocos2d::Director::getInstance()->setDisplayStats(getShowFps());
}

inline bool getShowGrid() {
    return cocos2d::UserDefault::getInstance()->getBoolForKey(kShowGridKey, true);
}

inline void setShowGrid(bool value) {
    auto* defaults = cocos2d::UserDefault::getInstance();
    defaults->setBoolForKey(kShowGridKey, value);
    defaults->flush();
}

inline float clampBattleSpeed(float value) {
    if (value < 0.5f) {
        return 0.5f;
    }
    if (value > 2.0f) {
        return 2.0f;
    }
    return value;
}

inline float getBattleSpeed() {
    float value = cocos2d::UserDefault::getInstance()->getFloatForKey(kBattleSpeedKey, 1.0f);
    return clampBattleSpeed(value);
}

inline void setBattleSpeed(float value) {
    float clamped = clampBattleSpeed(value);
    auto* defaults = cocos2d::UserDefault::getInstance();
    defaults->setFloatForKey(kBattleSpeedKey, clamped);
    defaults->flush();
}

inline void applyTimeScale(float value) {
    auto* scheduler = cocos2d::Director::getInstance()->getScheduler();
    if (scheduler) {
        scheduler->setTimeScale(value);
    }
}

inline void applyBattleSpeed(bool inBattle) {
    applyTimeScale(inBattle ? getBattleSpeed() : 1.0f);
}
} // namespace GameSettings

#endif // __GAME_SETTINGS_H__
