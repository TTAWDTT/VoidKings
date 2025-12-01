// Unit.h
// 兵种/单位类
// 功能: 游戏中所有可移动单位的基类

#ifndef __UNIT_H__
#define __UNIT_H__

#include "cocos2d.h"
#include "../Core/GameDefines.h"
#include <vector>

USING_NS_CC;

class Building;

/**
 * @class Unit
 * @brief 兵种/单位基类
 * 
 * 兵种基础属性:
 * - 血量
 * - 攻击力
 * - 移速(地面/空中)
 * - 攻击距离(近程/远程)
 * - 类型(空/陆)
 * - 索敌优先级
 * 
 * 养成/战斗:
 * - 训练消耗(资源/人口/时间)
 * - 模型/动画/音效
 * - 死亡后掉落资源
 * - 特殊技能
 */
class Unit : public Node {
public:
    // 创建单位
    static Unit* create(UnitType type, Faction faction = Faction::PLAYER);
    
    // 初始化
    virtual bool init() override;
    virtual bool initWithType(UnitType type, Faction faction);
    
    // ==================== 属性获取接口 ====================
    
    // 获取单位类型
    UnitType getUnitType() const { return _unitType; }
    
    // 获取阵营
    Faction getFaction() const { return _faction; }
    
    // 获取状态
    UnitState getState() const { return _state; }
    
    // 获取当前血量
    int getCurrentHP() const { return _currentHP; }
    
    // 获取最大血量
    int getMaxHP() const { return _maxHP; }
    
    // 获取攻击力
    int getDamage() const { return _damage; }
    
    // 获取攻击范围
    float getAttackRange() const { return _attackRange; }
    
    // 获取移动速度
    float getMoveSpeed() const { return _moveSpeed; }
    
    // 获取人口占用
    int getPopulation() const { return _population; }
    
    // 获取目标优先级
    TargetPriority getTargetPriority() const { return _targetPriority; }
    
    // 是否空中单位
    bool isAirUnit() const { return _isAirUnit; }
    
    // 是否死亡
    bool isDead() const { return _state == UnitState::DEAD; }
    
    // 获取单位名称
    const std::string& getName() const { return _unitName; }
    
    // ==================== 战斗接口 ====================
    
    /**
     * @brief 受到伤害
     * @param damage 伤害值
     * @return 剩余血量
     */
    virtual int takeDamage(int damage);
    
    /**
     * @brief 治疗
     * @param amount 治疗量
     */
    virtual void heal(int amount);
    
    /**
     * @brief 设置建筑列表(用于索敌)
     * @param buildings 建筑列表
     */
    void setBuildingList(const std::vector<Building*>& buildings);
    
    /**
     * @brief 设置敌方单位列表
     * @param enemies 敌方单位列表
     */
    void setEnemyList(const std::vector<Unit*>& enemies);
    
    /**
     * @brief 索敌 - 查找攻击目标
     * @return 找到的目标节点
     */
    virtual Node* findTarget();
    
    /**
     * @brief 移动到目标位置
     * @param target 目标位置
     */
    virtual void moveTo(const Vec2& target);
    
    /**
     * @brief 攻击目标
     * @param target 攻击目标
     */
    virtual void attack(Node* target);
    
    /**
     * @brief 停止移动
     */
    virtual void stopMoving();
    
    /**
     * @brief 部署单位到指定位置
     * @param position 部署位置
     */
    virtual void deploy(const Vec2& position);
    
    // ==================== 更新接口 ====================
    
    // 帧更新
    virtual void update(float dt) override;
    
    // 选中/取消选中
    virtual void setSelected(bool selected);
    bool isSelected() const { return _isSelected; }

protected:
    Unit();
    virtual ~Unit();
    
    // 初始化精灵
    virtual void initSprite();
    
    // 初始化单位属性
    virtual void initAttributes();
    
    // 创建血条
    virtual void createHealthBar();
    
    // 更新血条
    virtual void updateHealthBar();
    
    // 状态机更新
    virtual void updateStateMachine(float dt);
    
    // 播放移动动画
    virtual void playMoveAnimation();
    
    // 播放攻击动画
    virtual void playAttackAnimation();
    
    // 播放死亡动画
    virtual void playDeathAnimation();
    
    // 播放待机动画
    virtual void playIdleAnimation();
    
    // 死亡处理
    virtual void onDead();
    
    // 计算到目标的路径
    virtual std::vector<Vec2> findPath(const Vec2& target);
    
    // 沿路径移动
    virtual void moveAlongPath();
    
    // ==================== 成员变量 ====================
    
    // 基础属性
    UnitType _unitType;              // 单位类型
    Faction _faction;                // 阵营
    UnitState _state;                // 状态
    std::string _unitName;           // 单位名称
    
    // 生命属性
    int _currentHP;                  // 当前血量
    int _maxHP;                      // 最大血量
    
    // 攻击属性
    int _damage;                     // 攻击力
    float _attackRange;              // 攻击范围
    float _attackSpeed;              // 攻击速度(每秒攻击次数)
    float _attackCooldown;           // 攻击冷却
    
    // 移动属性
    float _moveSpeed;                // 移动速度
    bool _isAirUnit;                 // 是否空中单位
    
    // 索敌属性
    TargetPriority _targetPriority;  // 目标优先级
    Node* _currentTarget;            // 当前目标
    
    // 人口属性
    int _population;                 // 人口占用
    
    // 路径属性
    std::vector<Vec2> _path;         // 移动路径
    int _pathIndex;                  // 当前路径点索引
    Vec2 _targetPosition;            // 目标位置
    
    // 战场引用
    std::vector<Building*> _buildings; // 可攻击的建筑
    std::vector<Unit*> _enemies;       // 敌方单位
    
    // UI组件
    Sprite* _sprite;                 // 单位精灵
    Node* _healthBar;                // 血条
    DrawNode* _selectionIndicator;   // 选中指示器
    
    // 状态标记
    bool _isSelected;                // 是否被选中
    bool _isMoving;                  // 是否在移动
};

#endif // __UNIT_H__
