/**
 * @file PlacementManager.h
 * @brief 建筑放置管理器
 *
 * 将建筑放置逻辑从BaseScene中提取出来，实现高度模块化。
 * 负责管理建筑放置预览、格子占用检测、放置确认等功能。
 */

#ifndef __PLACEMENT_MANAGER_H__
#define __PLACEMENT_MANAGER_H__

#include "cocos2d.h"
#include "Map/GridMap.h"
#include "BuildShopPanel.h"
#include <functional>

USING_NS_CC;

// ===================================================
// 放置管理器配置常量
// ===================================================
namespace PlacementConfig {
    // 预览透明度
    constexpr int PREVIEW_OPACITY = 180;

    // 格子颜色配置
    const Color4F GRID_COLOR_CAN_PLACE = Color4F(0.0f, 0.8f, 0.0f, 0.4f);      // 可放置：绿色
    const Color4F GRID_BORDER_CAN_PLACE = Color4F(0.0f, 1.0f, 0.0f, 0.8f);
    const Color4F GRID_COLOR_CANNOT_PLACE = Color4F(0.8f, 0.0f, 0.0f, 0.4f);   // 不可放置：红色
    const Color4F GRID_BORDER_CANNOT_PLACE = Color4F(1.0f, 0.0f, 0.0f, 0.8f);
    const Color4F GRID_COLOR_OCCUPIED = Color4F(0.5f, 0.5f, 0.5f, 0.3f);       // 已占用：灰色
    const Color4F GRID_COLOR_EMPTY = Color4F(0.0f, 0.5f, 0.0f, 0.2f);          // 空闲：淡绿色
    const Color4F GRID_COLOR_FORBIDDEN = Color4F(0.3f, 0.3f, 0.3f, 0.3f);      // 禁止区域：深灰色
}

// ===================================================
// 放置状态结构
// ===================================================
struct PlacementState {
    bool isPlacing = false;         // 是否处于放置模式
    int buildingType = 0;           // 当前建筑类型
    int gridWidth = 0;              // 建筑占用宽度
    int gridHeight = 0;             // 建筑占用高度
    std::string spritePath;         // 建筑图片路径
    int cost = 0;                   // 建造费用
};

// ===================================================
// 建筑放置管理器类
// ===================================================
class PlacementManager : public Node {
public:
    /**
     * @brief 创建放置管理器
     * @param gridMap 网格地图引用
     * @param onPlacementConfirmed 放置确认回调
     * @param onPlacementCancelled 放置取消回调
     */
    static PlacementManager* create(
        GridMap* gridMap,
        const std::function<void(const BuildingOption&, int, int)>& onPlacementConfirmed = nullptr,
        const std::function<void()>& onPlacementCancelled = nullptr
    );

    /**
     * @brief 初始化
     */
    virtual bool init(
        GridMap* gridMap,
        const std::function<void(const BuildingOption&, int, int)>& onPlacementConfirmed,
        const std::function<void()>& onPlacementCancelled
    );

    /**
     * @brief 开始放置建筑
     * @param option 建筑选项
     */
    void startPlacement(const BuildingOption& option);

    /**
     * @brief 更新预览位置（触摸移动时调用）
     * @param worldPos 世界坐标位置
     */
    void updatePreviewPosition(const Vec2& worldPos);

    /**
     * @brief 尝试确认放置（触摸结束时调用）
     * @param worldPos 世界坐标位置
     * @return 是否成功放置
     */
    bool tryConfirmPlacement(const Vec2& worldPos);

    /**
     * @brief 拖拽连续放置（用于地刺等）
     * @param worldPos 世界坐标位置
     * @return 是否成功放置
     */
    bool tryPaintPlacement(const Vec2& worldPos);

    /**
     * @brief 取消放置
     */
    void cancelPlacement();

    /**
     * @brief 检查是否处于放置模式
     */
    bool isPlacing() const { return _state.isPlacing; }

    /**
     * @brief 获取当前放置状态
     */
    const PlacementState& getState() const { return _state; }

    /**
     * @brief 绘制全地图格子状态（建造模式时显示）
     */
    void drawAllGridCells();

    /**
     * @brief 隐藏格子状态显示
     */
    void hideGridDisplay();

private:
    // 引用
    GridMap* _gridMap = nullptr;

    // UI组件
    Sprite* _previewSprite = nullptr;       // 建筑预览精灵
    DrawNode* _gridDisplayNode = nullptr;   // 全地图格子显示节点
    DrawNode* _placementGridNode = nullptr; // 当前放置位置格子显示节点
    Node* _previewInfoPanel = nullptr;      // 预览信息面板
    Label* _previewInfoLabel = nullptr;     // 预览信息文本

    // 放置状态
    PlacementState _state;
    BuildingOption _currentOption;
    int _currentGridX = 0;
    int _currentGridY = 0;
    int _lastPaintGridX = -9999;
    int _lastPaintGridY = -9999;

    // 回调函数
    std::function<void(const BuildingOption&, int, int)> _onPlacementConfirmed;
    std::function<void()> _onPlacementCancelled;

    /**
     * @brief 计算建筑在世界坐标系中的中心位置
     * @param gridX 网格X坐标（左下角）
     * @param gridY 网格Y坐标（左下角）
     * @param width 建筑宽度（格子数）
     * @param height 建筑高度（格子数）
     * @return 建筑中心的世界坐标
     */
    Vec2 calculateBuildingCenterPosition(int gridX, int gridY, int width, int height);

    /**
     * @brief 更新当前放置位置的格子颜色显示
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @param canPlace 是否可以放置
     */
    void updatePlacementGridDisplay(int gridX, int gridY, bool canPlace);

    /**
     * @brief 创建预览信息面板
     */
    void setupPreviewInfo();

    /**
     * @brief 更新预览信息显示
     */
    void updatePreviewInfo(bool canPlace);

    /**
     * @brief 更新预览信息位置
     */
    void updatePreviewInfoPosition(const Vec2& centerPos);

    /**
     * @brief 清理预览资源
     */
    void cleanupPreview();
};

#endif // __PLACEMENT_MANAGER_H__
