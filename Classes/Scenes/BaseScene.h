/**
 * @file BaseScene.h
 * @brief 基地场景头文件
 * 
 * 基地场景是玩家的主要经营界面，包含以下功能：
 * - 建筑建造与管理
 * - 兵种训练
 * - 资源显示
 * - 进攻关卡入口
 */

#ifndef __BASE_SCENE_H__
#define __BASE_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Map/GridMap.h"
#include "Buildings/BuildingManager.h"
#include "UI/TrainPanel.h"

USING_NS_CC;
using namespace cocos2d::ui;

// 前向声明
class TrainPanel;

/**
 * @class BaseScene
 * @brief 基地场景类
 * 
 * 玩家的主基地场景，管理建筑放置、兵种训练、资源显示等功能
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

    /**
     * @brief 进攻按钮回调，跳转到关卡选择场景
     */
    void onAttackButton(Ref* sender);
    
    /**
     * @brief 建造按钮回调，显示建筑商店
     */
    void onBuildButton(Ref* sender);
    
    /**
     * @brief 退出按钮回调，返回主菜单
     */
    void onExitButton(Ref* sender);

private:
    // ==================== 核心组件 ====================
    GridMap* _gridMap = nullptr;           // 网格地图，管理建筑位置
    Node* _buildingLayer = nullptr;        // 建筑层，所有建筑的父节点
    Node* _uiLayer = nullptr;              // UI层，存放按钮等界面元素
    Node* _buildShopLayer = nullptr;       // 建筑商店层
    TrainPanel* _trainPanel = nullptr;     // 训练面板（点击兵营时显示）
    
    // ==================== 建造模式状态 ====================
    bool _isPlacingMode = false;           // 是否处于建造模式
    Node* _draggingBuilding = nullptr;     // 正在拖拽的建筑（暂未使用）
    Sprite* _placementPreview = nullptr;   // 建造预览精灵
    int _previewWidth = 0;                 // 预览建筑宽度（格子数）
    int _previewHeight = 0;                // 预览建筑高度（格子数）
    int _selectedBuildingType = 0;         // 当前选中的建筑类型
    int _selectedLevel = 0;                // 当前选中的建筑等级
    
    // ==================== 资源管理 ====================
    int _currentGold = 0;                  // 当前金币数量
    int _currentDiamond = 0;               // 当前钻石数量
    Label* _goldLabel = nullptr;           // 金币显示标签
    Label* _diamondLabel = nullptr;        // 钻石显示标签
    
    // ==================== 建筑配置 ====================
    ProductionBuildingConfig _baseConfig;  // 基地建筑配置
    
    // ==================== 网格显示 ====================
    DrawNode* _gridBackgroundNode = nullptr;  // 网格背景绘制节点
    DrawNode* _placementGridNode = nullptr;   // 放置状态网格显示节点
    
    // ==================== 初始化方法 ====================
    
    /**
     * @brief 创建网格背景
     * 绘制黑白虚线网格作为建造参考
     */
    void createGridBackground();
    
    /**
     * @brief 创建主UI界面
     * 包含进攻、建造、退出按钮和资源显示
     */
    void createUI();
    
    /**
     * @brief 创建工具提示面板
     * @param text 提示文字
     * @param size 面板尺寸
     * @return 创建的提示节点
     */
    Node* createTooltip(const std::string& text, const Size& size);
    
    /**
     * @brief 绑定工具提示到按钮
     * @param owner 提示的父节点
     * @param targetBtn 目标按钮
     * @param tooltip 提示节点
     * @param offsetX X方向偏移量
     */
    void bindTooltip(Node* owner, Node* targetBtn, Node* tooltip, float offsetX = 10.0f);
    
    /**
     * @brief 创建建筑商店界面
     */
    void createBuildShop();
    
    /**
     * @brief 创建训练面板
     */
    void createTrainPanel();
    
    /**
     * @brief 初始化基地默认建筑
     * 在地图中心放置初始的基地建筑
     */
    void initBaseBuilding();

    // ==================== 建造相关方法 ====================
    
    /**
     * @brief 建筑选择回调
     * @param buildingType 选中的建筑类型ID
     */
    void onBuildingSelected(int buildingType);
    
    /**
     * @brief 更新建造预览显示
     * 根据当前触摸位置更新预览位置和颜色
     */
    void updatePlacementPreview();
    
    /**
     * @brief 更新放置网格颜色显示
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @param canPlace 是否可以放置
     */
    void updatePlacementGrid(int gridX, int gridY, bool canPlace);
    
    /**
     * @brief 确认放置建筑
     * 在当前位置创建实际建筑并扣除资源
     */
    void confirmPlacement();
    
    /**
     * @brief 取消建造模式
     * 移除预览并退出建造状态
     */
    void cancelPlacement();
    
    /**
     * @brief 计算建筑在世界坐标系中的位置
     * @param gridX 网格X坐标（左下角）
     * @param gridY 网格Y坐标（左下角）
     * @param width 建筑宽度（格子数）
     * @param height 建筑高度（格子数）
     * @return 建筑中心的世界坐标
     */
    Vec2 calculateBuildingPosition(int gridX, int gridY, int width, int height);
    
    // ==================== 训练相关方法 ====================
    
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
};

#endif // __BASE_SCENE_H__