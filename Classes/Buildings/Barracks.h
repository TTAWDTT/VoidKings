// Barracks.h
// 兵营类
// 功能: 训练兵种，管理兵种队列

#ifndef __BARRACKS_H__
#define __BARRACKS_H__

#include "Building.h"
#include <queue>
#include <vector>

/**
 * @class Barracks
 * @brief 兵营类
 * 
 * 兵营特有属性:
 * - 可训练兵种列表(等级解锁新兵种)
 * - 训练单兵种耗时/消耗资源/占用人口
 * - 已训练兵种存储上限(人口限制)
 * - 兵种销毁返还资源比例
 * - 显示当前已训兵种(数量/属性)
 */
class Barracks : public Building {
public:
    // 创建兵营
    static Barracks* create(Faction faction = Faction::PLAYER);
    
    // 初始化
    virtual bool initWithType(BuildingType type, Faction faction) override;
    
    // ==================== 训练接口 ====================
    
    /**
     * @brief 获取可训练的兵种列表
     * @return 可训练兵种类型列表
     */
    std::vector<UnitType> getAvailableUnits() const;
    
    /**
     * @brief 训练兵种
     * @param type 兵种类型
     * @return 是否成功加入训练队列
     */
    bool trainUnit(UnitType type);
    
    /**
     * @brief 取消训练
     * @param index 队列中的索引
     * @return 是否成功取消
     */
    bool cancelTraining(int index);
    
    /**
     * @brief 获取训练队列
     * @return 训练队列
     */
    const std::vector<UnitType>& getTrainingQueue() const { return _trainingQueue; }
    
    /**
     * @brief 获取当前训练进度
     * @return 训练进度(0-1)
     */
    float getTrainingProgress() const { return _trainingProgress; }
    
    /**
     * @brief 获取已训练完成的兵种
     * @return 已完成兵种列表
     */
    const std::vector<UnitType>& getTrainedUnits() const { return _trainedUnits; }
    
    /**
     * @brief 取出一个已训练的兵种
     * @param type 兵种类型
     * @return 是否成功取出
     */
    bool deployUnit(UnitType type);
    
    /**
     * @brief 获取队列容量
     */
    int getQueueCapacity() const { return _queueCapacity; }
    
    /**
     * @brief 获取当前队列大小
     */
    int getCurrentQueueSize() const { return (int)_trainingQueue.size(); }
    
    // 帧更新
    virtual void update(float dt) override;

protected:
    Barracks();
    virtual ~Barracks();
    
    // 初始化兵营属性
    virtual void initBarracksAttributes();
    
    // 获取兵种训练时间
    int getTrainTime(UnitType type) const;
    
    // 获取兵种训练消耗
    std::pair<int, int> getTrainCost(UnitType type) const;
    
    // 获取兵种人口占用
    int getUnitPopulation(UnitType type) const;
    
    // 完成当前训练
    void completeTraining();
    
    // ==================== 兵营属性 ====================
    
    std::vector<UnitType> _availableUnits;    // 可训练兵种
    std::vector<UnitType> _trainingQueue;     // 训练队列
    std::vector<UnitType> _trainedUnits;      // 已训练完成的兵种
    int _queueCapacity;                       // 队列容量
    float _trainingProgress;                  // 当前训练进度
    float _trainingTimer;                     // 训练计时器
    float _currentTrainTime;                  // 当前训练所需时间
};

#endif // __BARRACKS_H__
