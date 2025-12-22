#ifndef __DEFENCE_BUILDING_H__
#define __DEFENCE_BUILDING_H__

#include "cocos2d.h"
#include "DefenseBuildingData.h"
#include <vector>

class Soldier;

class DefenceBuilding : public cocos2d::Node {
public:
    static DefenceBuilding* create(const DefenceBuildingConfig* config, int level = 0);
    virtual bool init(const DefenceBuildingConfig* config, int level = 0);
    virtual void update(float dt) override;
    virtual void onExit() override;

    // 设置敌方士兵列表（由战斗场景提供）
    static void setEnemySoldiers(const std::vector<Soldier*>* soldiers);

    void takeDamage(float damage);

    int getLevel() const { return _level; }
    void setLevel(int level);

    float getCurrentMaxHP() const;
    float getCurrentDP() const;
    float getCurrentATK_SPEED() const;
    float getCurrentATK() const;
    float getCurrentATK_RANGE() const;

    int getLength() const;
    int getWidth() const;

private:
    static const std::vector<Soldier*>* s_enemySoldiers;

    const DefenceBuildingConfig* _config;
    int _level;
    float _currentHP;
    float _lastAttackTime;
    cocos2d::Sprite* _bodySprite;
    cocos2d::Sprite* _healthBar;
    cocos2d::Node* _target;
    std::string _currentActionKey;

    // 判断目标是否可被当前建筑攻击（空/地判定、存活判定）
    bool canTargetSoldier(const Soldier* soldier) const;
    // 获取可用的敌方单位列表（优先使用外部注入，必要时临时扫描）
    const std::vector<Soldier*>* getEnemySoldiers(std::vector<Soldier*>& fallback) const;
    // 近战/范围伤害的统一处理
    void applyAoeDamage(const cocos2d::Vec2& center, float range, float damage) const;

    void setTarget(cocos2d::Node* target);
    void findTarget();
    void attackTarget();
    void updateHealthBar(bool animate = true);
    void playAnimation(const std::string& animType, int frameCount, float delay, bool loop);
    void stopCurrentAnimation();

    /**
     * @brief 尝试播放待机动画
     * 对于有帧动画的建筑（如Tree），自动加载并播放循环动画
     */
    void tryPlayIdleAnimation();
};

#endif // __DEFENCE_BUILDING_H__
