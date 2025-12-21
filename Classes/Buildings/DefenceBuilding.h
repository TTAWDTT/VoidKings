#ifndef __DEFENCE_BUILDING_H__
#define __DEFENCE_BUILDING_H__

#include "cocos2d.h"
#include "DefenseBuildingData.h"

class DefenceBuilding : public cocos2d::Node {
public:
    static DefenceBuilding* create(const DefenceBuildingConfig* config, int level = 0);
    virtual bool init(const DefenceBuildingConfig* config, int level = 0);
    virtual void update(float dt) override;

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
    const DefenceBuildingConfig* _config;
    int _level;
    float _currentHP;
    float _lastAttackTime;
    cocos2d::Sprite* _bodySprite;
    cocos2d::Sprite* _healthBar;
    cocos2d::Node* _target;
    std::string _currentActionKey;

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
