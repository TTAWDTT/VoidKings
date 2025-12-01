// DefenseBuilding.h
// 防御建筑类
// 功能: 防御建筑的基类，负责基地防御
// 包含攻击范围、攻击对象、攻速、索敌组件等属性

#ifndef __DEFENSE_BUILDING_H__
#define __DEFENSE_BUILDING_H__

#include "Building.h"
#include <vector>

class Unit;
class Projectile;

/**
 * @class DefenseBuilding
 * @brief 防御建筑类
 * 
 * 防御建筑特有属性:
 * - 攻击范围(数值+可视化显示)
 * - 攻击对象(优先目标)
 * - 攻速(攻击间隔)
 * - 索敌组件(优先攻击距离/血量)
 * - 是否可穿墙
 * - 伤害类型(单体/范围)
 * - 子弹系统
 */
class DefenseBuilding : public Building {
public:
    // 创建防御建筑
    static DefenseBuilding* create(BuildingType type, Faction faction = Faction::PLAYER);
    
    // 初始化
    virtual bool initWithType(BuildingType type, Faction faction) override;
    
    // ==================== 防御属性获取 ====================
    
    // 获取攻击范围
    float getAttackRange() const { return _attackRange; }
    
    // 获取攻击力
    int getDamage() const { return _damage; }
    
    // 获取攻击速度(每秒攻击次数)
    float getAttackSpeed() const { return _attackSpeed; }
    
    // 获取伤害类型
    DamageType getDamageType() const { return _damageType; }
    
    // 获取伤害范围(范围伤害时使用)
    float getDamageRadius() const { return _damageRadius; }
    
    // 获取弹道类型
    ProjectileType getProjectileType() const { return _projectileType; }
    
    // 获取目标优先级
    TargetPriority getTargetPriority() const { return _targetPriority; }
    
    // 是否可以攻击空中单位
    bool canAttackAir() const { return _canAttackAir; }
    
    // 是否可以穿墙攻击
    bool canAttackOverWall() const { return _canAttackOverWall; }
    
    // ==================== 防御属性设置 ====================
    
    // 设置攻击范围
    void setAttackRange(float range) { _attackRange = range; }
    
    // 设置攻击力
    void setDamage(int damage) { _damage = damage; }
    
    // 设置攻击速度
    void setAttackSpeed(float speed) { _attackSpeed = speed; }
    
    // 设置目标优先级
    void setTargetPriority(TargetPriority priority) { _targetPriority = priority; }
    
    // ==================== 战斗接口 ====================
    
    /**
     * @brief 设置可攻击的敌人列表
     * @param enemies 敌人单位列表
     */
    void setEnemyList(const std::vector<Unit*>& enemies);
    
    /**
     * @brief 索敌 - 查找攻击目标
     * @return 找到的目标，如果没有返回nullptr
     */
    Unit* findTarget();
    
    /**
     * @brief 攻击当前目标
     */
    void attackTarget();
    
    /**
     * @brief 获取当前目标
     */
    Unit* getCurrentTarget() const { return _currentTarget; }
    
    /**
     * @brief 清除当前目标
     */
    void clearTarget();
    
    // ==================== 显示接口 ====================
    
    /**
     * @brief 显示/隐藏攻击范围
     * @param show 是否显示
     */
    virtual void showRange(bool show) override;
    
    /**
     * @brief 旋转炮塔朝向目标
     * @param target 目标位置
     */
    void rotateTurretToTarget(const Vec2& target);
    
    // 帧更新
    virtual void update(float dt) override;

protected:
    DefenseBuilding();
    virtual ~DefenseBuilding();
    
    // 初始化防御属性
    virtual void initDefenseAttributes();
    
    // 创建攻击范围显示
    virtual void createRangeIndicator();
    
    // 创建子弹
    virtual Projectile* createProjectile();
    
    // 发射子弹
    virtual void fireProjectile(Unit* target);
    
    // 播放攻击动画
    virtual void playAttackAnimation();
    
    // ==================== 防御属性 ====================
    
    float _attackRange;              // 攻击范围
    int _damage;                     // 攻击力
    float _attackSpeed;              // 攻击速度(每秒攻击次数)
    float _attackCooldown;           // 攻击冷却计时器
    DamageType _damageType;          // 伤害类型
    float _damageRadius;             // 伤害范围(范围伤害时)
    ProjectileType _projectileType;  // 弹道类型
    TargetPriority _targetPriority;  // 目标优先级
    bool _canAttackAir;              // 是否可攻击空中单位
    bool _canAttackOverWall;         // 是否可穿墙攻击
    
    // 战斗状态
    Unit* _currentTarget;            // 当前攻击目标
    std::vector<Unit*> _enemies;     // 敌人列表
    
    // UI组件
    DrawNode* _rangeIndicator;       // 攻击范围指示器
    Sprite* _turretSprite;           // 炮塔精灵(可旋转)
};

#endif // __DEFENSE_BUILDING_H__
