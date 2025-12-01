// StorageBuilding.h
// 存储建筑类
// 功能: 资源存储建筑，管理资源容量

#ifndef __STORAGE_BUILDING_H__
#define __STORAGE_BUILDING_H__

#include "Building.h"

/**
 * @class StorageBuilding
 * @brief 存储建筑类
 * 
 * 存储建筑特有属性:
 * - 各资源最大容量
 * - 死亡后资源损失比例
 * - 容量升级规则(等级对应扩容)
 */
class StorageBuilding : public Building {
public:
    // 创建存储建筑
    static StorageBuilding* create(BuildingType type, Faction faction = Faction::PLAYER);
    
    // 初始化
    virtual bool initWithType(BuildingType type, Faction faction) override;
    
    // ==================== 存储属性获取 ====================
    
    // 获取存储的资源类型
    ResourceType getStorageType() const { return _storageType; }
    
    // 获取存储容量
    int getCapacity() const { return _capacity; }
    
    // 获取死亡损失比例
    float getLossRate() const { return _lossRateOnDeath; }
    
    // ==================== 操作接口 ====================
    
    /**
     * @brief 完成建造时增加全局存储容量
     */
    virtual void finishBuilding() override;
    
    /**
     * @brief 完成升级时增加全局存储容量
     */
    virtual void finishUpgrade() override;
    
    // 受到伤害时处理
    virtual int takeDamage(int damage) override;

protected:
    StorageBuilding();
    virtual ~StorageBuilding();
    
    // 初始化存储属性
    virtual void initStorageAttributes();
    
    // 更新全局存储容量
    void updateGlobalCapacity();
    
    // ==================== 存储属性 ====================
    
    ResourceType _storageType;       // 存储资源类型
    int _capacity;                   // 存储容量
    int _previousCapacity;           // 升级前的容量(用于计算增量)
    float _lossRateOnDeath;          // 死亡时资源损失比例
};

#endif // __STORAGE_BUILDING_H__
