// Building.h
// 建筑基类
// 功能: 所有建筑的基础类，统一管理生命周期、UI绑定、升级系统等

#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include "../Core/GameDefines.h"
#include <functional>

USING_NS_CC;

// 建筑点击回调类型
typedef std::function<void(class Building*)> BuildingCallback;

/**
 * @class Building
 * @brief 建筑基类
 * 
 * 所有建筑的基础属性:
 * - 血量/耐久
 * - 占地格子数
 * - 建筑类型(己方/敌方)
 * - UI绑定(信息显示)
 * - 等级系统(升级解锁属性)
 * - 模型/动画/音效绑定
 * - 阵营归属
 * - 建造/升级耗时
 * - 拆除返还资源比例
 */
class Building : public Node {
public:
    // 创建建筑
    static Building* create(BuildingType type, Faction faction = Faction::PLAYER);
    
    // 初始化
    virtual bool init() override;
    virtual bool initWithType(BuildingType type, Faction faction);
    
    // ==================== 属性获取接口 ====================
    
    // 获取建筑类型
    BuildingType getBuildingType() const { return _buildingType; }
    
    // 获取阵营
    Faction getFaction() const { return _faction; }
    
    // 获取建筑状态
    BuildingState getState() const { return _state; }
    
    // 获取当前血量
    int getCurrentHP() const { return _currentHP; }
    
    // 获取最大血量
    int getMaxHP() const { return _maxHP; }
    
    // 获取当前等级
    int getLevel() const { return _level; }
    
    // 获取最大等级
    int getMaxLevel() const { return _maxLevel; }
    
    // 获取占地宽度(格子数)
    int getGridWidth() const { return _gridWidth; }
    
    // 获取占地高度(格子数)
    int getGridHeight() const { return _gridHeight; }
    
    // 获取格子位置
    Vec2 getGridPosition() const { return _gridPosition; }
    
    // 获取建筑名称
    const std::string& getName() const { return _buildingName; }
    
    // 是否被摧毁
    bool isDestroyed() const { return _state == BuildingState::DESTROYED; }
    
    // 是否在建造中
    bool isBuilding() const { return _state == BuildingState::BUILDING; }
    
    // 是否在升级中
    bool isUpgrading() const { return _state == BuildingState::UPGRADING; }
    
    // ==================== 属性设置接口 ====================
    
    // 设置格子位置
    void setGridPosition(const Vec2& gridPos);
    void setGridPosition(int gridX, int gridY);
    
    // 设置阵营
    void setFaction(Faction faction) { _faction = faction; }
    
    // ==================== 操作接口 ====================
    
    /**
     * @brief 受到伤害
     * @param damage 伤害值
     * @return 剩余血量
     */
    virtual int takeDamage(int damage);
    
    /**
     * @brief 修复建筑
     * @param amount 修复量
     */
    virtual void repair(int amount);
    
    /**
     * @brief 开始建造
     */
    virtual void startBuilding();
    
    /**
     * @brief 完成建造
     */
    virtual void finishBuilding();
    
    /**
     * @brief 开始升级
     * @return 是否成功开始升级
     */
    virtual bool startUpgrade();
    
    /**
     * @brief 完成升级
     */
    virtual void finishUpgrade();
    
    /**
     * @brief 取消建造/升级
     */
    virtual void cancelConstruction();
    
    /**
     * @brief 拆除建筑
     * @return 返还的资源(金币, 圣水)
     */
    virtual std::pair<int, int> demolish();
    
    /**
     * @brief 获取升级所需资源
     * @return (金币, 圣水)
     */
    virtual std::pair<int, int> getUpgradeCost() const;
    
    /**
     * @brief 获取升级所需时间(秒)
     * @return 升级时间
     */
    virtual int getUpgradeTime() const;
    
    /**
     * @brief 获取建造所需资源
     * @return (金币, 圣水)
     */
    virtual std::pair<int, int> getBuildCost() const;
    
    /**
     * @brief 获取建造所需时间(秒)
     * @return 建造时间
     */
    virtual int getBuildTime() const;
    
    // ==================== 回调设置 ====================
    
    // 设置点击回调
    void setClickCallback(BuildingCallback callback) { _clickCallback = callback; }
    
    // 设置摧毁回调
    void setDestroyCallback(BuildingCallback callback) { _destroyCallback = callback; }
    
    // ==================== 更新接口 ====================
    
    // 帧更新
    virtual void update(float dt) override;
    
    // 选中/取消选中
    virtual void setSelected(bool selected);
    bool isSelected() const { return _isSelected; }
    
    // 显示攻击范围(防御建筑用)
    virtual void showRange(bool show);

protected:
    // 受保护的构造函数
    Building();
    virtual ~Building();
    
    // 初始化精灵和动画
    virtual void initSprite();
    
    // 初始化建筑属性(根据类型和等级)
    virtual void initAttributes();
    
    // 创建血条
    virtual void createHealthBar();
    
    // 更新血条显示
    virtual void updateHealthBar();
    
    // 创建建造进度条
    virtual void createProgressBar();
    
    // 更新建造进度
    virtual void updateProgressBar(float progress);
    
    // 播放建造动画
    virtual void playBuildAnimation();
    
    // 播放摧毁动画
    virtual void playDestroyAnimation();
    
    // 播放升级动画
    virtual void playUpgradeAnimation();
    
    // 触发摧毁回调
    virtual void onDestroyed();
    
    // 处理触摸事件
    virtual bool onTouchBegan(Touch* touch, Event* event);
    
    // ==================== 成员变量 ====================
    
    // 基础属性
    BuildingType _buildingType;         // 建筑类型
    Faction _faction;                   // 阵营
    BuildingState _state;               // 建筑状态
    std::string _buildingName;          // 建筑名称
    
    // 生命属性
    int _currentHP;                     // 当前血量
    int _maxHP;                         // 最大血量
    
    // 等级属性
    int _level;                         // 当前等级
    int _maxLevel;                      // 最大等级
    
    // 占地属性
    int _gridWidth;                     // 占地宽度
    int _gridHeight;                    // 占地高度
    Vec2 _gridPosition;                 // 格子位置
    
    // 建造属性
    int _buildTime;                     // 建造时间(秒)
    float _buildProgress;               // 建造进度(0-1)
    
    // 资源属性
    int _goldCost;                      // 金币消耗
    int _elixirCost;                    // 圣水消耗
    float _refundRate;                  // 拆除返还比例
    
    // UI组件
    Sprite* _sprite;                    // 建筑精灵
    Sprite* _shadowSprite;              // 阴影精灵
    Node* _healthBar;                   // 血条
    Node* _progressBar;                 // 进度条
    Sprite* _selectionIndicator;        // 选中指示器
    
    // 状态标记
    bool _isSelected;                   // 是否被选中
    
    // 回调
    BuildingCallback _clickCallback;     // 点击回调
    BuildingCallback _destroyCallback;   // 摧毁回调
};

#endif // __BUILDING_H__
