// Soldier.cpp
#include "Soldier.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Buildings/StorageBuilding.h"
#include "Utils/AnimationUtils.h"
#include "Utils/EffectUtils.h"
#include "Utils/AudioManager.h"
#include "Utils/NodeUtils.h"
#include <algorithm>
#include <cmath>
#include <limits>
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

bool isMageUnit(const UnitConfig* config) {
    if (!config) {
        return false;
    }
    return config->name.find("Mage") != std::string::npos;
}

// 攻击判定的容差，避免贴近目标却卡在移动动画
constexpr float kAttackRangeTolerance = 6.0f;
// 最小移动步长，低于此值不切换为移动动画
constexpr float kMinMoveStep = 0.05f;
// 目标刷新间隔，避免每帧全量扫描
constexpr float kTargetRefreshInterval = 0.25f;
// 目标切换门槛，差距不大时保持当前目标
constexpr float kTargetSwitchThreshold = 15.0f;
// 评分比较的微小容差
constexpr float kTargetScoreEpsilon = 0.01f;

cocos2d::Rect rectInParentSpace(const cocos2d::Node* node,
                               const cocos2d::Sprite* sprite,
                               const cocos2d::Node* parent) {
    const cocos2d::Node* ref = sprite ? static_cast<const cocos2d::Node*>(sprite) : node;
    if (!ref) {
        return cocos2d::Rect::ZERO;
    }

    cocos2d::Rect localRect = ref->getBoundingBox();
    const cocos2d::Node* refParent = ref->getParent();
    if (!refParent) {
        if (!parent) {
            return localRect;
        }
        cocos2d::Vec2 parentBL = parent->convertToNodeSpace(localRect.origin);
        cocos2d::Vec2 parentTR = parent->convertToNodeSpace(localRect.origin +
            cocos2d::Vec2(localRect.size.width, localRect.size.height));
        float minX = std::min(parentBL.x, parentTR.x);
        float minY = std::min(parentBL.y, parentTR.y);
        float maxX = std::max(parentBL.x, parentTR.x);
        float maxY = std::max(parentBL.y, parentTR.y);
        return cocos2d::Rect(minX, minY, maxX - minX, maxY - minY);
    }

    cocos2d::Vec2 worldBL = refParent->convertToWorldSpace(localRect.origin);
    cocos2d::Vec2 worldTR = refParent->convertToWorldSpace(localRect.origin +
        cocos2d::Vec2(localRect.size.width, localRect.size.height));

    if (!parent) {
        float minX = std::min(worldBL.x, worldTR.x);
        float minY = std::min(worldBL.y, worldTR.y);
        float maxX = std::max(worldBL.x, worldTR.x);
        float maxY = std::max(worldBL.y, worldTR.y);
        return cocos2d::Rect(minX, minY, maxX - minX, maxY - minY);
    }

    cocos2d::Vec2 parentBL = parent->convertToNodeSpace(worldBL);
    cocos2d::Vec2 parentTR = parent->convertToNodeSpace(worldTR);
    float minX = std::min(parentBL.x, parentTR.x);
    float minY = std::min(parentBL.y, parentTR.y);
    float maxX = std::max(parentBL.x, parentTR.x);
    float maxY = std::max(parentBL.y, parentTR.y);
    return cocos2d::Rect(minX, minY, maxX - minX, maxY - minY);
}

float rectDistance(const cocos2d::Rect& a, const cocos2d::Rect& b) {
    float dx = 0.0f;
    if (a.getMaxX() < b.getMinX()) {
        dx = b.getMinX() - a.getMaxX();
    }
    else if (b.getMaxX() < a.getMinX()) {
        dx = a.getMinX() - b.getMaxX();
    }

    float dy = 0.0f;
    if (a.getMaxY() < b.getMinY()) {
        dy = b.getMinY() - a.getMaxY();
    }
    else if (b.getMaxY() < a.getMinY()) {
        dy = a.getMinY() - b.getMaxY();
    }

    return std::sqrt(dx * dx + dy * dy);
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
   _targetRefreshTimer = 0.0f;
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
    if (_level >= 0 && static_cast<size_t>(_level) < _config->HP.size()) {
        return _config->HP[_level];
    }
    return _config->HP.empty() ? 0.0f : _config->HP[0];
}

float Soldier::getCurrentSpeed() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->SPEED.size()) {
        return _config->SPEED[_level];
    }
    return _config->SPEED.empty() ? 0.0f : _config->SPEED[0];
}

float Soldier::getCurrentATK() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->ATK.size()) {
        return _config->ATK[_level];
    }
    return _config->ATK.empty() ? 0.0f : _config->ATK[0];
}

float Soldier::getCurrentRange() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->RANGE.size()) {
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
        _targetRefreshTimer = 0.0f;
    }

    _targetRefreshTimer -= dt;
    if (_targetRefreshTimer <= 0.0f) {
        findTarget();
        _targetRefreshTimer = kTargetRefreshInterval;
    }

    if (_target) {
        float dist = getDistanceToTarget(_target);
        float attackRange = getCurrentRange();
        if (dist <= attackRange + kAttackRangeTolerance) {
            attackTarget();
            tryPlayIdleAnimation();
        }
        else {
            moveToTarget(dt);
        }
    }
    else {
        tryPlayIdleAnimation();
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

    const bool wantDefense = _config && _config->aiType == TargetPriority::DEFENSE;
    const bool wantResource = _config && _config->aiType == TargetPriority::RESOURCE;

    // 过滤无效目标并标记类型
    auto classifyBuilding = [&](cocos2d::Node* building, bool& isDefense, bool& isResource) -> bool {
        if (!building || !building->getParent()) {
            return false;
        }

        isDefense = false;
        isResource = false;

        if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
            if (defence->getCurrentHP() <= 0.0f) {
                return false;
            }
            isDefense = true;
        }
        else if (auto* production = dynamic_cast<ProductionBuilding*>(building)) {
            if (production->getCurrentHP() <= 0.0f) {
                return false;
            }
            isResource = true;
        }
        else if (auto* storage = dynamic_cast<StorageBuilding*>(building)) {
            if (storage->getCurrentHP() <= 0.0f) {
                return false;
            }
            isResource = true;
        }

        return true;
    };

    // 评分：距离扣除攻击范围，越小越接近可攻击
    auto calcScore = [&](cocos2d::Node* building, float& outDist) -> float {
        outDist = getDistanceToTarget(building);
        float attackRange = getCurrentRange();
        float score = outDist - attackRange;
        if (score < 0.0f) {
            score = 0.0f;
        }
        return score;
    };

    // 按偏好挑选最佳目标（若分数接近则选更近的）
    auto pickBest = [&](bool onlyDefense, bool onlyResource, float& outScore) -> cocos2d::Node* {
        cocos2d::Node* best = nullptr;
        float bestScore = std::numeric_limits<float>::max();
        float bestDist = std::numeric_limits<float>::max();

        for (auto* building : *s_enemyBuildings) {
            bool isDefense = false;
            bool isResource = false;
            if (!classifyBuilding(building, isDefense, isResource)) {
                continue;
            }
            if (onlyDefense && !isDefense) {
                continue;
            }
            if (onlyResource && !isResource) {
                continue;
            }

            float dist = 0.0f;
            float score = calcScore(building, dist);
            if (score < bestScore - kTargetScoreEpsilon
                || (std::abs(score - bestScore) <= kTargetScoreEpsilon && dist < bestDist)) {
                bestScore = score;
                bestDist = dist;
                best = building;
            }
        }

        outScore = bestScore;
        return best;
    };

    cocos2d::Node* bestTarget = nullptr;
    float bestScore = std::numeric_limits<float>::max();
    bool hasPriorityTarget = false;

    if (wantDefense || wantResource) {
        bestTarget = pickBest(wantDefense, wantResource, bestScore);
        hasPriorityTarget = (bestTarget != nullptr);
    }

    if (!bestTarget) {
        bestTarget = pickBest(false, false, bestScore);
    }

    if (!bestTarget) {
        return;
    }

    if (_target && _target->getParent()) {
        bool currentDefense = false;
        bool currentResource = false;
        if (classifyBuilding(_target, currentDefense, currentResource)) {
            float currentDist = 0.0f;
            float currentScore = calcScore(_target, currentDist);
            float attackRange = getCurrentRange();
            bool inRange = currentDist <= attackRange + kAttackRangeTolerance;

            // 已进入攻击距离时保持目标，避免来回切换
            if (inRange) {
                return;
            }

            // 有优先级目标且当前目标不匹配时直接切换
            if (hasPriorityTarget && ((wantDefense && !currentDefense) || (wantResource && !currentResource))) {
                setTarget(bestTarget);
                return;
            }

            if (bestTarget == _target) {
                return;
            }

            // 目标差距不明显时不切换，减少抖动
            if (currentScore <= bestScore + kTargetSwitchThreshold) {
                return;
            }
        }
    }

    setTarget(bestTarget);
}

void Soldier::moveToTarget(float dt) {
    if (!_target) return;

    cocos2d::Vec2 targetPos = getTargetPositionInParent(_target);
    cocos2d::Vec2 diff = targetPos - this->getPosition();
    float dist = getDistanceToTarget(_target);
    float stopDistance = getCurrentRange() + kAttackRangeTolerance;
    if (dist <= stopDistance) {
        tryPlayIdleAnimation();
        return;
    }

    cocos2d::Vec2 direction = diff.getNormalized();
    float step = getCurrentSpeed() * dt;
    if (step <= 0.0f) {
        // 速度为0或dt异常时不切换为移动动画
        tryPlayIdleAnimation();
        return;
    }
    float remaining = dist - stopDistance;
    if (remaining < 0.0f) {
        remaining = 0.0f;
    }
    float move = std::min(step, remaining);
    if (remaining <= kMinMoveStep) {
        // 剩余距离很小也要补齐，否则会卡在攻击距离外
        if (remaining > 0.0f) {
            this->setPosition(this->getPosition() + direction * remaining);
        }
        tryPlayIdleAnimation();
        return;
    }
    if (move <= kMinMoveStep) {
        // 步长过小，先推进但不切换移动动画，避免抖动
        this->setPosition(this->getPosition() + direction * move);
        tryPlayIdleAnimation();
        return;
    }

    // 计算方向(只有左右)
    Direction newDir = calcDirection(this->getPosition(), targetPos);
    // 更新精灵朝向并播放移动动画
    updateSpriteDirection(newDir);
    playAnimation(_config->anim_walk, _config->anim_walk_frames, _config->anim_walk_delay, true);

    cocos2d::Vec2 newPos = this->getPosition() + (direction * move);
    this->setPosition(newPos);
}

void Soldier::takeDamage(float damage) {
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;

    EffectUtils::playHitFlash(_bodySprite);
    AudioManager::playRandomHit();
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

    float dist = getDistanceToTarget(_target);
    float attackRange = getCurrentRange();
    if (dist > attackRange + kAttackRangeTolerance) {
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
    cocos2d::Vec2 targetPos = getTargetPositionInParent(_target);
    Direction attackDir = calcDirection(this->getPosition(), targetPos);
    updateSpriteDirection(attackDir);

    // 播放攻击动画
    playAnimation(_config->anim_attack, _config->anim_attack_frames, _config->anim_attack_delay, false);

    if (_config) {
        if (_config->ISREMOTE) {
            if (isMageUnit(_config)) {
                AudioManager::playMagicAttack();
            }
            else {
                AudioManager::playArrowShoot();
            }
        }
        else {
            AudioManager::playMeleeHit();
        }
    }

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

void Soldier::tryPlayIdleAnimation() {
    if (!_config) {
        return;
    }
    if (_currentActionKey == _config->anim_attack || _currentActionKey == _config->anim_dead) {
        return;
    }

    std::string prevKey = _currentActionKey;
    playAnimation(_config->anim_idle, _config->anim_idle_frames, _config->anim_idle_delay, true);
    if (_currentActionKey == prevKey && prevKey != _config->anim_idle) {
        // 没有待机资源时停止循环移动动画
        stopCurrentAnimation();
    }
}

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

cocos2d::Vec2 Soldier::getTargetPositionInParent(const cocos2d::Node* target) const {
    if (!target) {
        return cocos2d::Vec2::ZERO;
    }

    auto* parent = this->getParent();
    if (!parent || target->getParent() == parent) {
        return target->getPosition();
    }

    cocos2d::Vec2 worldPos = target->convertToWorldSpace(cocos2d::Vec2::ZERO);
    return parent->convertToNodeSpace(worldPos);
}

float Soldier::getDistanceToTarget(const cocos2d::Node* target) const {
    if (!target) {
        return std::numeric_limits<float>::max();
    }

    cocos2d::Vec2 selfPos = this->getPosition();
    cocos2d::Vec2 targetPos = getTargetPositionInParent(target);
    if (_config && _config->ISREMOTE) {
        return selfPos.distance(targetPos);
    }

    const cocos2d::Node* parent = this->getParent();
    cocos2d::Rect selfRect = rectInParentSpace(this, _bodySprite, parent);
    cocos2d::Rect targetRect = rectInParentSpace(target, NodeUtils::findBodySprite(target), parent);
    if (selfRect.size.width <= 0.0f || selfRect.size.height <= 0.0f
        || targetRect.size.width <= 0.0f || targetRect.size.height <= 0.0f) {
        return selfPos.distance(targetPos);
    }

    // 近战单位使用边缘距离，避免贴近目标却一直走动。
    return rectDistance(selfRect, targetRect);
}

// 更新精灵朝向 - 通过水平翻转实现左向
void Soldier::updateSpriteDirection(Direction dir) {
    if (!_bodySprite) return;
    
    if (dir == _direction) return; // 方向未变,无需更新
    
    _direction = dir;
    
    // 素材默认朝右,左向通过scaleX=-1实现翻转
    float baseScaleX = std::abs(_bodySprite->getScaleX());
    if (baseScaleX <= 0.0f) {
        baseScaleX = 1.0f;
    }
    if (dir == Direction::LEFT) {
        _bodySprite->setScaleX(-baseScaleX);
    }
    else {
        _bodySprite->setScaleX(baseScaleX);
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
