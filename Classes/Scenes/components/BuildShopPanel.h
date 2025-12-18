/**
 * @file BuildShopPanel.h
 * @brief 建筑商店面板组件
 *
 * 将建筑商店逻辑从BaseScene中提取出来，实现高度模块化。
 * 该面板显示可建造的建筑列表，玩家可以选择要建造的建筑。
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
    const Size PANEL_SIZE = Size(400.0f, 500.0f);

    // 标题配置
    constexpr int TITLE_FONT_SIZE = 28;
    constexpr float TITLE_TOP_OFFSET = 40.0f;

    // 按钮配置
    constexpr float BUTTON_SPACING = 60.0f;
    constexpr float BUTTON_START_Y = 400.0f;
    constexpr int BUTTON_FONT_SIZE = 18;

    // 关闭按钮配置
    constexpr float CLOSE_BUTTON_BOTTOM = 50.0f;
}

// ===================================================
// 建筑选项结构（按照部落冲突设定）
// ===================================================
struct BuildingOption {
    int type;               // 建筑类型ID
    std::string name;       // 建筑名称
    int cost;               // 建造费用
    int gridWidth;          // 占用格子宽度（部落冲突设定）
    int gridHeight;         // 占用格子高度（部落冲突设定）
    std::string spritePath; // 建筑图片路径
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

private:
    // UI组件
    LayerColor* _background = nullptr;      // 半透明背景
    LayerColor* _panel = nullptr;           // 面板主体
    Label* _titleLabel = nullptr;           // 标题

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
    void setupBuildingButtons();
    void setupCloseButton();

    /**
     * @brief 初始化建筑选项
     * 按照部落冲突的设定配置建筑尺寸
     */
    void initBuildingOptions();

    /**
     * @brief 创建建筑按钮
     * @param option 建筑选项
     * @param yPos Y轴位置
     * @return 创建的按钮
     */
    Button* createBuildingButton(const BuildingOption& option, float yPos);
};

#endif // __BUILD_SHOP_PANEL_H__