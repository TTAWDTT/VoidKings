/**
 * @file BuildShopPanel.h
 * @brief �����̵�������
 *
 * �������̵��߼���BaseScene�г�ȡ������ʵ�ָ߶�ģ�黯��
 * �������ʾ�ɽ���Ľ����б�����ҿ���ѡ��Ҫ����Ľ�����
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
// �����̵����ó���
// ===================================================
namespace BuildShopConfig {
    // 面板尺寸（缩小以适应更小的建筑图标）
    const Size PANEL_SIZE = Size(450.0f, 350.0f);

    // 标题配置
    constexpr int TITLE_FONT_SIZE = 22;
    constexpr float TITLE_TOP_OFFSET = 30.0f;

    // 网格布局配置（缩小至原来的1/4大小）
    constexpr int GRID_COLS = 3;                    // 每行显示的建筑数量
    constexpr float GRID_ITEM_SIZE = 70.0f;         // 每个建筑项的尺寸（原140/2）
    constexpr float GRID_SPACING = 15.0f;           // 建筑项之间的间距
    constexpr float GRID_START_Y = 260.0f;          // 网格起始Y位置（调整适应新面板大小）
    constexpr float ICON_SIZE = 40.0f;              // 建筑图标尺寸（原80/2）

    // 关闭按钮配置
    constexpr float CLOSE_BUTTON_BOTTOM = 30.0f;
}

// ===================================================
// ����ѡ��ṹ���������ͻ�趨��
// ===================================================
struct BuildingOption {
    int type;               // ��������ID
    std::string name;       // ��������
    int cost;               // �������
    int gridWidth;          // ռ�ø��ӿ��ȣ������ͻ�趨��
    int gridHeight;         // ռ�ø��Ӹ߶ȣ������ͻ�趨��
    std::string spritePath; // ����ͼƬ·��
    bool canBuild;          // �Ƿ���Խ��죨���غͱ�Ӫֻ����һ����
};

// ===================================================
// �����̵������
// ===================================================
class BuildShopPanel : public Node {
public:
    /**
     * @brief ���������̵����
     * @param onBuildingSelected ѡ����ʱ�Ļص�������Ϊ����ѡ��
     * @param onClose �ر����ʱ�Ļص�
     * @return ���������ʵ��
     */
    static BuildShopPanel* create(
        const std::function<void(const BuildingOption&)>& onBuildingSelected = nullptr,
        const std::function<void()>& onClose = nullptr
    );

    /**
     * @brief ��ʼ�����
     */
    virtual bool init(
        const std::function<void(const BuildingOption&)>& onBuildingSelected,
        const std::function<void()>& onClose
    );

    /**
     * @brief ��ʾ���
     */
    void show();

    /**
     * @brief �������
     */
    void hide();

    /**
     * @brief �������Ƿ�������ʾ
     */
    bool isShowing() const { return _isShowing; }

    /**
     * @brief ��ȡ����ѡ���б�
     */
    const std::vector<BuildingOption>& getBuildingOptions() const { return _buildingOptions; }

    /**
     * @brief ���ý����Ƿ�ɽ���
     * @param type ��������
     * @param canBuild �Ƿ�ɽ���
     */
    void setBuildingCanBuild(int type, bool canBuild);

private:
    // UI���
    LayerColor* _background = nullptr;      // ��͸������
    LayerColor* _panel = nullptr;           // �������
    Label* _titleLabel = nullptr;           // ����
    Node* _gridContainer = nullptr;         // ��������

    // ����ѡ���б�
    std::vector<BuildingOption> _buildingOptions;

    // �ص�����
    std::function<void(const BuildingOption&)> _onBuildingSelected;
    std::function<void()> _onClose;

    // ״̬
    bool _isShowing = false;

    // ��ʼ������
    void setupBackground();
    void setupPanel();
    void setupTitle();
    void setupBuildingGrid();
    void setupCloseButton();

    /**
     * @brief ��ʼ������ѡ��
     * ���ղ����ͻ���趨���ý����ߴ�
     */
    void initBuildingOptions();

    /**
     * @brief �������������������ͼ�꣩
     * @param option ����ѡ��
     * @param row �к�
     * @param col �к�
     * @return ������������ڵ�
     */
    Node* createBuildingGridItem(const BuildingOption& option, int row, int col);
};

#endif // __BUILD_SHOP_PANEL_H__