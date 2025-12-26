/**
 * @file BuildShopPanel.h
 * @brief 建筑商店面板
 *
 * 将建筑商店逻辑从BaseScene中抽取出来，实现高度模块化。
 * 面板用于显示可建造的建筑列表，玩家可以选择要建造的建筑。
 */

#ifndef __BUILD_SHOP_PANEL_H__
#define __BUILD_SHOP_PANEL_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Buildings/BuildingCatalog.h"
#include <functional>
#include <vector>

USING_NS_CC;
using namespace cocos2d::ui;

// ===================================================
// 建筑商店配置常量
// ===================================================
namespace BuildShopConfig {
    // 面板尺寸（缩小以适应更小的建筑图标）
    const Size PANEL_SIZE = Size(450.0f, 450.0f);

    // 标题配置
    constexpr int TITLE_FONT_SIZE = 22;
    constexpr float TITLE_TOP_OFFSET = 30.0f;

    // 网格布局配置（缩小至原来的1/2大小，保持美观）
    constexpr int GRID_COLS = 3;                    // 每行显示的建筑数量
    constexpr float GRID_ITEM_SIZE = 70.0f;         // 每个建筑项的尺寸（原140的一半）
    constexpr float GRID_SPACING = 15.0f;           // 建筑项之间的间距
    constexpr float GRID_START_Y = 260.0f;          // 网格起始Y位置（调整适应新面板大小）
    constexpr float ICON_SIZE = 40.0f;              // 建筑图标尺寸（原80的一半）

    // 关闭按钮配置
    constexpr float CLOSE_BUTTON_BOTTOM = 30.0f;
}

// ===================================================
// 建筑商店面板类
// ===================================================
class BuildShopPanel : public Node {
public:
    /**
     * @brief 创建建筑商店面板
     * @param onBuildingSelected 选择建筑时的回调，参数为建筑选项
     * @param onClose 关闭面板时的回调
     * @return 面板实例
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
    LayerColor* _background = nullptr;      // 半透明遮罩
    LayerColor* _panel = nullptr;           // 主面板
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
     * 根据部落冲突的设定设置建筑尺寸
     */
    void initBuildingOptions();

    /**
     * @brief 创建建筑网格项（包含小图标）
     * @param option 建筑选项
     * @param row 行号
     * @param col 列号
     * @return 建筑网格项节点
     */
    Node* createBuildingGridItem(const BuildingOption& option, int row, int col);
};

#endif // __BUILD_SHOP_PANEL_H__
