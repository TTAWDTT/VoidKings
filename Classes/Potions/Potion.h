// Potion.h
// 药水/法术类
// 功能: 实现各种药水和法术效果

#ifndef __POTION_H__
#define __POTION_H__

#include "cocos2d.h"
#include "../Core/GameDefines.h"
#include <vector>

USING_NS_CC;

class Unit;
class Building;

/**
 * @class Potion
 * @brief 药水/法术类
 * 
 * 药水属性:
 * - 生效范围(圆形半径/矩形区域)
 * - 生效作用(伤害/治疗/增益/减益)
 * - 持续时间(即时生效/持续生效)
 * - 价格/人口
 * - 特效/音效
 * - 作用目标(己方/敌方/兵种/建筑)
 */
class Potion : public Node {
public:
    // 创建药水
    static Potion* create(PotionType type, Faction faction = Faction::PLAYER);
    
    // 初始化
    virtual bool init() override;
    virtual bool initWithType(PotionType type, Faction faction);
    
    // ==================== 属性获取 ====================
    
    PotionType getPotionType() const { return _potionType; }
    float getRadius() const { return _radius; }
    float getDuration() const { return _duration; }
    float getEffectValue() const { return _effectValue; }
    int getGoldCost() const { return _goldCost; }
    int getElixirCost() const { return _elixirCost; }
    int getPopulation() const { return _population; }
    const std::string& getName() const { return _potionName; }
    
    // ==================== 操作接口 ====================
    
    /**
     * @brief 使用药水
     * @param position 使用位置
     */
    void use(const Vec2& position);
    
    /**
     * @brief 设置友军单位列表
     */
    void setFriendlyUnits(const std::vector<Unit*>& units);
    
    /**
     * @brief 设置敌方单位列表
     */
    void setEnemyUnits(const std::vector<Unit*>& enemies);
    
    /**
     * @brief 设置敌方建筑列表
     */
    void setEnemyBuildings(const std::vector<Building*>& buildings);
    
    // 帧更新
    virtual void update(float dt) override;

protected:
    Potion();
    virtual ~Potion();
    
    // 初始化药水属性
    virtual void initPotionAttributes();
    
    // 创建视觉效果
    virtual void createVisualEffect();
    
    // 应用效果
    virtual void applyEffect();
    
    // 治疗效果
    virtual void applyHealEffect();
    
    // 狂暴效果
    virtual void applyRageEffect();
    
    // 闪电效果
    virtual void applyLightningEffect();
    
    // 冰冻效果
    virtual void applyFreezeEffect();
    
    // 效果结束
    virtual void onEffectEnd();
    
    // ==================== 成员变量 ====================
    
    PotionType _potionType;          // 药水类型
    Faction _faction;                // 使用方阵营
    std::string _potionName;         // 药水名称
    
    float _radius;                   // 作用范围
    float _duration;                 // 持续时间
    float _effectValue;              // 效果数值
    float _elapsedTime;              // 已经过时间
    bool _isActive;                  // 是否激活
    
    int _goldCost;                   // 金币消耗
    int _elixirCost;                 // 圣水消耗
    int _population;                 // 人口占用
    
    std::vector<Unit*> _friendlyUnits;      // 友军单位
    std::vector<Unit*> _enemyUnits;         // 敌方单位
    std::vector<Building*> _enemyBuildings; // 敌方建筑
    
    // UI组件
    DrawNode* _rangeIndicator;       // 范围指示器
    Node* _effectNode;               // 特效节点
};

#endif // __POTION_H__
