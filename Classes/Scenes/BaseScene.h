/**
 * @file BaseScene.h
 * @brief 基地场景头文件
 *
 * 基地场景是玩家的主要经营界面，包含以下功能：
 * - 建筑建造与管理
 * - 兵种训练
 * - 资源显示
 * - 进攻关卡入口
 *
 * 该场景采用高度模块化设计，各功能由独立组件实现：
 * - BuildShopPanel: 建筑商店面板
 * - PlacementManager: 建筑放置管理器
 * - BaseUIPanel: UI面板
 * - GridBackground: 网格背景
 */

#ifndef __BASE_SCENE_H__
#define __BASE_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Map/GridMap.h"
#include "Buildings/BuildingManager.h"
#include "UI/TrainPanel.h"
#include "Scenes/Components/BuildShopPanel.h"
#include "Scenes/Components/PlacementManager.h"
#include "Scenes/Components/BaseUIPanel.h"
#include "Scenes/Components/GridBackground.h"

USING_NS_CC;
using namespace cocos2d::ui;

// 前向声明
class TrainPanel;
class BuildShopPanel;
class PlacementManager;
class BaseUIPanel;
class GridBackground;
enum class ResourceType;

/**
 * @class BaseScene
 * @brief 基地场景类
 *
 * 玩家的主基地场景，采用模块化设计管理建筑放置、兵种训练、资源显示等功能。
 * 建筑格子占用遵循部落冲突设定：
 * - 防御塔类（箭塔、炮塔等）：3x3格子
 * - 资源建筑（仓库等）：3x3格子
 * - 兵营：5x5格子
 * - 大本营：4x4格子
 * - 装饰物（树等）：2x2格子
 */
class BaseScene : public Scene {
public:
    /**
     * @brief 创建场景
     * @return 新创建的场景实例
     */
    static Scene* createScene();

    /**
     * @brief 初始化场景
     * @return 初始化是否成功
     */
    virtual bool init() override;

    CREATE_FUNC(BaseScene);


private:
    // ==================== 核心组件 ====================
    GridMap* _gridMap = nullptr;                   // 网格地图，管理建筑位置
    Node* _buildingLayer = nullptr;                // 建筑层，所有建筑的父节点
    GridBackground* _gridBackground = nullptr;     // 网格背景组件
    BaseUIPanel* _uiPanel = nullptr;               // UI面板组件
    BuildShopPanel* _buildShopPanel = nullptr;     // 建筑商店面板组件
    PlacementManager* _placementManager = nullptr; // 建筑放置管理器组件
    TrainPanel* _trainPanel = nullptr;             // 训练面板    
    Node* _effectLayer = nullptr;                  // 特效层

    // ==================== 悬浮信息 ====================
    Node* _hoverInfoPanel = nullptr;               // 悬浮信息面板
    LayerColor* _hoverInfoBg = nullptr;            // 悬浮信息背景
    Label* _hoverInfoLabel = nullptr;              // 悬浮信息文字
    DrawNode* _hoverRangeNode = nullptr;           // 攻击范围虚线
    DrawNode* _hoverFootprintNode = nullptr;       // 占地实线框
    Node* _hoveredBuilding = nullptr;              // 当前悬浮建筑

    // ==================== 建筑配置 ====================
    ProductionBuildingConfig _baseConfig;          // 基地建筑配置
    ProductionBuildingConfig _barracksConfig;      // 兵营建筑配置

    // ==================== 初始化方法 ====================

    /**
     * @brief 初始化网格地图和背景
     */
    void initGridMap();

    /**
     * @brief 初始化UI组件
     */
    void initUIComponents();

    /**
     * @brief 初始化建造系统组件
     */
    void initBuildingSystem();

    /**
     * @brief 创建训练面板
     */
    void createTrainPanel();

    /**
     * @brief 初始化基地默认建筑
     * 在地图中心放置初始的基地建筑（4x4格子）
     */
    void initBaseBuilding();
    /**
    * @brief 初始化触摸事件监听
    */
    void initTouchListener();
    void initHoverInfo();

    // ==================== 回调方法 ====================
    /**
     * @brief 进攻按钮回调，跳转到关卡选择场景
     */
    void onAttackClicked();

    /**
     * @brief 建造按钮回调，显示建筑商店
     */
    void onBuildClicked();

    /**
     * @brief 退出按钮回调，返回主菜单
     */
    void onExitClicked();

    /**
     * @brief 建筑选择回调（从建筑商店）
     * @param option 选中的建筑选项
     */
    void onBuildingSelected(const BuildingOption& option);

    /**
     * @brief 建筑放置确认回调
     * @param option 建筑选项
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     */
    void onPlacementConfirmed(const BuildingOption& option, int gridX, int gridY);

    /**
     * @brief 显示训练面板
     * 点击兵营建筑时调用
     */
    void showTrainPanel();

    /**
     * @brief 兵种训练完成回调
     * @param unitId 完成训练的兵种ID
     */
    void onUnitTrainComplete(int unitId);

    // ==================== 建筑创建方法 ====================

    /**
     * @brief 根据建筑选项创建实际建筑
     * @param option 建筑选项
     * @return 创建的建筑节点
     */
    Node* createBuildingFromOption(const BuildingOption& option);

    /**
     * @brief 计算建筑在世界坐标系中的位置
     * @param gridX 网格X坐标（左下角）
     * @param gridY 网格Y坐标（左下角）
     * @param width 建筑宽度（格子数）
     * @param height 建筑高度（格子数）
     * @return 建筑中心的世界坐标
     */
    Vec2 calculateBuildingPosition(int gridX, int gridY, int width, int height);

    /**
     * @brief 恢复已建造的建筑
     * 用于从进攻界面返回时保持基地状态
     */
    void restoreSavedBuildings();

    /**
     * @brief 记录已建造的建筑
     * @param option 建筑选项
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     */
    void savePlacedBuilding(const BuildingOption& option, int gridX, int gridY);

    /**
     * @brief 调整建筑缩放以适应格子大小
     * @param building 建筑节点
     * @param gridWidth 格子宽度
     * @param gridHeight 格子高度
     * @param cellSize 单个格子的像素尺寸
     */
    void scaleBuildingToFit(Node* building, int gridWidth, int gridHeight, float cellSize);
    void setupProductionCollect(ProductionBuilding* building);
    void playCollectEffect(ResourceType type, int amount, const Vec2& worldPos);

    // ==================== 触摸事件处理 ====================

    /**
     * @brief 触摸开始事件
     * @return 是否处理此触摸事件
     */
    bool onTouchBegan(Touch* touch, Event* event);

    /**
     * @brief 触摸移动事件
     * 在建造模式下更新预览位置
     */
    void onTouchMoved(Touch* touch, Event* event);

    /**
     * @brief 触摸结束事件
     * 在建造模式下确认或取消放置
     */
    void onTouchEnded(Touch* touch, Event* event);

    // ==================== 悬浮信息处理 ====================
    void updateHoverInfo(const Vec2& worldPos);
    Node* pickBuildingAt(const Vec2& worldPos) const;
    void showBuildingInfo(Node* building);
    void clearBuildingInfo();
    void updateHoverOverlays(Node* building);
    void updateHoverPanelPosition(const Vec2& worldPos);
};

#endif // __BASE_SCENE_H__
