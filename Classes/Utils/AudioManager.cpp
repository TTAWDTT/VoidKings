#include "Utils/AudioManager.h"
#include "audio/include/SimpleAudioEngine.h"
#include "cocos2d.h"
#include <array>
#include <string>

using namespace CocosDenshion;

namespace {
// 音量统一在此处调节
constexpr float kBgmVolume = 0.55f;
constexpr float kSfxVolume = 0.9f;

// 音频资源映射
#if defined(__cpp_char8_t)
#define VK_UTF8_LITERAL(text) reinterpret_cast<const char*>(u8##text)
#else
#define VK_UTF8_LITERAL(text) u8##text
#endif

constexpr const char* kBgmStart = "music/StartBackGround.wav";
constexpr const char* kBgmBattle1 = "music/BattleBackGround1.wav";
constexpr const char* kBgmBattle2 = "music/BattleBackGround2.wav";
constexpr const char* kBgmBattle3 = "music/BattleBackGround3.wav";

constexpr const char* kSfxButtonClick = "music/ButtonClick.wav";
constexpr const char* kSfxButtonCancel = "music/ButtonCross.wav";

const char* kSfxArrowShoot = VK_UTF8_LITERAL("music/\u5F13\u7BAD\u5C04\u51FA.wav");
const char* kSfxArrowHit = VK_UTF8_LITERAL("music/\u5F13\u7BAD\u547D\u4E2D.wav");
const char* kSfxMeleeHit = VK_UTF8_LITERAL("music/\u8FD1\u6218\u51FB\u6253.wav");
constexpr const char* kSfxMagicAttack = "music/MagAttack.wav";
constexpr const char* kSfxBoom = "music/boom.wav";
constexpr const char* kSfxBuildingCollapse = "music/building collapse.wav";
constexpr const char* kSfxHit1 = "music/hit1.wav";
constexpr const char* kSfxHit2 = "music/hit2.wav";
constexpr const char* kSfxHit3 = "music/hit3.wav";
constexpr const char* kSfxVictory = "music/victory.wav";
constexpr const char* kSfxLose = "music/Lose.wav";

#undef VK_UTF8_LITERAL

std::string s_currentBgm;

SimpleAudioEngine* getEngine() {
    return SimpleAudioEngine::getInstance();
}

void playBgmInternal(const char* file) {
    if (!file) {
        return;
    }
    if (s_currentBgm == file) {
        return;
    }
    auto* engine = getEngine();
    if (!engine) {
        return;
    }
    engine->setBackgroundMusicVolume(kBgmVolume);
    engine->playBackgroundMusic(file, true);
    s_currentBgm = file;
}

void playEffectInternal(const char* file) {
    if (!file) {
        return;
    }
    auto* engine = getEngine();
    if (!engine) {
        return;
    }
    engine->setEffectsVolume(kSfxVolume);
    engine->playEffect(file, false);
}
} // namespace

namespace AudioManager {
void preload() {
    // 只预加载短音效，避免占用过多内存
    auto* engine = getEngine();
    if (!engine) {
        return;
    }
    engine->setBackgroundMusicVolume(kBgmVolume);
    engine->setEffectsVolume(kSfxVolume);

    engine->preloadBackgroundMusic(kBgmStart);
    engine->preloadBackgroundMusic(kBgmBattle1);
    engine->preloadBackgroundMusic(kBgmBattle2);
    engine->preloadBackgroundMusic(kBgmBattle3);

    std::array<const char*, 13> effects = {
        kSfxButtonClick,
        kSfxButtonCancel,
        kSfxArrowShoot,
        kSfxArrowHit,
        kSfxMeleeHit,
        kSfxMagicAttack,
        kSfxBoom,
        kSfxBuildingCollapse,
        kSfxHit1,
        kSfxHit2,
        kSfxHit3,
        kSfxVictory,
        kSfxLose
    };
    for (const auto& effect : effects) {
        engine->preloadEffect(effect);
    }
}

void playMainBgm() {
    playBgmInternal(kBgmStart);
}

void playBattleBgm(int levelId) {
    int index = (levelId - 1) % 3;
    if (index < 0) {
        index = 0;
    }
    switch (index) {
    case 0:
        playBgmInternal(kBgmBattle1);
        break;
    case 1:
        playBgmInternal(kBgmBattle2);
        break;
    default:
        playBgmInternal(kBgmBattle3);
        break;
    }
}

void stopBgm() {
    auto* engine = getEngine();
    if (engine) {
        engine->stopBackgroundMusic(false);
    }
    s_currentBgm.clear();
}

void pauseAll() {
    auto* engine = getEngine();
    if (!engine) {
        return;
    }
    engine->pauseBackgroundMusic();
    engine->pauseAllEffects();
}

void resumeAll() {
    auto* engine = getEngine();
    if (!engine) {
        return;
    }
    engine->resumeBackgroundMusic();
    engine->resumeAllEffects();
}

void playButtonClick() {
    playEffectInternal(kSfxButtonClick);
}

void playButtonCancel() {
    playEffectInternal(kSfxButtonCancel);
}

void playArrowShoot() {
    playEffectInternal(kSfxArrowShoot);
}

void playArrowHit() {
    playEffectInternal(kSfxArrowHit);
}

void playMeleeHit() {
    playEffectInternal(kSfxMeleeHit);
}

void playMagicAttack() {
    playEffectInternal(kSfxMagicAttack);
}

void playBoom() {
    playEffectInternal(kSfxBoom);
}

void playBuildingCollapse() {
    playEffectInternal(kSfxBuildingCollapse);
}

void playRandomHit() {
    static std::array<const char*, 3> hits = { kSfxHit1, kSfxHit2, kSfxHit3 };
    int index = cocos2d::RandomHelper::random_int(0, static_cast<int>(hits.size() - 1));
    playEffectInternal(hits[static_cast<size_t>(index)]);
}

void playVictory() {
    playEffectInternal(kSfxVictory);
}

void playLose() {
    playEffectInternal(kSfxLose);
}
} // namespace AudioManager
