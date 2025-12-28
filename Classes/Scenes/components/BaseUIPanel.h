/**
 * @file BaseUIPanel.h
 * @brief 基地场景UI面板组件
 * 
 * 将基地场景的UI布局逻辑提取出来，实现高度模块化。
 * 包含进攻、建造、退出等按钮以及资源显示。
 */

#ifndef __BASE_UI_PANEL_H__
#define __BASE_UI_PANEL_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <functional>

USING_NS_CC;
using namespace cocos2d::ui;

enum class ResourceType;

// ===================================================
// 基地UI配置常量
// ===================================================
namespace BaseUIConfig {
    // 按钮布局配置
    constexpr float EDGE_MARGIN = 24.0f;
    constexpr float BUTTON_SPACING = 18.0f;
    constexpr float BUTTON_SCALE = 3.0f;
    
    // 提示框配置
    constexpr float TOOLTIP_OFFSET_X = 10.0f;
    constexpr int TOOLTIP_FONT_SIZE = 18;
}

// ===================================================
// 按钮回调类型定义
// ===================================================
struct BaseUICallbacks {
    std::function<void()> onAttack;     // 进攻按钮回调
    std::function<void()> onBuild;      // 建造按钮回调
    std::function<void()> onExit;       // 退出按钮回调
    std::function<void()> onAsync;      // 异步分享面板
};

// ===================================================
// 基地UI面板类
// ===================================================
class BaseUIPanel : public Node {
public:
    /**
     * @brief 创建UI面板
     * @param callbacks 按钮回调函数集
     */
    static BaseUIPanel* create(const BaseUICallbacks& callbacks);
    
    /**
     * @brief 初始化
     */
    virtual bool init(const BaseUICallbacks& callbacks);
    
    /**
     * @brief 更新资源显示
     * @param gold 金币数量
     * @param diamond 钻石数量
     */
    void updateResourceDisplay(int gold, int diamond);

    /**
     * @brief 获取资源图标的世界坐标
     */
    Vec2 getResourceIconWorldPosition(ResourceType type) const;

    /**
     * @brief 设置主按钮可用状态
     */
    void setButtonsEnabled(bool enabled);
    
private:
    // UI组件
    Button* _attackButton = nullptr;
    Button* _buildButton = nullptr;
    Button* _exitButton = nullptr;
    Button* _asyncButton = nullptr;
    Label* _goldLabel = nullptr;
    Label* _diamondLabel = nullptr;
    Node* _idCardPanel = nullptr;
    
    // 回调
    BaseUICallbacks _callbacks;
    
    /**
     * @brief 创建按钮面板
     */
    void setupButtons();
    
    /**
     * @brief 创建资源显示面板
     */
    void setupResourcePanel();
    
    /**
     * @brief 创建提示框
     * @param text 提示文字
     * @param size 提示框尺寸
     * @return 创建的提示节点
     */
    Node* createTooltip(const std::string& text, const Size& size);
    
    /**
     * @brief 绑定提示框到按钮
     * @param targetBtn 目标按钮
     * @param tooltip 提示节点
     * @param offsetX X方向偏移量
     */
    void bindTooltip(Node* targetBtn, Node* tooltip, float offsetX = BaseUIConfig::TOOLTIP_OFFSET_X);

    /**
     * @brief 按钮悬停效果
     * @param button 目标按钮
     */
    void bindHoverEffect(Button* button);
};

#endif // __BASE_UI_PANEL_H__
