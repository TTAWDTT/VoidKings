// Projectile.h
// 投射物/子弹类
// 功能: 防御建筑发射的投射物

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "cocos2d.h"
#include "../Core/GameDefines.h"

USING_NS_CC;

class Unit;

/**
 * @class Projectile
 * @brief 投射物类
 * 
 * 投射物属性:
 * - 模型/音效
 * - 弹道类型(直线/抛物线/跟踪)
 * - 移动速度
 * - 伤害数值
 * - 伤害范围(单体/半径)
 * - 伤害判定(命中后消失/穿透)
 */
class Projectile : public Node {
public:
    // 创建投射物
    static Projectile* create(ProjectileType type = ProjectileType::STRAIGHT);
    
    // 初始化
    virtual bool init() override;
    virtual bool initWithType(ProjectileType type);
    
    // ==================== 属性设置 ====================
    
    // 设置伤害
    void setDamage(int damage) { _damage = damage; }
    
    // 设置伤害类型
    void setDamageType(DamageType type) { _damageType = type; }
    
    // 设置伤害范围
    void setDamageRadius(float radius) { _damageRadius = radius; }
    
    // 设置目标
    void setTarget(Unit* target) { _target = target; }
    
    // 设置目标位置(用于固定目标位置的投射物)
    void setTargetPosition(const Vec2& pos) { _targetPosition = pos; }
    
    // 设置阵营
    void setFaction(Faction faction) { _faction = faction; }
    
    // 设置速度
    void setSpeed(float speed) { _speed = speed; }
    
    // ==================== 属性获取 ====================
    
    int getDamage() const { return _damage; }
    DamageType getDamageType() const { return _damageType; }
    float getDamageRadius() const { return _damageRadius; }
    float getSpeed() const { return _speed; }
    ProjectileType getProjectileType() const { return _projectileType; }
    
    // ==================== 操作接口 ====================
    
    /**
     * @brief 发射投射物
     */
    void launch();
    
    /**
     * @brief 设置敌人列表(范围伤害时使用)
     */
    void setEnemyList(const std::vector<Unit*>& enemies);
    
    // 帧更新
    virtual void update(float dt) override;

protected:
    Projectile();
    virtual ~Projectile();
    
    // 初始化精灵
    virtual void initSprite();
    
    // 移动逻辑
    virtual void moveToTarget(float dt);
    
    // 直线移动
    virtual void moveStraight(float dt);
    
    // 抛物线移动
    virtual void moveParabolic(float dt);
    
    // 追踪移动
    virtual void moveTracking(float dt);
    
    // 命中检测
    virtual bool checkHit();
    
    // 命中处理
    virtual void onHit();
    
    // 造成伤害
    virtual void dealDamage();
    
    // 播放命中特效
    virtual void playHitEffect();
    
    // ==================== 成员变量 ====================
    
    ProjectileType _projectileType;  // 弹道类型
    Faction _faction;                // 阵营
    
    int _damage;                     // 伤害值
    DamageType _damageType;          // 伤害类型
    float _damageRadius;             // 伤害范围
    
    float _speed;                    // 移动速度
    bool _isLaunched;                // 是否已发射
    
    Unit* _target;                   // 追踪目标
    Vec2 _targetPosition;            // 目标位置
    Vec2 _startPosition;             // 起始位置
    float _flightTime;               // 飞行时间
    float _totalFlightTime;          // 总飞行时间
    float _maxHeight;                // 抛物线最大高度
    
    std::vector<Unit*> _enemies;     // 敌人列表
    
    // UI组件
    Sprite* _sprite;                 // 投射物精灵
    ParticleSystem* _trail;          // 轨迹特效
};

#endif // __PROJECTILE_H__
