// ProductionBuilding.h
// 生产建筑类
// 功能: 资源生产建筑，定时产出金币或圣水

#ifndef __PRODUCTION_BUILDING_H__
#define __PRODUCTION_BUILDING_H__

#include "Building.h"

/**
 * @class ProductionBuilding
 * @brief 生产建筑类
 * 
 * 生产建筑特有属性:
 * - 资源加成定时器(生产间隔)
 * - 生产资源类型(金币/圣水)
 * - 生产效率(等级对应产量)
 * - 死亡后资源损失比例
 * - 最大存储量(若为资源生产+存储二合一建筑)
 */
class ProductionBuilding : public Building {
public:
    // 创建生产建筑
    static ProductionBuilding* create(BuildingType type, Faction faction = Faction::PLAYER);
    
    // 初始化
    virtual bool initWithType(BuildingType type, Faction faction) override;
    
    // ==================== 生产属性获取 ====================
    
    // 获取生产的资源类型
    ResourceType getProductionType() const { return _productionType; }
    
    // 获取生产速率(每秒产量)
    float getProductionRate() const { return _productionRate; }
    
    // 获取当前储量
    int getCurrentStorage() const { return _currentStorage; }
    
    // 获取最大储量
    int getMaxStorage() const { return _maxStorage; }
    
    // 获取储量填充比例
    float getStorageRatio() const { return (float)_currentStorage / (float)_maxStorage; }
    
    // ==================== 操作接口 ====================
    
    /**
     * @brief 收集资源
     * @return 收集到的资源数量
     */
    int collectResource();
    
    /**
     * @brief 是否可以收集(有资源可收集)
     */
    bool canCollect() const { return _currentStorage > 0; }
    
    // 帧更新
    virtual void update(float dt) override;
    
    // 受到伤害时处理资源损失
    virtual int takeDamage(int damage) override;

protected:
    ProductionBuilding();
    virtual ~ProductionBuilding();
    
    // 初始化生产属性
    virtual void initProductionAttributes();
    
    // 创建资源显示
    virtual void createResourceIndicator();
    
    // 更新资源显示
    virtual void updateResourceIndicator();
    
    // ==================== 生产属性 ====================
    
    ResourceType _productionType;    // 生产资源类型
    float _productionRate;           // 生产速率(每秒)
    float _productionTimer;          // 生产计时器
    int _currentStorage;             // 当前储量
    int _maxStorage;                 // 最大储量
    float _lossRateOnDeath;          // 死亡时资源损失比例
    
    // UI组件
    Node* _resourceIndicator;        // 资源指示器
    Label* _resourceLabel;           // 资源数量标签
};

#endif // __PRODUCTION_BUILDING_H__
