// Soldier.cpp
#include "Soldier.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Buildings/StorageBuilding.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace {
// 根据ID映射资源目录，避免兵种资源找不到
std::string resolveSpriteBaseName(const UnitConfig* config) {
    if (!config) return "";

    switch (config->id) {
    case 101:
        return "unit/MiniSpearMan_output/spearman";
    case 102:
        return "unit/MiniSwordMan_output/swordman";
    case 103:
        return "unit/MiniArcherMan_output/archer";
    default:
        return config->spriteFrameName;
    }
}
} // namespace

const std::vector<cocos2d::Node*>* Soldier::s_enemyBuildings = nullptr;

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

void Soldier::setEnemyBuildings(const std::vector<cocos2d::Node*>* buildings) {
    s_enemyBuildings = buildings;
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
   _lastAttackTime = 0.0f;
   _direction = Direction::RIGHT;  // 默认朝右
   _currentActionKey.clear();
   _target = nullptr;

   // 4. 创建精灵
   _spriteBaseName = resolveSpriteBaseName(_config);
   std::string initialFrame = _spriteBaseName.empty() ? _config->spriteFrameName : _spriteBaseName;

   std::string filePath = initialFrame;
   if (filePath.find(".png") == std::string::npos) {
       filePath += ".png";
   }
   auto cachedFrame = cocos2d::SpriteFrameCache::getInstance()->getSpriteFrameByName(filePath);
   if (cachedFrame) {
       _bodySprite = cocos2d::Sprite::createWithSpriteFrame(cachedFrame);
   }
   else {
       _bodySprite = cocos2d::Sprite::create(filePath);
   }
   if (!_bodySprite) {
       // 创建占位精灵，避免空指针崩溃
       _bodySprite = cocos2d::Sprite::create("grass/grass_0000.png");
       if (_bodySprite) {
           _bodySprite->setScale(0.6f);
           _bodySprite->setColor(cocos2d::Color3B(200, 200, 200));
       }
   }

   _healthBar = cocos2d::Sprite::create("health_bar.png");
   if (!_healthBar) {
       _healthBar = cocos2d::Sprite::create("res/health_bar.png");
   }
   if (!_healthBar) {
       // 兜底：使用现有贴图作为血条底图
       _healthBar = cocos2d::Sprite::create("grass/grass_0000.png");
       if (_healthBar) {
           _healthBar->setTextureRect(cocos2d::Rect(0, 0, 40, 6));
       }
   }

   if (_bodySprite) {
       this->addChild(_bodySprite);
   }
   if (_healthBar) {
       this->addChild(_healthBar);
   }

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
    if (_currentHP <= 0) {
        return;
    }

    if (_target && !_target->getParent()) {
        setTarget(nullptr);
    }

    if (!_target) {
        findTarget();
    }

    if (_target) {
        float dist = this->getPosition().distance(_target->getPosition());
        float attackDistance = getAttackDistance(_target);
        if (dist <= attackDistance) {
            attackTarget();
        }
        else {
            moveToTarget(dt);
        }
    }
}

void Soldier::onExit() {
    setTarget(nullptr);
    Node::onExit();
}

void Soldier::setTarget(cocos2d::Node* target) {
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

void Soldier::findTarget() {
    if (!s_enemyBuildings || s_enemyBuildings->empty()) {
        return;
    }

    // 简化的寻敌：先按AI偏好找，找不到再选最近目标
    auto pickNearest = [&](bool onlyDefense, bool onlyResource) -> cocos2d::Node* {
        cocos2d::Node* nearest = nullptr;
        float nearestDist = 999999.0f;
        const auto selfPos = this->getPosition();

        for (auto* building : *s_enemyBuildings) {
            if (!building || !building->getParent()) {
                continue;
            }

            bool isDefense = dynamic_cast<DefenceBuilding*>(building) != nullptr;
            bool isResource = dynamic_cast<ProductionBuilding*>(building) != nullptr
                || dynamic_cast<StorageBuilding*>(building) != nullptr;

            if (onlyDefense && !isDefense) {
                continue;
            }
            if (onlyResource && !isResource) {
                continue;
            }

            float dist = selfPos.distance(building->getPosition());
            if (dist < nearestDist) {
                nearestDist = dist;
                nearest = building;
            }
        }

        return nearest;
    };

    cocos2d::Node* target = nullptr;
    if (_config) {
        if (_config->aiType == TargetPriority::DEFENSE) {
            target = pickNearest(true, false);
        }
        else if (_config->aiType == TargetPriority::RESOURCE) {
            target = pickNearest(false, true);
        }
    }

    if (!target) {
        target = pickNearest(false, false);
    }

    setTarget(target);
}

void Soldier::moveToTarget(float dt) {
    if (!_target) return;

    cocos2d::Vec2 diff = _target->getPosition() - this->getPosition();
    float dist = diff.length();
    float stopDistance = getAttackDistance(_target);
    if (dist <= stopDistance) {
        return;
    }
    
    // 计算方向(只有左右)
    Direction newDir = calcDirection(this->getPosition(), _target->getPosition());
    
    // 更新精灵朝向并播放移动动画
    updateSpriteDirection(newDir);
    playAnimation(_config->anim_walk, _config->anim_walk_frames, _config->anim_walk_delay, true);

    // 使用setPosition实现移动 - 每帧更新位置,动画同时播放
    if (dist <= 0.0001f) {
        return;
    }

    cocos2d::Vec2 direction = diff.getNormalized();
    float step = getCurrentSpeed() * dt;
    float remaining = dist - stopDistance;
    if (remaining < 0.0f) {
        remaining = 0.0f;
    }
    float move = std::min(step, remaining);
    cocos2d::Vec2 newPos = this->getPosition() + (direction * move);
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
    if (!_target || !_target->getParent()) {
        setTarget(nullptr);
        return;
    }

    float dist = this->getPosition().distance(_target->getPosition());
    float attackDistance = getAttackDistance(_target);
    if (dist > attackDistance) {
        return;
    }

    float currentTime = cocos2d::Director::getInstance()->getTotalFrames() / 60.0f;
    float attackInterval = 0.4f;
    if (_config) {
        attackInterval = _config->anim_attack_delay * _config->anim_attack_frames;
    }
    if (attackInterval < 0.2f) {
        attackInterval = 0.2f;
    }

    // 简单攻击间隔控制
    if (currentTime - _lastAttackTime < attackInterval) {
        return;
    }
    _lastAttackTime = currentTime;

    // 计算攻击方向
    Direction attackDir = calcDirection(this->getPosition(), _target->getPosition());
    updateSpriteDirection(attackDir);

    // 播放攻击动画
    playAnimation(_config->anim_attack, _config->anim_attack_frames, _config->anim_attack_delay, false);

    float attackPower = getCurrentATK();
    if (auto defence = dynamic_cast<DefenceBuilding*>(_target)) {
        defence->takeDamage(attackPower);
    }
    else if (auto production = dynamic_cast<ProductionBuilding*>(_target)) {
        production->takeDamage(attackPower);
    }
    else if (auto storage = dynamic_cast<StorageBuilding*>(_target)) {
        storage->takeDamage(attackPower);
    }
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

float Soldier::getBodyRadius() const {
    if (!_bodySprite) {
        return 0.0f;
    }

    cocos2d::Size size = _bodySprite->getContentSize();
    float scaleX = std::abs(_bodySprite->getScaleX());
    float scaleY = std::abs(_bodySprite->getScaleY());
    return 0.5f * std::max(size.width * scaleX, size.height * scaleY);
}

float Soldier::getTargetRadius(const cocos2d::Node* target) const {
    if (!target) {
        return 0.0f;
    }

    float maxRadius = 0.0f;
    for (const auto& child : target->getChildren()) {
        auto sprite = dynamic_cast<cocos2d::Sprite*>(child);
        if (sprite) {
            cocos2d::Size size = sprite->getContentSize();
            float scaleX = std::abs(sprite->getScaleX());
            float scaleY = std::abs(sprite->getScaleY());
            float radius = 0.5f * std::max(size.width * scaleX, size.height * scaleY);
            if (radius > maxRadius) {
                maxRadius = radius;
            }
        }
    }
    if (maxRadius > 0.0f) {
        return maxRadius;
    }

    cocos2d::Size size = target->getContentSize();
    if (size.width > 0.0f && size.height > 0.0f) {
        float scaleX = std::abs(target->getScaleX());
        float scaleY = std::abs(target->getScaleY());
        return 0.5f * std::max(size.width * scaleX, size.height * scaleY);
    }

    return 0.0f;
}

float Soldier::getAttackDistance(const cocos2d::Node* target) const {
    float range = getCurrentRange();
    float targetRadius = getTargetRadius(target);
    float selfRadius = getBodyRadius();
    return range + targetRadius + selfRadius;
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
    
    std::string baseName = _spriteBaseName.empty() ? _config->spriteFrameName : _spriteBaseName;
    auto anim = AnimationUtils::buildAnimationFromFrames(baseName, animType, frameCount, delay);
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
