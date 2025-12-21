/**
 * @file BuildShopPanel.h
 * @brief 建筑商店面板组件
 *
 * 将建筑商店逻辑从BaseScene中抽取出来，实现高度模块化。
 * 本组件显示可建造的建筑列表，玩家可以选择要建造的建筑。
 */

#ifndef __BUILD_SHOP_PANEL_H__
#define __BUILD_SHOP_PANEL_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <functional>
#include <vector>

USING_NS_CC;
using namespace cocos2d::ui;

// ===================================================
// 建筑商店配置常量
// ===================================================
namespace BuildShopConfig {
    // 面板尺寸
    const Size PANEL_SIZE = Size(600.0f, 500.0f);

    // 标题配置
    constexpr int TITLE_FONT_SIZE = 28;
    constexpr float TITLE_TOP_OFFSET = 40.0f;

    // 网格布局配置
    constexpr int GRID_COLS = 3;                    // 每行显示的建筑数量
    constexpr float GRID_ITEM_SIZE = 140.0f;        // 每个网格项的尺寸
    constexpr float GRID_SPACING = 20.0f;           // 网格项之间的间距
    constexpr float GRID_START_Y = 380.0f;          // 网格起始Y位置
    constexpr float ICON_SIZE = 80.0f;              // 建筑图标尺寸

    // 关闭按钮配置
    constexpr float CLOSE_BUTTON_BOTTOM = 40.0f;
}

// ===================================================
// 建筑选项结构（含部落冲突设定）
// ===================================================
struct BuildingOption {
    int type;               // 建筑类型ID
    std::string name;       // 建筑名称
    int cost;               // 建造费用
    int gridWidth;          // 占用格子宽度（部落冲突设定）
    int gridHeight;         // 占用格子高度（部落冲突设定）
    std::string spritePath; // 建筑图片路径
    bool canBuild;          // 是否可以建造（基地和兵营只能有一个）
};

// ===================================================
// 建筑商店面板类
// ===================================================
class BuildShopPanel : public Node {
public:
    /**
     * @brief 创建建筑商店面板
     * @param onBuildingSelected 选择建筑时的回调，参数为建筑选项
     * @param onClose 关闭面板时的回调
     * @return 创建的面板实例
     */
    static BuildShopPanel* create(
        const std::function<void(const BuildingOption&)>& onBuildingSelected = nullptr,
        const std::function<void()>& onClose = nullptr
    );

    /**
     * @brief 初始化面板
     */
    virtual bool init(
        const std::function<void(const BuildingOption&)>& onBuildingSelected,
        const std::function<void()>& onClose
    );

    /**
     * @brief 显示面板
     */
    void show();

    /**
     * @brief 隐藏面板
     */
    void hide();

    /**
     * @brief 检查面板是否正在显示
     */
    bool isShowing() const { return _isShowing; }

    /**
     * @brief 获取建筑选项列表
     */
    const std::vector<BuildingOption>& getBuildingOptions() const { return _buildingOptions; }

    /**
     * @brief 设置建筑是否可建造
     * @param type 建筑类型
     * @param canBuild 是否可建造
     */
    void setBuildingCanBuild(int type, bool canBuild);

private:
    // UI组件
    LayerColor* _background = nullptr;      // 半透明背景
    LayerColor* _panel = nullptr;           // 面板主体
    Label* _titleLabel = nullptr;           // 标题
    Node* _gridContainer = nullptr;         // 网格容器

    // 建筑选项列表
    std::vector<BuildingOption> _buildingOptions;

    // 回调函数
    std::function<void(const BuildingOption&)> _onBuildingSelected;
    std::function<void()> _onClose;

    // 状态
    bool _isShowing = false;

    // 初始化方法
    void setupBackground();
    void setupPanel();
    void setupTitle();
    void setupBuildingGrid();
    void setupCloseButton();

    /**
     * @brief 初始化建筑选项
     * 按照部落冲突的设定配置建筑尺寸
     */
    void initBuildingOptions();

    /**
     * @brief 创建单个建筑网格项（带图标）
     * @param option 建筑选项
     * @param row 行号
     * @param col 列号
     * @return 创建的网格项节点
     */
    Node* createBuildingGridItem(const BuildingOption& option, int row, int col);
};

#endif // __BUILD_SHOP_PANEL_H__