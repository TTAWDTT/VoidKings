// Potion.cpp
// 药水/法术类实现文件
// 功能: 实现各种药水和法术效果

#include "Potion.h"
#include "../Units/Unit.h"
#include "../Buildings/Building.h"
#include "../Core/ResourceManager.h"

// ==================== 创建和初始化 ====================

Potion* Potion::create(PotionType type, Faction faction) {
    Potion* potion = new (std::nothrow) Potion();
    if (potion && potion->initWithType(type, faction)) {
        potion->autorelease();
        return potion;
    }
    CC_SAFE_DELETE(potion);
    return nullptr;
}

Potion::Potion()
    : _potionType(PotionType::HEAL)
    , _faction(Faction::PLAYER)
    , _potionName("Potion")
    , _radius(100.0f)
    , _duration(5.0f)
    , _effectValue(50.0f)
    , _elapsedTime(0)
    , _isActive(false)
    , _goldCost(0)
    , _elixirCost(100)
    , _population(1)
    , _rangeIndicator(nullptr)
    , _effectNode(nullptr)
{
}

Potion::~Potion() {
    _friendlyUnits.clear();
    _enemyUnits.clear();
    _enemyBuildings.clear();
}

bool Potion::init() {
    if (!Node::init()) {
        return false;
    }
    return true;
}

bool Potion::initWithType(PotionType type, Faction faction) {
    if (!init()) {
        return false;
    }
    
    _potionType = type;
    _faction = faction;
    
    // 初始化属性
    initPotionAttributes();
    
    return true;
}

// ==================== 属性初始化 ====================

void Potion::initPotionAttributes() {
    switch (_potionType) {
        case PotionType::HEAL:
            _potionName = "Heal Spell";
            _radius = 120.0f;
            _duration = 8.0f;
            _effectValue = 30.0f;  // 每秒治疗量
            _goldCost = 0;
            _elixirCost = 1000;
            _population = 2;
            break;
            
        case PotionType::RAGE:
            _potionName = "Rage Spell";
            _radius = 100.0f;
            _duration = 10.0f;
            _effectValue = 1.5f;  // 伤害加成倍率
            _goldCost = 0;
            _elixirCost = 1500;
            _population = 2;
            break;
            
        case PotionType::LIGHTNING:
            _potionName = "Lightning Spell";
            _radius = 80.0f;
            _duration = 0;  // 即时效果
            _effectValue = 300.0f;  // 伤害值
            _goldCost = 0;
            _elixirCost = 2000;
            _population = 2;
            break;
            
        case PotionType::FREEZE:
            _potionName = "Freeze Spell";
            _radius = 90.0f;
            _duration = 5.0f;
            _effectValue = 0;  // 冰冻效果
            _goldCost = 0;
            _elixirCost = 2500;
            _population = 1;
            break;
            
        default:
            break;
    }
}

// ==================== 使用接口 ====================

void Potion::use(const Vec2& position) {
    this->setPosition(position);
    _isActive = true;
    _elapsedTime = 0;
    
    // 创建视觉效果
    createVisualEffect();
    
    // 应用即时效果(对于闪电等即时生效的法术)
    if (_potionType == PotionType::LIGHTNING) {
        applyEffect(0);  // 即时效果不需要dt
    }
    
    // 如果是持续效果，启用更新
    if (_duration > 0) {
        scheduleUpdate();
    } else {
        // 即时效果，播放特效后移除
        auto delay = DelayTime::create(0.5f);
        auto remove = RemoveSelf::create();
        this->runAction(Sequence::create(delay, remove, nullptr));
    }
}

void Potion::setFriendlyUnits(const std::vector<Unit*>& units) {
    _friendlyUnits = units;
}

void Potion::setEnemyUnits(const std::vector<Unit*>& enemies) {
    _enemyUnits = enemies;
}

void Potion::setEnemyBuildings(const std::vector<Building*>& buildings) {
    _enemyBuildings = buildings;
}

// ==================== 视觉效果 ====================

void Potion::createVisualEffect() {
    // 创建范围指示器
    _rangeIndicator = DrawNode::create();
    
    Color4F color;
    switch (_potionType) {
        case PotionType::HEAL:
            color = Color4F(0.2f, 1.0f, 0.2f, 0.3f);
            break;
        case PotionType::RAGE:
            color = Color4F(1.0f, 0.2f, 0.8f, 0.3f);
            break;
        case PotionType::LIGHTNING:
            color = Color4F(1.0f, 1.0f, 0.2f, 0.5f);
            break;
        case PotionType::FREEZE:
            color = Color4F(0.2f, 0.6f, 1.0f, 0.3f);
            break;
        default:
            color = Color4F(1.0f, 1.0f, 1.0f, 0.3f);
            break;
    }
    
    _rangeIndicator->drawSolidCircle(Vec2::ZERO, _radius, 0, 40, color);
    _rangeIndicator->drawCircle(Vec2::ZERO, _radius, 0, 40, false, 
                                Color4F(color.r, color.g, color.b, 0.8f));
    
    this->addChild(_rangeIndicator, 0);
    
    // 创建特效节点
    _effectNode = Node::create();
    this->addChild(_effectNode, 1);
    
    // 根据药水类型添加特殊效果
    switch (_potionType) {
        case PotionType::HEAL:
            // 治疗光环：向上飘动的粒子效果
            for (int i = 0; i < 10; i++) {
                auto particle = DrawNode::create();
                particle->drawSolidCircle(Vec2::ZERO, 3, 0, 8, Color4F(0.2f, 1.0f, 0.2f, 0.8f));
                
                float angle = (float)i / 10 * M_PI * 2;
                float dist = _radius * 0.5f;
                particle->setPosition(Vec2(cos(angle) * dist, sin(angle) * dist));
                
                // 动画
                auto moveUp = MoveBy::create(1.0f, Vec2(0, 30));
                auto fade = FadeOut::create(1.0f);
                auto spawn = Spawn::create(moveUp, fade, nullptr);
                auto reset = Place::create(Vec2(cos(angle) * dist, sin(angle) * dist));
                auto fadeIn = FadeIn::create(0.1f);
                auto seq = Sequence::create(spawn, reset, fadeIn, nullptr);
                particle->runAction(RepeatForever::create(seq));
                
                _effectNode->addChild(particle);
            }
            break;
            
        case PotionType::RAGE:
            // 狂暴光环：脉冲效果
            {
                auto pulse = ScaleTo::create(0.5f, 1.1f);
                auto pulseBack = ScaleTo::create(0.5f, 1.0f);
                auto seq = Sequence::create(pulse, pulseBack, nullptr);
                _rangeIndicator->runAction(RepeatForever::create(seq));
            }
            break;
            
        case PotionType::LIGHTNING:
            // 闪电效果：闪烁的线条
            for (int i = 0; i < 5; i++) {
                auto lightning = DrawNode::create();
                float startX = (rand() % 100 - 50);
                float startY = _radius;
                
                Vec2 points[5];
                points[0] = Vec2(startX, startY);
                for (int j = 1; j < 5; j++) {
                    points[j] = Vec2(startX + (rand() % 40 - 20), startY - j * _radius / 2);
                }
                
                for (int j = 0; j < 4; j++) {
                    lightning->drawLine(points[j], points[j+1], Color4F(1, 1, 0.5f, 1));
                }
                
                _effectNode->addChild(lightning);
            }
            break;
            
        case PotionType::FREEZE:
            // 冰冻效果：蓝色结晶
            for (int i = 0; i < 8; i++) {
                auto crystal = DrawNode::create();
                float angle = (float)i / 8 * M_PI * 2;
                float dist = _radius * 0.6f;
                
                // 绘制六边形结晶
                Vec2 verts[6];
                for (int j = 0; j < 6; j++) {
                    float a = (float)j / 6 * M_PI * 2;
                    verts[j] = Vec2(cos(a) * 8, sin(a) * 8);
                }
                crystal->drawPolygon(verts, 6, Color4F(0.5f, 0.8f, 1.0f, 0.5f), 1, Color4F(0.8f, 0.9f, 1.0f, 1.0f));
                crystal->setPosition(Vec2(cos(angle) * dist, sin(angle) * dist));
                
                _effectNode->addChild(crystal);
            }
            break;
            
        default:
            break;
    }
}

// ==================== 效果应用 ====================

void Potion::applyEffect(float dt) {
    switch (_potionType) {
        case PotionType::HEAL:
            applyHealEffect(dt);
            break;
        case PotionType::RAGE:
            applyRageEffect(dt);
            break;
        case PotionType::LIGHTNING:
            applyLightningEffect();
            break;
        case PotionType::FREEZE:
            applyFreezeEffect();
            break;
        default:
            break;
    }
}

void Potion::applyHealEffect(float dt) {
    Vec2 myPos = this->getPosition();
    
    for (auto unit : _friendlyUnits) {
        if (!unit || unit->isDead()) continue;
        
        float distance = myPos.distance(unit->getPosition());
        if (distance <= _radius) {
            // 在范围内，进行治疗 - 使用dt实现帧率无关的治疗
            int healAmount = (int)(_effectValue * dt);
            if (healAmount < 1) healAmount = 1;  // 确保至少治疗1点
            unit->heal(healAmount);
        }
    }
}

void Potion::applyRageEffect(float dt) {
    // 狂暴效果：在update中持续检测范围内单位
    // 实际实现需要给单位添加buff系统
    Vec2 myPos = this->getPosition();
    
    for (auto unit : _friendlyUnits) {
        if (!unit || unit->isDead()) continue;
        
        float distance = myPos.distance(unit->getPosition());
        if (distance <= _radius) {
            // 在范围内，添加狂暴效果
            // 这里简化处理，实际应该通过buff系统
            // unit->addBuff(BuffType::RAGE, _effectValue, _duration);
        }
    }
}

void Potion::applyLightningEffect() {
    Vec2 myPos = this->getPosition();
    
    // 对范围内的敌方单位和建筑造成伤害
    for (auto unit : _enemyUnits) {
        if (!unit || unit->isDead()) continue;
        
        float distance = myPos.distance(unit->getPosition());
        if (distance <= _radius) {
            unit->takeDamage((int)_effectValue);
        }
    }
    
    for (auto building : _enemyBuildings) {
        if (!building || building->isDestroyed()) continue;
        
        float distance = myPos.distance(building->getPosition());
        if (distance <= _radius) {
            building->takeDamage((int)_effectValue);
        }
    }
}

void Potion::applyFreezeEffect() {
    // 冰冻效果：停止范围内敌人的行动
    // 实际实现需要给单位和建筑添加冰冻状态
    Vec2 myPos = this->getPosition();
    
    for (auto unit : _enemyUnits) {
        if (!unit || unit->isDead()) continue;
        
        float distance = myPos.distance(unit->getPosition());
        if (distance <= _radius) {
            // 添加冰冻效果
            // unit->freeze(_duration);
        }
    }
}

void Potion::onEffectEnd() {
    // 效果结束，移除节点
    auto fade = FadeOut::create(0.5f);
    auto remove = RemoveSelf::create();
    this->runAction(Sequence::create(fade, remove, nullptr));
}

// ==================== 帧更新 ====================

void Potion::update(float dt) {
    if (!_isActive) return;
    
    _elapsedTime += dt;
    
    // 检查持续时间
    if (_elapsedTime >= _duration) {
        _isActive = false;
        onEffectEnd();
        return;
    }
    
    // 持续应用效果
    if (_potionType == PotionType::HEAL || _potionType == PotionType::RAGE) {
        applyEffect(dt);
    }
}
