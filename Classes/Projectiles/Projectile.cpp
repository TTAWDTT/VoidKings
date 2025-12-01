// Projectile.cpp
// 投射物/子弹类实现文件
// 功能: 实现投射物的飞行和伤害逻辑

#include "Projectile.h"
#include "../Units/Unit.h"

// ==================== 创建和初始化 ====================

Projectile* Projectile::create(ProjectileType type) {
    Projectile* projectile = new (std::nothrow) Projectile();
    if (projectile && projectile->initWithType(type)) {
        projectile->autorelease();
        return projectile;
    }
    CC_SAFE_DELETE(projectile);
    return nullptr;
}

Projectile::Projectile()
    : _projectileType(ProjectileType::STRAIGHT)
    , _faction(Faction::PLAYER)
    , _damage(50)
    , _damageType(DamageType::SINGLE)
    , _damageRadius(0)
    , _speed(400.0f)
    , _isLaunched(false)
    , _target(nullptr)
    , _targetPosition(Vec2::ZERO)
    , _startPosition(Vec2::ZERO)
    , _flightTime(0)
    , _totalFlightTime(0)
    , _maxHeight(50.0f)
    , _sprite(nullptr)
    , _trail(nullptr)
{
}

Projectile::~Projectile() {
    _target = nullptr;
    _enemies.clear();
}

bool Projectile::init() {
    if (!Node::init()) {
        return false;
    }
    return true;
}

bool Projectile::initWithType(ProjectileType type) {
    if (!init()) {
        return false;
    }
    
    _projectileType = type;
    
    // 根据弹道类型设置属性
    switch (_projectileType) {
        case ProjectileType::STRAIGHT:
            _speed = 500.0f;
            break;
        case ProjectileType::PARABOLIC:
            _speed = 200.0f;
            _maxHeight = 80.0f;
            break;
        case ProjectileType::TRACKING:
            _speed = 350.0f;
            break;
    }
    
    // 初始化精灵
    initSprite();
    
    return true;
}

void Projectile::initSprite() {
    // 创建投射物精灵
    auto drawNode = DrawNode::create();
    
    Color4F color;
    float size = 6.0f;
    
    switch (_projectileType) {
        case ProjectileType::STRAIGHT:
            color = Color4F(0.8f, 0.8f, 0.0f, 1.0f);  // 黄色 - 炮弹
            size = 6.0f;
            break;
        case ProjectileType::PARABOLIC:
            color = Color4F(0.3f, 0.3f, 0.3f, 1.0f);  // 灰色 - 迫击炮弹
            size = 10.0f;
            break;
        case ProjectileType::TRACKING:
            color = Color4F(1.0f, 0.5f, 0.0f, 1.0f);  // 橙色 - 追踪导弹
            size = 5.0f;
            break;
    }
    
    // 绘制投射物
    drawNode->drawSolidCircle(Vec2::ZERO, size/2, 0, 12, color);
    drawNode->drawCircle(Vec2::ZERO, size/2, 0, 12, false, Color4F(1, 1, 1, 0.5f));
    
    this->addChild(drawNode);
    this->setContentSize(Size(size, size));
}

// ==================== 发射和移动 ====================

void Projectile::launch() {
    if (_isLaunched) return;
    
    _isLaunched = true;
    _startPosition = this->getPosition();
    _flightTime = 0;
    
    // 计算目标位置
    if (_target) {
        _targetPosition = _target->getPosition();
    }
    
    // 计算总飞行时间
    float distance = _startPosition.distance(_targetPosition);
    _totalFlightTime = distance / _speed;
    
    // 启用更新
    scheduleUpdate();
}

void Projectile::setEnemyList(const std::vector<Unit*>& enemies) {
    _enemies = enemies;
}

void Projectile::update(float dt) {
    if (!_isLaunched) return;
    
    // 根据弹道类型移动
    moveToTarget(dt);
    
    // 检查是否命中
    if (checkHit()) {
        onHit();
    }
}

void Projectile::moveToTarget(float dt) {
    switch (_projectileType) {
        case ProjectileType::STRAIGHT:
            moveStraight(dt);
            break;
        case ProjectileType::PARABOLIC:
            moveParabolic(dt);
            break;
        case ProjectileType::TRACKING:
            moveTracking(dt);
            break;
    }
}

void Projectile::moveStraight(float dt) {
    Vec2 currentPos = this->getPosition();
    Vec2 direction = _targetPosition - currentPos;
    float distance = direction.length();
    
    if (distance < _speed * dt) {
        // 到达目标
        this->setPosition(_targetPosition);
    } else {
        direction.normalize();
        Vec2 newPos = currentPos + direction * _speed * dt;
        this->setPosition(newPos);
        
        // 更新朝向
        float angle = CC_RADIANS_TO_DEGREES(atan2(direction.y, direction.x));
        this->setRotation(-angle);
    }
}

void Projectile::moveParabolic(float dt) {
    _flightTime += dt;
    
    if (_flightTime >= _totalFlightTime) {
        // 到达目标
        this->setPosition(_targetPosition);
        return;
    }
    
    // 计算当前位置(抛物线)
    float t = _flightTime / _totalFlightTime;  // 归一化时间 0-1
    
    // 水平位置：线性插值
    float x = _startPosition.x + (_targetPosition.x - _startPosition.x) * t;
    
    // 垂直位置：线性插值 + 抛物线偏移
    float linearY = _startPosition.y + (_targetPosition.y - _startPosition.y) * t;
    float parabolicOffset = _maxHeight * 4 * t * (1 - t);  // 抛物线公式
    float y = linearY + parabolicOffset;
    
    this->setPosition(Vec2(x, y));
    
    // 更新朝向(指向运动方向)
    Vec2 velocity;
    velocity.x = (_targetPosition.x - _startPosition.x) / _totalFlightTime;
    velocity.y = ((_targetPosition.y - _startPosition.y) / _totalFlightTime) + 
                 _maxHeight * 4 * (1 - 2*t) / _totalFlightTime;
    float angle = CC_RADIANS_TO_DEGREES(atan2(velocity.y, velocity.x));
    this->setRotation(-angle);
}

void Projectile::moveTracking(float dt) {
    // 追踪类型：如果目标还活着，更新目标位置
    if (_target && !_target->isDead()) {
        _targetPosition = _target->getPosition();
    }
    
    // 使用直线移动到更新后的目标位置
    moveStraight(dt);
}

// ==================== 命中检测和处理 ====================

bool Projectile::checkHit() {
    Vec2 currentPos = this->getPosition();
    float hitDistance = 10.0f;  // 命中判定距离
    
    // 检查是否到达目标位置
    float distance = currentPos.distance(_targetPosition);
    return distance < hitDistance;
}

void Projectile::onHit() {
    // 造成伤害
    dealDamage();
    
    // 播放命中特效
    playHitEffect();
    
    // 移除投射物
    this->removeFromParent();
}

void Projectile::dealDamage() {
    if (_damageType == DamageType::SINGLE) {
        // 单体伤害
        if (_target && !_target->isDead()) {
            _target->takeDamage(_damage);
        }
    } else {
        // 范围伤害
        Vec2 hitPos = this->getPosition();
        
        for (auto enemy : _enemies) {
            if (!enemy || enemy->isDead()) continue;
            
            float distance = hitPos.distance(enemy->getPosition());
            if (distance <= _damageRadius) {
                // 在范围内，造成伤害(可以根据距离衰减)
                float damageMultiplier = 1.0f - (distance / _damageRadius) * 0.5f;
                int actualDamage = (int)(_damage * damageMultiplier);
                enemy->takeDamage(actualDamage);
            }
        }
    }
}

void Projectile::playHitEffect() {
    // 创建命中特效
    auto parent = this->getParent();
    if (!parent) return;
    
    Vec2 hitPos = this->getPosition();
    
    // 创建爆炸效果
    auto effect = Node::create();
    
    auto drawNode = DrawNode::create();
    Color4F color = (_damageType == DamageType::AREA) ? 
                    Color4F(1.0f, 0.5f, 0.0f, 0.8f) :
                    Color4F(1.0f, 1.0f, 0.0f, 0.8f);
    float radius = (_damageType == DamageType::AREA) ? _damageRadius : 15.0f;
    
    drawNode->drawSolidCircle(Vec2::ZERO, radius, 0, 20, color);
    effect->addChild(drawNode);
    
    effect->setPosition(hitPos);
    parent->addChild(effect, Z_EFFECT);
    
    // 播放动画后移除
    auto scale = ScaleTo::create(0.2f, 1.5f);
    auto fade = FadeOut::create(0.2f);
    auto spawn = Spawn::create(scale, fade, nullptr);
    auto remove = RemoveSelf::create();
    effect->runAction(Sequence::create(spawn, remove, nullptr));
}
