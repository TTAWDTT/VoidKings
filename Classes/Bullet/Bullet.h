// Bullet.h
#ifndef __BULLET_H__
#define __BULLET_H__

#include "cocos2d.h"

class Bullet : public cocos2d::Node {
public:
    static Bullet* create(const std::string& spriteFrame, float damage, float speed);
    virtual bool init(const std::string& spriteFrame, float damage, float speed);
    virtual void update(float dt) override;
    void onExit() override;

    void setTarget(cocos2d::Node* target);
    void setRotateToTarget(bool rotate, float rotationOffsetDegrees = 0.0f);
    float getDamage() const { return _damage; }
    
protected:
    cocos2d::Sprite* _sprite;
    cocos2d::Node* _target;
    float _damage;
    float _speed;
    bool _rotateToTarget = true;
    float _rotationOffsetDegrees = 0.0f;
    
    virtual void onReachTarget();
};

#endif // __BULLET_H__
