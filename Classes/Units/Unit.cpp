// Unit.cpp
// 兵种/单位类实现文件
// 功能: 实现单位的移动、攻击、索敌等功能

#include "Unit.h"
#include "../Buildings/Building.h"
#include "../Core/ResourceManager.h"

// ==================== 创建和初始化 ====================

Unit* Unit::create(UnitType type, Faction faction) {
    Unit* unit = new (std::nothrow) Unit();
    if (unit && unit->initWithType(type, faction)) {
        unit->autorelease();
        return unit;
    }
    CC_SAFE_DELETE(unit);
    return nullptr;
}

Unit::Unit()
    : _unitType(UnitType::BARBARIAN)
    , _faction(Faction::PLAYER)
    , _state(UnitState::IDLE)
    , _unitName("Unit")
    , _currentHP(100)
    , _maxHP(100)
    , _damage(20)
    , _attackRange(50.0f)
    , _attackSpeed(1.0f)
    , _attackCooldown(0)
    , _moveSpeed(100.0f)
    , _isAirUnit(false)
    , _targetPriority(TargetPriority::NEAREST)
    , _currentTarget(nullptr)
    , _population(1)
    , _pathIndex(0)
    , _targetPosition(Vec2::ZERO)
    , _sprite(nullptr)
    , _healthBar(nullptr)
    , _selectionIndicator(nullptr)
    , _isSelected(false)
    , _isMoving(false)
{
}

Unit::~Unit() {
    _buildings.clear();
    _enemies.clear();
    _path.clear();
    _currentTarget = nullptr;
}

bool Unit::init() {
    if (!Node::init()) {
        return false;
    }
    return true;
}

bool Unit::initWithType(UnitType type, Faction faction) {
    if (!init()) {
        return false;
    }
    
    _unitType = type;
    _faction = faction;
    
    // 初始化单位属性
    initAttributes();
    
    // 初始化精灵
    initSprite();
    
    // 创建血条
    createHealthBar();
    
    // 启用帧更新
    scheduleUpdate();
    
    return true;
}

// ==================== 属性初始化 ====================

void Unit::initAttributes() {
    switch (_unitType) {
        case UnitType::BARBARIAN:
            _unitName = "Barbarian";
            _maxHP = 100;
            _damage = 20;
            _attackRange = 30.0f;
            _attackSpeed = 1.0f;
            _moveSpeed = 100.0f;
            _isAirUnit = false;
            _population = 1;
            _targetPriority = TargetPriority::NEAREST;
            break;
            
        case UnitType::ARCHER:
            _unitName = "Archer";
            _maxHP = 80;
            _damage = 15;
            _attackRange = 200.0f;
            _attackSpeed = 1.2f;
            _moveSpeed = 120.0f;
            _isAirUnit = false;
            _population = 1;
            _targetPriority = TargetPriority::NEAREST;
            break;
            
        case UnitType::GIANT:
            _unitName = "Giant";
            _maxHP = 400;
            _damage = 40;
            _attackRange = 40.0f;
            _attackSpeed = 0.5f;
            _moveSpeed = 60.0f;
            _isAirUnit = false;
            _population = 5;
            _targetPriority = TargetPriority::DEFENSE_FIRST;
            break;
            
        case UnitType::GOBLIN:
            _unitName = "Goblin";
            _maxHP = 60;
            _damage = 12;
            _attackRange = 30.0f;
            _attackSpeed = 1.5f;
            _moveSpeed = 150.0f;
            _isAirUnit = false;
            _population = 1;
            _targetPriority = TargetPriority::RESOURCE_FIRST;
            break;
            
        case UnitType::WIZARD:
            _unitName = "Wizard";
            _maxHP = 75;
            _damage = 50;
            _attackRange = 180.0f;
            _attackSpeed = 0.8f;
            _moveSpeed = 80.0f;
            _isAirUnit = false;
            _population = 4;
            _targetPriority = TargetPriority::NEAREST;
            break;
            
        case UnitType::HEALER:
            _unitName = "Healer";
            _maxHP = 100;
            _damage = 0;  // 治疗单位，不造成伤害
            _attackRange = 300.0f;
            _attackSpeed = 0.7f;
            _moveSpeed = 80.0f;
            _isAirUnit = true;
            _population = 14;
            _targetPriority = TargetPriority::LOWEST_HP;
            break;
            
        default:
            break;
    }
    
    _currentHP = _maxHP;
}

void Unit::initSprite() {
    // 根据单位类型选择纹理
    std::string texturePath = "textures/units/";
    
    switch (_unitType) {
        case UnitType::BARBARIAN:
            texturePath += "barbarian.png";
            break;
        case UnitType::ARCHER:
            texturePath += "archer.png";
            break;
        case UnitType::GIANT:
            texturePath += "giant.png";
            break;
        case UnitType::GOBLIN:
            texturePath += "goblin.png";
            break;
        case UnitType::WIZARD:
            texturePath += "wizard.png";
            break;
        case UnitType::HEALER:
            texturePath += "healer.png";
            break;
        default:
            texturePath += "default.png";
            break;
    }
    
    // 尝试加载纹理
    _sprite = Sprite::create(texturePath);
    if (!_sprite) {
        // 创建占位图形
        auto drawNode = DrawNode::create();
        float size = 20.0f;
        
        // 根据单位类型设置颜色和大小
        Color4F color;
        switch (_unitType) {
            case UnitType::BARBARIAN:
                color = Color4F(0.8f, 0.6f, 0.2f, 1.0f);
                size = 16.0f;
                break;
            case UnitType::ARCHER:
                color = Color4F(0.2f, 0.8f, 0.2f, 1.0f);
                size = 14.0f;
                break;
            case UnitType::GIANT:
                color = Color4F(0.6f, 0.4f, 0.2f, 1.0f);
                size = 28.0f;
                break;
            case UnitType::GOBLIN:
                color = Color4F(0.2f, 0.6f, 0.2f, 1.0f);
                size = 12.0f;
                break;
            case UnitType::WIZARD:
                color = Color4F(0.6f, 0.2f, 0.8f, 1.0f);
                size = 16.0f;
                break;
            case UnitType::HEALER:
                color = Color4F(1.0f, 0.8f, 0.9f, 1.0f);
                size = 18.0f;
                break;
            default:
                color = Color4F::WHITE;
                break;
        }
        
        // 绘制单位圆形
        drawNode->drawSolidCircle(Vec2::ZERO, size/2, 0, 20, color);
        drawNode->drawCircle(Vec2::ZERO, size/2, 0, 20, false, Color4F::BLACK);
        
        this->addChild(drawNode, 1);
        this->setContentSize(Size(size, size));
    } else {
        _sprite->setScale(0.5f);
        this->addChild(_sprite, 1);
        this->setContentSize(_sprite->getContentSize() * 0.5f);
    }
    
    // 创建选中指示器
    _selectionIndicator = DrawNode::create();
    float size = this->getContentSize().width;
    _selectionIndicator->drawCircle(Vec2::ZERO, size/2 + 3, 0, 20, false, Color4F::GREEN);
    _selectionIndicator->setVisible(false);
    this->addChild(_selectionIndicator, 0);
}

// ==================== 血条管理 ====================

void Unit::createHealthBar() {
    float barWidth = 30;
    float barHeight = 4;
    
    _healthBar = Node::create();
    
    // 背景条
    auto bgBar = DrawNode::create();
    bgBar->drawSolidRect(Vec2(-barWidth/2, -barHeight/2), 
                         Vec2(barWidth/2, barHeight/2), 
                         Color4F(0.2f, 0.2f, 0.2f, 0.8f));
    bgBar->setName("bgBar");
    _healthBar->addChild(bgBar);
    
    // 血量条
    auto hpBar = DrawNode::create();
    hpBar->drawSolidRect(Vec2(-barWidth/2 + 1, -barHeight/2 + 1), 
                        Vec2(barWidth/2 - 1, barHeight/2 - 1), 
                        Color4F(0.2f, 0.8f, 0.2f, 1.0f));
    hpBar->setName("hpBar");
    _healthBar->addChild(hpBar);
    
    // 设置位置
    float height = this->getContentSize().height;
    _healthBar->setPosition(Vec2(0, height/2 + 8));
    
    this->addChild(_healthBar, 100);
}

void Unit::updateHealthBar() {
    if (!_healthBar) return;
    
    float hpRatio = (float)_currentHP / (float)_maxHP;
    hpRatio = std::max(0.0f, std::min(1.0f, hpRatio));
    
    auto hpBar = dynamic_cast<DrawNode*>(_healthBar->getChildByName("hpBar"));
    if (hpBar) {
        hpBar->clear();
        
        float barWidth = 30;
        float barHeight = 4;
        
        Color4F hpColor;
        if (hpRatio > 0.6f) {
            hpColor = Color4F(0.2f, 0.8f, 0.2f, 1.0f);
        } else if (hpRatio > 0.3f) {
            hpColor = Color4F(0.8f, 0.8f, 0.2f, 1.0f);
        } else {
            hpColor = Color4F(0.8f, 0.2f, 0.2f, 1.0f);
        }
        
        float actualWidth = (barWidth - 2) * hpRatio;
        hpBar->drawSolidRect(Vec2(-barWidth/2 + 1, -barHeight/2 + 1), 
                            Vec2(-barWidth/2 + 1 + actualWidth, barHeight/2 - 1), 
                            hpColor);
    }
}

// ==================== 战斗接口 ====================

int Unit::takeDamage(int damage) {
    if (_state == UnitState::DEAD) return 0;
    
    _currentHP -= damage;
    
    if (_currentHP <= 0) {
        _currentHP = 0;
        _state = UnitState::DEAD;
        onDead();
    }
    
    updateHealthBar();
    return _currentHP;
}

void Unit::heal(int amount) {
    if (_state == UnitState::DEAD) return;
    
    _currentHP += amount;
    if (_currentHP > _maxHP) {
        _currentHP = _maxHP;
    }
    
    updateHealthBar();
}

void Unit::setBuildingList(const std::vector<Building*>& buildings) {
    _buildings = buildings;
}

void Unit::setEnemyList(const std::vector<Unit*>& enemies) {
    _enemies = enemies;
}

Node* Unit::findTarget() {
    Node* bestTarget = nullptr;
    float bestValue = -1;
    Vec2 myPos = this->getPosition();
    
    // 根据目标优先级查找目标
    switch (_targetPriority) {
        case TargetPriority::DEFENSE_FIRST:
            // 优先攻击防御建筑
            for (auto building : _buildings) {
                if (!building || building->isDestroyed()) continue;
                
                BuildingType type = building->getBuildingType();
                if (type == BuildingType::CANNON || 
                    type == BuildingType::ARCHER_TOWER || 
                    type == BuildingType::MORTAR) {
                    float distance = myPos.distance(building->getPosition());
                    float value = 10000.0f - distance;  // 优先防御建筑
                    if (value > bestValue) {
                        bestValue = value;
                        bestTarget = building;
                    }
                }
            }
            // 如果没有防御建筑，攻击最近的建筑
            if (!bestTarget) {
                for (auto building : _buildings) {
                    if (!building || building->isDestroyed()) continue;
                    float distance = myPos.distance(building->getPosition());
                    float value = 5000.0f - distance;
                    if (value > bestValue) {
                        bestValue = value;
                        bestTarget = building;
                    }
                }
            }
            break;
            
        case TargetPriority::RESOURCE_FIRST:
            // 优先攻击资源建筑
            for (auto building : _buildings) {
                if (!building || building->isDestroyed()) continue;
                
                BuildingType type = building->getBuildingType();
                if (type == BuildingType::GOLD_MINE || 
                    type == BuildingType::ELIXIR_COLLECTOR ||
                    type == BuildingType::GOLD_STORAGE ||
                    type == BuildingType::ELIXIR_STORAGE) {
                    float distance = myPos.distance(building->getPosition());
                    float value = 10000.0f - distance;
                    if (value > bestValue) {
                        bestValue = value;
                        bestTarget = building;
                    }
                }
            }
            // 如果没有资源建筑，攻击最近的建筑
            if (!bestTarget) {
                for (auto building : _buildings) {
                    if (!building || building->isDestroyed()) continue;
                    float distance = myPos.distance(building->getPosition());
                    float value = 5000.0f - distance;
                    if (value > bestValue) {
                        bestValue = value;
                        bestTarget = building;
                    }
                }
            }
            break;
            
        case TargetPriority::NEAREST:
        default:
            // 攻击最近的目标
            for (auto building : _buildings) {
                if (!building || building->isDestroyed()) continue;
                float distance = myPos.distance(building->getPosition());
                float value = 10000.0f - distance;
                if (value > bestValue) {
                    bestValue = value;
                    bestTarget = building;
                }
            }
            break;
    }
    
    return bestTarget;
}

void Unit::moveTo(const Vec2& target) {
    _targetPosition = target;
    _state = UnitState::MOVING;
    _isMoving = true;
    
    // 简单的直线移动，实际游戏中应使用寻路算法
    _path.clear();
    _path.push_back(target);
    _pathIndex = 0;
    
    playMoveAnimation();
}

void Unit::attack(Node* target) {
    if (!target) return;
    
    _currentTarget = target;
    _state = UnitState::ATTACKING;
    
    // 检查是否在攻击范围内
    Vec2 myPos = this->getPosition();
    Vec2 targetPos = target->getPosition();
    float distance = myPos.distance(targetPos);
    
    if (distance <= _attackRange) {
        // 在范围内，直接攻击
        playAttackAnimation();
        
        // 造成伤害
        if (auto building = dynamic_cast<Building*>(target)) {
            building->takeDamage(_damage);
        } else if (auto unit = dynamic_cast<Unit*>(target)) {
            unit->takeDamage(_damage);
        }
    } else {
        // 不在范围内，移动靠近
        Vec2 direction = (targetPos - myPos).getNormalized();
        Vec2 moveTarget = targetPos - direction * (_attackRange * 0.8f);
        moveTo(moveTarget);
    }
}

void Unit::stopMoving() {
    _isMoving = false;
    _state = UnitState::IDLE;
    _path.clear();
    this->stopAllActions();
    playIdleAnimation();
}

void Unit::deploy(const Vec2& position) {
    this->setPosition(position);
    _state = UnitState::IDLE;
    
    // 播放部署动画
    this->setScale(0);
    auto scale = ScaleTo::create(0.3f, 1.0f);
    this->runAction(scale);
}

// ==================== 路径移动 ====================

std::vector<Vec2> Unit::findPath(const Vec2& target) {
    // 简单实现：直线路径
    // 实际游戏中应使用A*等寻路算法
    std::vector<Vec2> path;
    path.push_back(target);
    return path;
}

// 检查目标是否有效（未被销毁且仍在场景中）
bool Unit::isTargetValid(Node* target) const {
    if (!target || target->getParent() == nullptr) {
        return false;
    }
    
    if (auto building = dynamic_cast<Building*>(target)) {
        return !building->isDestroyed();
    } else if (auto unit = dynamic_cast<Unit*>(target)) {
        return !unit->isDead();
    }
    
    return false;
}

void Unit::moveAlongPath(float dt) {
    if (_path.empty() || _pathIndex >= (int)_path.size()) {
        stopMoving();
        return;
    }
    
    Vec2 target = _path[_pathIndex];
    Vec2 myPos = this->getPosition();
    Vec2 direction = target - myPos;
    float distance = direction.length();
    
    if (distance < 5.0f) {
        // 到达当前路径点
        _pathIndex++;
        if (_pathIndex >= (int)_path.size()) {
            stopMoving();
        }
        return;
    }
    
    // 移动
    direction.normalize();
    Vec2 newPos = myPos + direction * _moveSpeed * dt;  // 使用dt实现帧率无关的移动
    this->setPosition(newPos);
    
    // 更新朝向
    float angle = CC_RADIANS_TO_DEGREES(atan2(direction.y, direction.x));
    // 根据方向翻转精灵
    if (_sprite) {
        _sprite->setFlippedX(direction.x < 0);
    }
}

// ==================== 动画效果 ====================

void Unit::playMoveAnimation() {
    // 移动时的轻微上下浮动效果
    auto moveUp = MoveBy::create(0.2f, Vec2(0, 2));
    auto moveDown = MoveBy::create(0.2f, Vec2(0, -2));
    auto seq = Sequence::create(moveUp, moveDown, nullptr);
    auto repeat = RepeatForever::create(seq);
    repeat->setTag(1);
    this->runAction(repeat);
}

void Unit::playAttackAnimation() {
    // 攻击时的缩放效果
    auto scale1 = ScaleTo::create(0.1f, 1.2f);
    auto scale2 = ScaleTo::create(0.1f, 1.0f);
    this->runAction(Sequence::create(scale1, scale2, nullptr));
}

void Unit::playDeathAnimation() {
    // 死亡动画：旋转+缩小+淡出
    auto rotate = RotateBy::create(0.5f, 360);
    auto scale = ScaleTo::create(0.5f, 0);
    auto fade = FadeOut::create(0.5f);
    auto spawn = Spawn::create(rotate, scale, fade, nullptr);
    auto callback = CallFunc::create([this]() {
        this->removeFromParent();
    });
    this->runAction(Sequence::create(spawn, callback, nullptr));
}

void Unit::playIdleAnimation() {
    // 停止移动动画
    this->stopActionByTag(1);
}

void Unit::setSelected(bool selected) {
    _isSelected = selected;
    if (_selectionIndicator) {
        _selectionIndicator->setVisible(selected);
    }
}

void Unit::onDead() {
    // 释放人口
    ResourceManager::getInstance()->releasePopulation(_population);
    
    // 播放死亡动画
    playDeathAnimation();
}

// ==================== 状态机更新 ====================

void Unit::updateStateMachine(float dt) {
    switch (_state) {
        case UnitState::IDLE:
            // 待机状态：寻找目标
            _currentTarget = findTarget();
            if (_currentTarget) {
                // 有目标，开始攻击
                attack(_currentTarget);
            }
            break;
            
        case UnitState::MOVING:
            // 移动状态：沿路径移动
            moveAlongPath(dt);
            
            // 检查目标是否仍有效
            if (_currentTarget) {
                if (!isTargetValid(_currentTarget)) {
                    _currentTarget = nullptr;
                    _state = UnitState::IDLE;
                    break;
                }
                
                Vec2 myPos = this->getPosition();
                float distance = myPos.distance(_currentTarget->getPosition());
                if (distance <= _attackRange) {
                    // 进入攻击范围
                    stopMoving();
                    _state = UnitState::ATTACKING;
                }
            }
            break;
            
        case UnitState::ATTACKING:
            // 攻击状态
            if (!_currentTarget || !isTargetValid(_currentTarget)) {
                // 目标丢失或无效，回到待机
                _currentTarget = nullptr;
                _state = UnitState::IDLE;
                break;
            }
            
            // 更新攻击冷却
            if (_attackCooldown > 0) {
                _attackCooldown -= dt;
            } else {
                // 可以攻击
                if (_currentTarget && isTargetValid(_currentTarget)) {
                    attack(_currentTarget);
                    _attackCooldown = 1.0f / _attackSpeed;
                }
            }
            break;
            
        case UnitState::DEAD:
            // 死亡状态，不做任何事
            break;
    }
}

// ==================== 帧更新 ====================

void Unit::update(float dt) {
    // 验证自身状态 - 优先检查以避免无效处理
    if (_state == UnitState::DEAD || !this->getParent()) return;
    
    updateStateMachine(dt);
}
