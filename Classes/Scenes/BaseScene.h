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

class BaseScene : public Scene {
public:
    static Scene* createScene();
    // 创建并返回一个新的场景实例。通常作为场景切换时的入口函数使用。
    virtual bool init() override;
    // 初始化场景，创建地图、UI、建筑等子节点并设置事件监听。返回是否初始化成功。
    CREATE_FUNC(BaseScene);

    void onAttackButton(Ref* sender);
    // 攻击按钮回调：响应点击攻击按钮，收集已训练部队并切换到关卡选择场景。
    void onBuildButton(Ref* sender);
    // 建造按钮回调：显示建筑商店界面以供玩家选择建造的建筑。
    void onExitButton(Ref* sender);
    // 退出按钮回调：返回主菜单或退出当前场景。

private:
    GridMap* _gridMap = nullptr;
    Node* _buildingLayer = nullptr;
    Node* _uiLayer = nullptr;
    Node* _buildShopLayer = nullptr;
    
    // 训练面板（点击兵营建筑时显示）
    TrainPanel* _trainPanel = nullptr;
    
    bool _isPlacingMode = false;
    Node* _draggingBuilding = nullptr;
    Sprite* _placementPreview = nullptr;
    int _previewWidth = 0;
    int _previewHeight = 0;
    int _selectedBuildingType = 0;
    int _selectedLevel = 0;
    
    int _currentGold = 0;
    int _currentDiamond = 0;
    Label* _goldLabel = nullptr;
    Label* _diamondLabel = nullptr;
    
    // Store building configs as member variables
    ProductionBuildingConfig _baseConfig;
    
    void createGrassBackground();
    // 创建草地背景贴图，填充整个地图区域以作为地面图层。
    void createUI();
    // 创建并布局游戏主UI（按钮、资源显示等），将UI节点添加到场景中。
    void createBuildShop();
    // 创建建筑商店界面并初始化建筑选项及其回调，但默认不显示。
    void createTrainPanel();
    // 创建兵种训练面板并传入训练完成和关闭的回调函数，将面板作为子节点添加到场景。
    void initBaseBuilding();
    // 初始化基地默认建筑（例如基地和初始兵营），并将其放置在地图指定位置。

    void onBuildingSelected(int buildingType);
    // 当玩家在商店中选择某种建筑时调用，进入放置模式并创建放置预览精灵。
    void updatePlacementPreview();
    // 更新放置预览位置和颜色，基于当前位置是否可放置来改变视觉反馈。
    void confirmPlacement();
    // 确认放置建筑：从预览切换为实际建筑、扣除资源并隐藏网格与预览。
    void cancelPlacement();
    // 取消放置模式：移除预览并恢复正常交互，隐藏网格线。
    
    // 打开训练面板（点击兵营时调用）
    void showTrainPanel();
    // 打开/显示训练面板（当点击兵营建筑时调用）。
    
    // 训练完成回调
    void onUnitTrainComplete(int unitId);
    // 处理兵种训练完成事件（unitId 表示训练完成的兵种ID），可用于更新UI或队列。
    
    bool onTouchBegan(Touch* touch, Event* event);
    // 触摸开始事件处理：处理点击建筑、打开训练面板或进入放置模式等交互逻辑。
    void onTouchMoved(Touch* touch, Event* event);
    // 触摸移动事件处理：在放置模式下移动预览并实时检测是否可放置。
    void onTouchEnded(Touch* touch, Event* event);
    // 触摸结束事件处理：在放置模式下进行最终位置检测并确认或取消放置。
};

#endif // __BASE_SCENE_H__
