// ResourceManager.h
// 资源管理器 - 单例模式
// 功能: 管理游戏中所有资源的生产、存储、消耗、损失和获得
// 实现: 单例模式，绑定UI实时更新

#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "cocos2d.h"
#include "GameDefines.h"
#include <functional>
#include <vector>

USING_NS_CC;

// 资源变化回调函数类型
typedef std::function<void(ResourceType, int, int)> ResourceCallback;

/**
 * @class ResourceManager
 * @brief 资源管理器单例类
 * 
 * 负责管理游戏中所有资源的:
 * - 生产: 生产建筑定时产出
 * - 存储: 存储建筑/生产建筑上限管理
 * - 消耗: 建造/升级/训练/药水制作
 * - 损失: 建筑被拆/战斗失败扣除
 * - 获得: 生产/战斗掠夺/任务奖励
 */
class ResourceManager {
public:
    // 获取单例实例
    static ResourceManager* getInstance();
    
    // 销毁单例
    static void destroyInstance();
    
    // ==================== 资源操作接口 ====================
    
    /**
     * @brief 获取指定类型资源的当前数量
     * @param type 资源类型
     * @return 当前资源数量
     */
    int getResource(ResourceType type) const;
    
    /**
     * @brief 获取指定类型资源的最大存储量
     * @param type 资源类型
     * @return 最大存储量
     */
    int getMaxResource(ResourceType type) const;
    
    /**
     * @brief 设置资源数量
     * @param type 资源类型
     * @param amount 数量
     */
    void setResource(ResourceType type, int amount);
    
    /**
     * @brief 设置资源最大存储量
     * @param type 资源类型
     * @param maxAmount 最大数量
     */
    void setMaxResource(ResourceType type, int maxAmount);
    
    /**
     * @brief 增加资源
     * @param type 资源类型
     * @param amount 增加数量
     * @return 实际增加的数量(受存储上限限制)
     */
    int addResource(ResourceType type, int amount);
    
    /**
     * @brief 消耗资源
     * @param type 资源类型
     * @param amount 消耗数量
     * @return 是否消耗成功
     */
    bool consumeResource(ResourceType type, int amount);
    
    /**
     * @brief 检查是否有足够的资源
     * @param type 资源类型
     * @param amount 需要的数量
     * @return 是否足够
     */
    bool hasEnoughResource(ResourceType type, int amount) const;
    
    /**
     * @brief 检查多种资源是否足够
     * @param gold 金币数量
     * @param elixir 圣水数量
     * @return 是否足够
     */
    bool hasEnoughResources(int gold, int elixir) const;
    
    /**
     * @brief 消耗多种资源
     * @param gold 金币数量
     * @param elixir 圣水数量
     * @return 是否消耗成功
     */
    bool consumeResources(int gold, int elixir);
    
    // ==================== 人口管理 ====================
    
    /**
     * @brief 获取当前使用的人口
     * @return 当前人口
     */
    int getCurrentPopulation() const;
    
    /**
     * @brief 获取最大人口上限
     * @return 最大人口
     */
    int getMaxPopulation() const;
    
    /**
     * @brief 增加最大人口上限
     * @param amount 增加数量
     */
    void addMaxPopulation(int amount);
    
    /**
     * @brief 使用人口
     * @param amount 使用数量
     * @return 是否成功
     */
    bool usePopulation(int amount);
    
    /**
     * @brief 释放人口
     * @param amount 释放数量
     */
    void releasePopulation(int amount);
    
    // ==================== 工人管理 ====================
    
    /**
     * @brief 获取可用工人数量
     * @return 可用工人数
     */
    int getAvailableWorkers() const;
    
    /**
     * @brief 获取总工人数量
     * @return 总工人数
     */
    int getTotalWorkers() const;
    
    /**
     * @brief 使用工人
     * @return 是否成功
     */
    bool useWorker();
    
    /**
     * @brief 释放工人
     */
    void releaseWorker();
    
    /**
     * @brief 增加工人
     */
    void addWorker();
    
    // ==================== 回调管理 ====================
    
    /**
     * @brief 注册资源变化回调
     * @param callback 回调函数
     */
    void registerCallback(ResourceCallback callback);
    
    /**
     * @brief 清除所有回调
     */
    void clearCallbacks();
    
    // ==================== 数据操作 ====================
    
    /**
     * @brief 获取所有资源数据
     * @return 资源数据结构
     */
    ResourceData getAllResources() const;
    
    /**
     * @brief 重置所有资源为初始值
     */
    void reset();
    
    /**
     * @brief 初始化默认资源
     */
    void initDefaultResources();

private:
    // 私有构造函数(单例模式)
    ResourceManager();
    ~ResourceManager();
    
    // 禁止拷贝和赋值
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    // 通知资源变化
    void notifyResourceChange(ResourceType type, int oldValue, int newValue);
    
    // 单例实例
    static ResourceManager* _instance;
    
    // 资源存储
    std::map<ResourceType, int> _resources;
    std::map<ResourceType, int> _maxResources;
    
    // 人口数据
    int _currentPopulation;
    int _maxPopulation;
    
    // 工人数据
    int _totalWorkers;
    int _busyWorkers;
    
    // 回调列表
    std::vector<ResourceCallback> _callbacks;
};

#endif // __RESOURCE_MANAGER_H__
