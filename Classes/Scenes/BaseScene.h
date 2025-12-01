// BaseScene.h
// 基地场景
// 功能: 玩家基地建设和管理的主场景

#ifndef __BASE_SCENE_H__
#define __BASE_SCENE_H__

#include "cocos2d.h"
#include "../Core/GameDefines.h"
#include <vector>
#include <map>

USING_NS_CC;

class Building;
class DefenseBuilding;
class ProductionBuilding;
class StorageBuilding;
class Barracks;
class Unit;

/**
 * @class BaseScene
 * @brief 基地场景类
 * 
 * 基地界面功能:
 * - 资源显示(金币/圣水/工人/人口实时数值)
 * - 建筑操作(建造/升级/拆除/移动)
 * - 建筑信息面板
 * - 进攻入口
 */
class BaseScene : public Scene {
public:
    static Scene* createScene();
    
    virtual bool init() override;
    
    // 帧更新
    virtual void update(float dt) override;
    
    CREATE_FUNC(BaseScene);

protected:
    // ==================== 初始化 ====================
    
    // 创建地图层
    void createMapLayer();
    
    // 创建网格
    void createGrid();
    
    // 创建UI层
    void createUILayer();
    
    // 创建资源显示
    void createResourceDisplay();
    
    // 创建底部工具栏
    void createToolbar();
    
    // 创建建筑菜单
    void createBuildMenu();
    
    // 初始化默认建筑
    void initDefaultBuildings();
    
    // ==================== 建筑操作 ====================
    
    // 添加建筑
    bool addBuilding(BuildingType type, int gridX, int gridY);
    
    // 移除建筑
    void removeBuilding(Building* building);
    
    // 选中建筑
    void selectBuilding(Building* building);
    
    // 取消选中
    void deselectBuilding();
    
    // 显示建筑信息面板
    void showBuildingInfoPanel(Building* building);
    
    // 隐藏建筑信息面板
    void hideBuildingInfoPanel();
    
    // 升级建筑
    void upgradeBuilding(Building* building);
    
    // 拆除建筑
    void demolishBuilding(Building* building);
    
    // 开始移动建筑
    void startMoveBuilding(Building* building);
    
    // 完成移动建筑
    void finishMoveBuilding(const Vec2& newGridPos);
    
    // 取消移动建筑
    void cancelMoveBuilding();
    
    // ==================== 地图操作 ====================
    
    // 检查格子是否可用
    bool isGridAvailable(int gridX, int gridY, int width, int height, Building* exclude = nullptr);
    
    // 屏幕坐标转格子坐标
    Vec2 screenToGrid(const Vec2& screenPos);
    
    // 格子坐标转屏幕坐标
    Vec2 gridToScreen(const Vec2& gridPos);
    
    // ==================== 输入处理 ====================
    
    // 触摸开始
    bool onTouchBegan(Touch* touch, Event* event);
    
    // 触摸移动
    void onTouchMoved(Touch* touch, Event* event);
    
    // 触摸结束
    void onTouchEnded(Touch* touch, Event* event);
    
    // 鼠标滚轮
    void onMouseScroll(Event* event);
    
    // ==================== UI回调 ====================
    
    // 打开建筑菜单
    void onBuildMenuOpen(Ref* sender);
    
    // 关闭建筑菜单
    void onBuildMenuClose(Ref* sender);
    
    // 选择要建造的建筑
    void onSelectBuildingType(BuildingType type);
    
    // 进入战斗
    void onAttack(Ref* sender);
    
    // 返回主菜单
    void onBackToMenu(Ref* sender);
    
    // 收集所有资源
    void onCollectAll(Ref* sender);
    
    // ==================== 资源更新 ====================
    
    // 更新资源显示
    void updateResourceDisplay();
    
    // 资源变化回调
    void onResourceChanged(ResourceType type, int oldValue, int newValue);
    
private:
    // 地图层
    Node* _mapLayer;
    Node* _gridLayer;
    Node* _buildingLayer;
    
    // UI层
    Node* _uiLayer;
    Node* _resourcePanel;
    Node* _toolbar;
    Node* _buildMenu;
    Node* _buildingInfoPanel;
    
    // 资源标签
    Label* _goldLabel;
    Label* _elixirLabel;
    Label* _workerLabel;
    Label* _populationLabel;
    
    // 建筑列表
    std::vector<Building*> _buildings;
    Building* _selectedBuilding;
    Building* _movingBuilding;
    bool _isMovingBuilding;
    
    // 建造模式
    bool _isBuildMode;
    BuildingType _buildingTypeToPlace;
    Node* _buildPreview;
    
    // 格子占用状态
    std::map<int, bool> _gridOccupied;  // key = gridY * MAP_WIDTH + gridX
    
    // 相机控制
    Vec2 _cameraOffset;
    float _cameraZoom;
    Vec2 _lastTouchPos;
    bool _isDragging;
};

#endif // __BASE_SCENE_H__
