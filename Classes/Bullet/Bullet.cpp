// Bullet.cpp
#include "Bullet.h"

USING_NS_CC;

Bullet* Bullet::create(const std::string& spriteFrame, float damage, float speed) {
    Bullet* pRet = new(std::nothrow) Bullet();
    if (pRet && pRet->init(spriteFrame, damage, speed)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool Bullet::init(const std::string& spriteFrame, float damage, float speed) {
    if (!Node::init()) return false;

    _damage = damage;
    _speed = speed;
    _target = nullptr;
    _rotateToTarget = true;
    _rotationOffsetDegrees = 0.0f;

    // Create sprite
    _sprite = Sprite::create(spriteFrame);
    if (_sprite) {
        this->addChild(_sprite);
    }

    this->scheduleUpdate();
    return true;
}

void Bullet::setTarget(cocos2d::Node* target) {
    if (_target == target) {
        return;
    }
    if (_target) {
        _target->release();
    }
    _target = target;
    if (_target) {
        _target->retain();
    }
}

void Bullet::setRotateToTarget(bool rotate, float rotationOffsetDegrees) {
    _rotateToTarget = rotate;
    _rotationOffsetDegrees = rotationOffsetDegrees;
}

void Bullet::update(float dt) {
    if (!_target || !_target->getParent()) {
        this->removeFromParent();
        return;
    }

    auto* parent = this->getParent();
    auto* targetParent = _target->getParent();
    Vec2 targetWorld = targetParent
        ? targetParent->convertToWorldSpace(_target->getPosition())
        : _target->getPosition();
    Vec2 targetPos = parent ? parent->convertToNodeSpace(targetWorld) : targetWorld;

    Vec2 currentPos = this->getPosition();
    Vec2 diff = targetPos - currentPos;
    float distance = diff.length();

    // Check if reached target
    if (distance < 5.0f) {
        onReachTarget();
        this->removeFromParent();
        return;
    }

    // Move towards target
    Vec2 direction = diff.getNormalized();
    Vec2 newPos = currentPos + direction * _speed * dt;
    this->setPosition(newPos);

    // Rotate bullet to face target
    if (_rotateToTarget && _sprite) {
        float angle = CC_RADIANS_TO_DEGREES(atan2(diff.y, diff.x));
        _sprite->setRotation(-angle + _rotationOffsetDegrees);
    }
}

void Bullet::onExit() {
    if (_target) {
        _target->release();
        _target = nullptr;
    }
    Node::onExit();
}

void Bullet::onReachTarget() {
    // 命中反馈：轻微冲击环
    auto* parent = this->getParent();
    if (!parent) {
        return;
    }

    auto ring = DrawNode::create();
    ring->drawCircle(Vec2::ZERO, 10.0f, 0.0f, 16, false, Color4F(1.0f, 1.0f, 1.0f, 0.8f));
    ring->setPosition(this->getPosition());
    parent->addChild(ring, this->getLocalZOrder());

    auto scale = ScaleTo::create(0.22f, 1.6f);
    auto fade = FadeTo::create(0.22f, 0);
    ring->runAction(Sequence::create(Spawn::create(scale, fade, nullptr), RemoveSelf::create(), nullptr));
}
