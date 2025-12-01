// DefenseBuilding.cpp
// 防御建筑类实现文件
// 功能: 实现防御建筑的攻击、索敌等功能

#include "DefenseBuilding.h"
#include "../Units/Unit.h"
#include "../Projectiles/Projectile.h"
#include <algorithm>

// ==================== 创建和初始化 ====================

DefenseBuilding* DefenseBuilding::create(BuildingType type, Faction faction) {
    DefenseBuilding* building = new (std::nothrow) DefenseBuilding();
    if (building && building->initWithType(type, faction)) {
        building->autorelease();
        return building;
    }
    CC_SAFE_DELETE(building);
    return nullptr;
}

DefenseBuilding::DefenseBuilding()
    : _attackRange(200.0f)
    , _damage(50)
    , _attackSpeed(1.0f)
    , _attackCooldown(0)
    , _damageType(DamageType::SINGLE)
    , _damageRadius(0)
    , _projectileType(ProjectileType::STRAIGHT)
    , _targetPriority(TargetPriority::NEAREST)
    , _canAttackAir(false)
    , _canAttackOverWall(false)
    , _currentTarget(nullptr)
    , _rangeIndicator(nullptr)
    , _turretSprite(nullptr)
{
}

DefenseBuilding::~DefenseBuilding() {
    _currentTarget = nullptr;
    _enemies.clear();
}

bool DefenseBuilding::initWithType(BuildingType type, Faction faction) {
    if (!Building::initWithType(type, faction)) {
        return false;
    }
    
    // 初始化防御属性
    initDefenseAttributes();
    
    // 创建攻击范围指示器
    createRangeIndicator();
    
    return true;
}

// ==================== 防御属性初始化 ====================

void DefenseBuilding::initDefenseAttributes() {
    // 根据防御建筑类型设置属性
    switch (_buildingType) {
        case BuildingType::CANNON:
            _attackRange = 250.0f;
            _damage = 60;
            _attackSpeed = 0.8f;
            _damageType = DamageType::SINGLE;
            _projectileType = ProjectileType::STRAIGHT;
            _canAttackAir = false;
            _canAttackOverWall = false;
            _targetPriority = TargetPriority::NEAREST;
            break;
            
        case BuildingType::ARCHER_TOWER:
            _attackRange = 300.0f;
            _damage = 40;
            _attackSpeed = 1.2f;
            _damageType = DamageType::SINGLE;
            _projectileType = ProjectileType::STRAIGHT;
            _canAttackAir = true;
            _canAttackOverWall = false;
            _targetPriority = TargetPriority::NEAREST;
            break;
            
        case BuildingType::MORTAR:
            _attackRange = 400.0f;
            _damage = 80;
            _attackSpeed = 0.3f;
            _damageType = DamageType::AREA;
            _damageRadius = 50.0f;
            _projectileType = ProjectileType::PARABOLIC;
            _canAttackAir = false;
            _canAttackOverWall = true;
            _targetPriority = TargetPriority::NEAREST;
            break;
            
        default:
            break;
    }
    
    // 根据等级调整属性
    float levelMultiplier = 1.0f + (_level - 1) * LEVEL_DAMAGE_MULTIPLIER;
    _damage = (int)(_damage * levelMultiplier);
    _attackRange *= (1.0f + (_level - 1) * LEVEL_RANGE_MULTIPLIER);
}

// ==================== 攻击范围显示 ====================

void DefenseBuilding::createRangeIndicator() {
    _rangeIndicator = DrawNode::create();
    
    // 绘制攻击范围圆形
    _rangeIndicator->drawCircle(Vec2::ZERO, _attackRange, 0, 60, false, 
                                Color4F(1.0f, 0.0f, 0.0f, 0.3f));
    _rangeIndicator->drawSolidCircle(Vec2::ZERO, _attackRange, 0, 60, 
                                     Color4F(1.0f, 0.0f, 0.0f, 0.1f));
    
    _rangeIndicator->setVisible(false);
    this->addChild(_rangeIndicator, -1);
}

void DefenseBuilding::showRange(bool show) {
    if (_rangeIndicator) {
        _rangeIndicator->setVisible(show);
    }
}

// ==================== 索敌系统 ====================

void DefenseBuilding::setEnemyList(const std::vector<Unit*>& enemies) {
    _enemies = enemies;
}

Unit* DefenseBuilding::findTarget() {
    if (_enemies.empty()) return nullptr;
    if (_state != BuildingState::NORMAL) return nullptr;
    
    Unit* bestTarget = nullptr;
    float bestValue = -1;
    
    Vec2 myPos = this->getPosition();
    
    // 清理无效的敌人引用
    _enemies.erase(
        std::remove_if(_enemies.begin(), _enemies.end(), 
            [](Unit* u) { return u == nullptr || u->isDead() || u->getParent() == nullptr; }),
        _enemies.end()
    );
    
    for (auto unit : _enemies) {
        if (!unit || unit->isDead() || unit->getParent() == nullptr) continue;
        
        // 检查是否可以攻击该单位
        if (unit->isAirUnit() && !_canAttackAir) continue;
        
        Vec2 unitPos = unit->getPosition();
        float distance = myPos.distance(unitPos);
        
        // 检查是否在攻击范围内
        if (distance > _attackRange) continue;
        
        float targetValue = 0;
        
        // 根据目标优先级计算目标价值
        switch (_targetPriority) {
            case TargetPriority::NEAREST:
                targetValue = _attackRange - distance;
                break;
                
            case TargetPriority::LOWEST_HP:
                targetValue = (float)(unit->getMaxHP() - unit->getCurrentHP());
                break;
                
            case TargetPriority::DEFENSE_FIRST:
                // 对于防御建筑，优先攻击靠近的单位
                targetValue = _attackRange - distance;
                break;
                
            default:
                targetValue = _attackRange - distance;
                break;
        }
        
        if (targetValue > bestValue) {
            bestValue = targetValue;
            bestTarget = unit;
        }
    }
    
    return bestTarget;
}

void DefenseBuilding::clearTarget() {
    _currentTarget = nullptr;
}

// ==================== 攻击系统 ====================

void DefenseBuilding::attackTarget() {
    if (!_currentTarget || _currentTarget->isDead()) {
        _currentTarget = nullptr;
        return;
    }
    
    // 检查目标是否仍在范围内
    Vec2 myPos = this->getPosition();
    Vec2 targetPos = _currentTarget->getPosition();
    float distance = myPos.distance(targetPos);
    
    if (distance > _attackRange) {
        _currentTarget = nullptr;
        return;
    }
    
    // 旋转炮塔朝向目标
    rotateTurretToTarget(targetPos);
    
    // 发射子弹
    fireProjectile(_currentTarget);
    
    // 播放攻击动画
    playAttackAnimation();
}

void DefenseBuilding::rotateTurretToTarget(const Vec2& target) {
    if (_turretSprite) {
        Vec2 myPos = this->getPosition();
        Vec2 direction = target - myPos;
        float angle = CC_RADIANS_TO_DEGREES(atan2(direction.y, direction.x));
        _turretSprite->setRotation(-angle + 90);
    }
}

void DefenseBuilding::fireProjectile(Unit* target) {
    if (!target) return;
    
    // 创建子弹
    Projectile* projectile = createProjectile();
    if (projectile) {
        // 设置子弹属性
        projectile->setDamage(_damage);
        projectile->setDamageType(_damageType);
        projectile->setDamageRadius(_damageRadius);
        projectile->setTarget(target);
        projectile->setFaction(_faction);
        
        // 设置子弹初始位置
        projectile->setPosition(this->getPosition());
        
        // 添加到父节点
        if (this->getParent()) {
            this->getParent()->addChild(projectile, Z_PROJECTILE);
            projectile->launch();
        }
    }
}

Projectile* DefenseBuilding::createProjectile() {
    return Projectile::create(_projectileType);
}

void DefenseBuilding::playAttackAnimation() {
    // 播放攻击时的后坐力动画
    auto scale1 = ScaleTo::create(0.05f, 0.95f);
    auto scale2 = ScaleTo::create(0.1f, 1.0f);
    this->runAction(Sequence::create(scale1, scale2, nullptr));
}

// ==================== 帧更新 ====================

void DefenseBuilding::update(float dt) {
    // 调用父类更新
    Building::update(dt);
    
    // 如果建筑不在正常状态，不进行战斗
    if (_state != BuildingState::NORMAL) return;
    
    // 验证自身状态
    if (!this->getParent()) return;
    
    // 更新攻击冷却
    if (_attackCooldown > 0) {
        _attackCooldown -= dt;
        return;
    }
    
    // 检查当前目标是否有效
    if (_currentTarget) {
        bool targetValid = !_currentTarget->isDead() && _currentTarget->getParent() != nullptr;
        if (!targetValid) {
            _currentTarget = nullptr;
        } else {
            float distance = this->getPosition().distance(_currentTarget->getPosition());
            if (distance > _attackRange) {
                _currentTarget = nullptr;
            }
        }
    }
    
    // 如果没有目标，尝试索敌
    if (!_currentTarget) {
        _currentTarget = findTarget();
    }
    
    // 如果有目标，进行攻击
    if (_currentTarget) {
        attackTarget();
        _attackCooldown = 1.0f / _attackSpeed;
    }
}
