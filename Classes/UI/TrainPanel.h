// TrainPanel.h
// 兵种训练面板组件
// 黑白网格风格，显示兵种待机动画，含招募和升级按钮

#ifndef __TRAIN_PANEL_H__
#define __TRAIN_PANEL_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Soldier/UnitData.h"
#include "Soldier/UnitManager.h"
#include <vector>
#include <functional>
#include <map>

USING_NS_CC;
using namespace cocos2d::ui;

// ===================================================
// 训练队列中的兵种信息
// ===================================================
struct TrainQueueItem {
    int unitId;                 // 兵种ID
    std::string unitName;       // 兵种名称
    float remainingTime;        // 剩余训练时间（秒）
    float totalTime;            // 总训练时间（秒）
};

// ===================================================
// 兵种等级信息
// ===================================================
struct UnitLevelInfo {
    int unitId;                 // 兵种ID
    int currentLevel;           // 当前等级
    int count;                  // 拥有数量
};

// ===================================================
// 训练面板配置常量（优化后的紧凑布局）
// ===================================================
namespace TrainPanelConfig {
    // 面板尺寸 - 提升容纳量
    const Size PANEL_SIZE = Size(640.0f, 600.0f);

    // 标题配置 - 缩小字体
    constexpr int TITLE_FONT_SIZE = 22;
    constexpr float TITLE_TOP_OFFSET = 20.0f;

    // 兵种卡片配置 - 缩小尺寸
    constexpr int GRID_COLS = 4;                    // 每行显示的兵种数量
    constexpr float UNIT_CARD_WIDTH = 130.0f;       // 兵种卡片宽度
    constexpr float UNIT_CARD_HEIGHT = 150.0f;      // 兵种卡片高度
    constexpr float UNIT_CARD_SPACING = 12.0f;      // 卡片之间的间距
    constexpr float UNIT_AREA_TOP_OFFSET = 50.0f;   // 兵种区域距离顶部的偏移
    constexpr float ANIM_SIZE = 44.0f;              // 动画显示尺寸

    // 按钮配置 - 缩小尺寸
    constexpr float BUTTON_WIDTH = 52.0f;
    constexpr float BUTTON_HEIGHT = 22.0f;

    // 默认招募费用（当配置中COST_COIN为0时使用）
    constexpr int DEFAULT_RECRUIT_COST = 50;

    // 关闭按钮配置
    constexpr float CLOSE_BUTTON_BOTTOM = 20.0f;
}

// ===================================================
// 兵种训练面板类
// ===================================================
class TrainPanel : public Node {
public:
    // 创建训练面板
    // onTrainComplete: 训练完成时的回调，参数为兵种ID
    // onClose: 关闭面板时的回调
    static TrainPanel* create(
        const std::function<void(int)>& onTrainComplete = nullptr,
        const std::function<void()>& onClose = nullptr
    );

    virtual bool init(
        const std::function<void(int)>& onTrainComplete,
        const std::function<void()>& onClose
    );

    // 显示/隐藏面板
    void show();
    void hide();
    bool isShowing() const { return _isShowing; }

    // 更新训练队列（每帧调用）
    void update(float dt) override;

    // 获取当前训练队列
    const std::vector<TrainQueueItem>& getTrainQueue() const { return _trainQueue; }

    // 获取已训练完成的兵种数量
    const std::map<int, int>& getTrainedUnits() const { return UnitManager::getInstance()->getTrainedUnits(); }

    // 清空已训练完成的兵种（派兵后调用）
    void clearTrainedUnits() { UnitManager::getInstance()->resetTrainedUnits(); }

private:
    // UI组件
    LayerColor* _background = nullptr;      // 半透明背景
    LayerColor* _panel = nullptr;           // 面板主体
    Label* _titleLabel = nullptr;           // 标题
    ScrollView* _unitCardArea = nullptr;          // 兵种卡片区域
    Label* _resourceLabel = nullptr;        // 资源显示

    // 训练队列
    std::vector<TrainQueueItem> _trainQueue;

    // 兵种等级信息 <兵种ID, 等级信息>
    std::map<int, UnitLevelInfo> _unitLevels;

    // 可训练的兵种ID列表
    std::vector<int> _availableUnits;

    // 回调函数
    std::function<void(int)> _onTrainComplete;
    std::function<void()> _onClose;

    // 状态
    bool _isShowing = false;

    // 初始化方法
    void setupBackground();
    void setupPanel();
    void setupTitle();
    void setupUnitCards();
    void setupCloseButton();
    void setupResourceDisplay();
    void initUnitLevels();

    // 招募兵种（消耗金币）
    void recruitUnit(int unitId);

    // 升级兵种（消耗钻石）
    void upgradeUnit(int unitId);

    // 更新资源显示
    void updateResourceDisplay();

    // 刷新兵种卡片显示
    void refreshUnitCards();

    // 检查是否有足够的金币招募
    bool canAffordRecruit(int unitId);

    // 检查是否有足够的钻石升级
    bool canAffordUpgrade(int unitId);

    // 检查兵种是否已满级
    bool isMaxLevel(int unitId);

    // 创建兵种卡片（带动画和按钮）
    Node* createUnitCard(int unitId);

    // 创建待机动画精灵
    Sprite* createIdleAnimationSprite(int unitId);
};

#endif // __TRAIN_PANEL_H__
