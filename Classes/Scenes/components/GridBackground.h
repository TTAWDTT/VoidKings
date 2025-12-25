/**
 * @file GridBackground.h
 * @brief 网格背景组件
 *
 * 将网格背景绘制逻辑从BaseScene中提取出来，实现高度模块化。
 * 负责绘制黑白虚线网格作为建造参考。
 */

#ifndef __GRID_BACKGROUND_H__
#define __GRID_BACKGROUND_H__

#include "cocos2d.h"

USING_NS_CC;

// ===================================================
// 网格背景配置常量
// ===================================================
namespace GridBackgroundConfig {
    // 背景颜色
    const Color4B BG_COLOR = Color4B(36, 58, 36, 255);

    // 虚线参数
    constexpr float DASH_LENGTH = 4.0f;
    constexpr float GAP_LENGTH = 4.0f;
    const Color4F LINE_COLOR = Color4F(0.2f, 0.35f, 0.2f, 0.45f);
}

// ===================================================
// 网格背景类
// ===================================================
class GridBackground : public Node {
public:
    /**
     * @brief 创建网格背景
     * @param gridWidth 网格宽度（格子数）
     * @param gridHeight 网格高度（格子数）
     * @param cellSize 每格像素大小
     */
    static GridBackground* create(int gridWidth, int gridHeight, float cellSize);

    /**
     * @brief 初始化
     */
    virtual bool init(int gridWidth, int gridHeight, float cellSize);

    /**
     * @brief 重新绘制网格
     */
    void redraw();

private:
    int _gridWidth = 0;
    int _gridHeight = 0;
    float _cellSize = 0.0f;

    LayerColor* _bgColor = nullptr;
    Sprite* _tileBackground = nullptr;
    DrawNode* _gridLines = nullptr;

    /**
     * @brief 绘制虚线网格
     */
    void drawDashedGrid();
};

#endif // __GRID_BACKGROUND_H__