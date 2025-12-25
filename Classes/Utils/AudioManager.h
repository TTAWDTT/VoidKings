#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

// 音频管理：统一控制BGM与音效
namespace AudioManager {
    void preload();

    void playMainBgm();
    void playBattleBgm(int levelId);
    void stopBgm();
    void pauseAll();
    void resumeAll();

    void playButtonClick();
    void playButtonCancel();
    void playArrowShoot();
    void playArrowHit();
    void playMeleeHit();
    void playMagicAttack();
    void playMagicHit();
    void playBoom();
    void playSpikeAppear();
    void playSnapTrap();
    void playFireSpray();
    void playBuildingCollapse();
    void playRandomHit();
    void playVictory();
    void playLose();
}

#endif // __AUDIO_MANAGER_H__
