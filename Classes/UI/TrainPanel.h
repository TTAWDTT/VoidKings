// TrainPanel.h
// 兵种训练面板组件
// 用于在兵营中训练兵种，管理训练队列

#ifndef __TRAIN_PANEL_H__
#define __TRAIN_PANEL_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Soldier/UnitData.h"
#include "Soldier/UnitManager.h"
#include <vector>
#include <functional>

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
// 训练面板配置常量
// ===================================================
namespace TrainPanelConfig {
    // 面板尺寸
    const Size PANEL_SIZE = Size(500.0f, 400.0f);
    
    // 标题配置
    constexpr int TITLE_FONT_SIZE = 28;
    constexpr float TITLE_TOP_OFFSET = 30.0f;
    
    // 兵种按钮配置
    constexpr float UNIT_BUTTON_SIZE = 80.0f;
    constexpr float UNIT_BUTTON_SPACING = 20.0f;
    constexpr float UNIT_AREA_TOP_OFFSET = 80.0f;
    
    // 队列区域配置
    constexpr float QUEUE_AREA_HEIGHT = 100.0f;
    constexpr float QUEUE_ITEM_SIZE = 60.0f;
    constexpr float QUEUE_ITEM_SPACING = 10.0f;
    
    // 关闭按钮配置
    constexpr float CLOSE_BUTTON_BOTTOM = 30.0f;
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
    const std::map<int, int>& getTrainedUnits() const { return _trainedUnits; }
    
    // 清空已训练完成的兵种（派兵后调用）
    void clearTrainedUnits() { _trainedUnits.clear(); }
    
private:
    // UI组件
    LayerColor* _background = nullptr;      // 半透明背景
    LayerColor* _panel = nullptr;           // 面板主体
    Label* _titleLabel = nullptr;           // 标题
    Node* _unitButtonArea = nullptr;        // 兵种按钮区域
    Node* _queueArea = nullptr;             // 训练队列显示区域
    Label* _resourceLabel = nullptr;        // 资源显示
    
    // 训练队列
    std::vector<TrainQueueItem> _trainQueue;
    
    // 已训练完成的兵种 <兵种ID, 数量>
    std::map<int, int> _trainedUnits;
    
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
    void setupUnitButtons();
    void setupQueueArea();
    void setupCloseButton();
    void setupResourceDisplay();
    
    // 添加兵种到训练队列
    void addToTrainQueue(int unitId);
    
    // 从训练队列移除（取消训练）
    void removeFromTrainQueue(int index);
    
    // 更新队列显示
    void updateQueueDisplay();
    
    // 更新资源显示
    void updateResourceDisplay();
    
    // 检查资源是否足够
    bool canAffordUnit(int unitId);
    
    // 创建兵种按钮
    Button* createUnitButton(int unitId, const Vec2& position);
    
    // 创建队列项显示
    Node* createQueueItemNode(const TrainQueueItem& item, int index);
};

#endif // __TRAIN_PANEL_H__
