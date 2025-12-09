// Soldier.cpp
#include "Soldier.h"
#include <string>

// 从config中创建
Soldier* Soldier::create(const UnitConfig* config, int level) {
    Soldier* pRet = new(std::nothrow) Soldier();
    if (pRet && pRet->init(config, level)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool Soldier::init(const UnitConfig* config, int level) {
   if (!Node::init()) return false;

   // 1. 保存配置
   _config = config;

   // 2. 设置等级并验证
   _level = level;
   if (_level < 0) _level = 0;
   if (_level > _config->MAXLEVEL) _level = _config->MAXLEVEL;
    
   // 3. 初始化运行时状态
   _currentHP = getCurrentMaxHP();
   _direction = Direction::RIGHT;  // 默认朝右
   _currentActionKey.clear();

   // 4. 创建精灵
   std::string initialFrame = _config->spriteFrameName;
   _bodySprite = cocos2d::Sprite::createWithSpriteFrameName(initialFrame);
    
    _healthBar = cocos2d::Sprite::createWithSpriteFrameName("health_bar.png");
    this->addChild(_bodySprite);
    this->addChild(_healthBar);

    // 初始化血条
    if (_healthBar) {
        _healthBar->setAnchorPoint(cocos2d::Vec2(0.0f, 0.5f));
        float offsetY = 10.0f;
        if (_bodySprite) {
            float y = _bodySprite->getContentSize().height * 0.5f + offsetY;
            float x = -_healthBar->getContentSize().width * 0.5f;
            _healthBar->setPosition(cocos2d::Vec2(x, y));
        }
        else {
            _healthBar->setPosition(cocos2d::Vec2(0, 30.0f));
        }
        _healthBar->setScaleX(1.0f);
        _healthBar->setColor(cocos2d::Color3B::GREEN);
    }

    updateHealthBar(false);

    // 5. 启动 Update 循环
    this->scheduleUpdate();

    return true;
}

void Soldier::setLevel(int level) {
    if (level < 0) level = 0;
    if (level > _config->MAXLEVEL) level = _config->MAXLEVEL;
    _level = level;
}

float Soldier::getCurrentMaxHP() const {
    if (_level >= 0 && _level < _config->HP.size()) {
        return _config->HP[_level];
    }
    return _config->HP.empty() ? 0.0f : _config->HP[0];
}

float Soldier::getCurrentSpeed() const {
    if (_level >= 0 && _level < _config->SPEED.size()) {
        return _config->SPEED[_level];
    }
    return _config->SPEED.empty() ? 0.0f : _config->SPEED[0];
}

float Soldier::getCurrentATK() const {
    if (_level >= 0 && _level < _config->ATK.size()) {
        return _config->ATK[_level];
    }
    return _config->ATK.empty() ? 0.0f : _config->ATK[0];
}

float Soldier::getCurrentRange() const {
    if (_level >= 0 && _level < _config->RANGE.size()) {
        return _config->RANGE[_level];
    }
    return _config->RANGE.empty() ? 0.0f : _config->RANGE[0];
}

float Soldier::getCurrentHP() const {
    return _currentHP;
}

void Soldier::update(float dt) {
    if (!_target) {
        findTarget();
    }
    else {
        float dist = this->getPosition().distance(_target->getPosition());
        if (dist <= getCurrentRange()) {
            attackTarget();
        }
        else {
            moveToTarget(dt);
        }
    }
}

void Soldier::findTarget() {
    // 伪代码 - 需要GameWorld实现
    // auto buildings = GameWorld::getInstance()->getAllBuildings();
    // switch (_config->aiType) { ... }
}

void Soldier::moveToTarget(float dt) {
    if (!_target) return;

    cocos2d::Vec2 diff = _target->getPosition() - this->getPosition();
    
    // 计算方向(只有左右)
    Direction newDir = calcDirection(this->getPosition(), _target->getPosition());
    
    // 更新精灵朝向并播放移动动画
    updateSpriteDirection(newDir);
    playAnimation(_config->anim_walk, _config->anim_walk_frames, _config->anim_walk_delay, true);

    // 使用setPosition实现移动 - 每帧更新位置,动画同时播放
    cocos2d::Vec2 direction = diff.getNormalized();
    cocos2d::Vec2 newPos = this->getPosition() + (direction * getCurrentSpeed() * dt);
    this->setPosition(newPos);
}

void Soldier::takeDamage(float damage) {
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;

    updateHealthBar(true);

    if (_currentHP <= 0) {
        // 播放死亡动画(使用当前方向)
        this->stopAllActions();
        playAnimation(_config->anim_dead, _config->anim_dead_frames, _config->anim_dead_delay, false);
    }
}

void Soldier::attackTarget() {
    if (!_target) return;
    
    float attackPower = getCurrentATK();
    
    // 计算攻击方向
    Direction attackDir = calcDirection(this->getPosition(), _target->getPosition());
    updateSpriteDirection(attackDir);
    
    // 播放攻击动画
    playAnimation(_config->anim_attack, _config->anim_attack_frames, _config->anim_attack_delay, false);
}

void Soldier::updateHealthBar(bool animate) {
    if (!_healthBar || !_config) return;

    float maxHP = getCurrentMaxHP();
    float pct = 0.0f;
    if (maxHP > 0.00001f) {
        pct = _currentHP / maxHP;
    }
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;

    float targetScaleX = pct;

    if (animate) {
        _healthBar->stopAllActions();
        auto action = cocos2d::ScaleTo::create(0.12f, targetScaleX, 1.0f);
        _healthBar->runAction(action);
    }
    else {
        _healthBar->setScaleX(targetScaleX);
    }

    if (pct > 0.5f) {
        _healthBar->setColor(cocos2d::Color3B::GREEN);
    }
    else if (pct > 0.2f) {
        _healthBar->setColor(cocos2d::Color3B::YELLOW);
    }
    else {
        _healthBar->setColor(cocos2d::Color3B::RED);
    }

    if (pct <= 0.0f) {
        _healthBar->setVisible(false);
    }
    else {
        _healthBar->setVisible(true);
    }
}

void Soldier::stopCurrentAnimation() {
    if (!_bodySprite) return;
    _bodySprite->stopAllActions();
    _currentActionKey.clear();
}

// buildAnimationFromFrames 已迁移到公共工具
#include "Utils/AnimationUtils.h"

// 计算方向: 简化为左右两个方向
// 向正上/正下时保持当前方向不变
Direction Soldier::calcDirection(const cocos2d::Vec2& from, const cocos2d::Vec2& to) {
    cocos2d::Vec2 diff = to - from;
    
    // 如果水平分量很小(接近垂直移动),保持当前方向
    if (std::abs(diff.x) < 0.01f) {
        return _direction;
    }
    
    // 根据x分量判断左右
    return (diff.x >= 0) ? Direction::RIGHT : Direction::LEFT;
}

// 更新精灵朝向 - 通过水平翻转实现左向
void Soldier::updateSpriteDirection(Direction dir) {
    if (!_bodySprite) return;
    
    if (dir == _direction) return; // 方向未变,无需更新
    
    _direction = dir;
    
    // 素材默认朝右,左向通过scaleX=-1实现翻转
    if (dir == Direction::LEFT) {
        _bodySprite->setScaleX(-1.0f);
    }
    else {
        _bodySprite->setScaleX(1.0f);
    }
}

// 统一的动画播放函数
void Soldier::playAnimation(const std::string& animType, int frameCount, float delay, bool loop) {
    if (!_bodySprite || !_config) return;
    
    // 生成动画key用于去重
    std::string key = animType;
    if (_currentActionKey == key) return; // 已在播放相同动画
    
    auto anim = AnimationUtils::buildAnimationFromFrames(_config->spriteFrameName, animType, frameCount, delay);
    if (!anim) return;
    
    _bodySprite->stopAllActions();
    
    if (loop) {
        // 循环播放(移动/待机)
        auto act = cocos2d::RepeatForever::create(cocos2d::Animate::create(anim));
        _bodySprite->runAction(act);
    }
    else {
        // 单次播放(攻击/死亡)
        auto sequence = cocos2d::Sequence::create(
            cocos2d::Animate::create(anim),
            cocos2d::CallFunc::create([this]() {
                _currentActionKey.clear();
                // 死亡动画结束后移除
                if (_currentHP <= 0) {
                    this->removeFromParent();
                }
            }),
            nullptr
        );
        _bodySprite->runAction(sequence);
    }
    
    _currentActionKey = key;
}