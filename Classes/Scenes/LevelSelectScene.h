// LevelSelectScene.h
// 关卡选择场景
// 独立的关卡选择界面，展示关卡按钮、兵种预览和背景

#ifndef __LEVEL_SELECT_SCENE_H__
#define __LEVEL_SELECT_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>
#include <map>

USING_NS_CC;
using namespace cocos2d::ui;

// ===================================================
// 关卡选择场景配置（优化后的紧凑布局）
// ===================================================
namespace LevelSelectConfig {
    // 关卡按钮配置 - 缩小尺寸使布局更紧凑
    constexpr int MAX_LEVELS = 5;                    // 最大关卡数
    constexpr float LEVEL_BUTTON_SIZE = 50.0f;       // 关卡按钮尺寸（缩小）
    constexpr float LEVEL_BUTTON_SPACING = 20.0f;    // 关卡按钮间距（缩小）
    constexpr float LEVEL_AREA_Y_RATIO = 0.55f;      // 关卡区域Y位置（相对屏幕高度）
    
    // 兵种预览区域配置 - 缩小尺寸
    constexpr float UNIT_PREVIEW_HEIGHT = 80.0f;     // 兵种预览区域高度（缩小）
    constexpr float UNIT_PREVIEW_BOTTOM = 10.0f;     // 距离底部距离（缩小）
    constexpr float UNIT_ICON_SIZE = 40.0f;          // 兵种图标尺寸（缩小）
    constexpr float UNIT_ICON_SPACING = 10.0f;       // 兵种图标间距（缩小）
    
    // 标题配置 - 缩小字体
    constexpr int TITLE_FONT_SIZE = 24;              // 标题字体大小（缩小）
    constexpr float TITLE_TOP_OFFSET = 30.0f;        // 标题顶部偏移（缩小）
    
    // 退出按钮配置 - 缩小尺寸
    constexpr float EXIT_BUTTON_SIZE = 30.0f;        // 退出按钮尺寸（缩小）
    constexpr float EXIT_BUTTON_MARGIN = 15.0f;      // 退出按钮边距（缩小）
}

// ===================================================
// 关卡信息结构
// ===================================================
struct LevelInfo {
    int levelId;                    // 关卡ID
    std::string name;               // 关卡名称
    bool isUnlocked;                // 是否解锁
    int starCount;                  // 已获得星星数（0-3）
    std::string description;        // 关卡描述
};

// ===================================================
// 兵种预览项
// ===================================================
struct UnitPreviewItem {
    int unitId;                     // 兵种ID
    int count;                      // 数量
};

// ===================================================
// 关卡选择场景类
// ===================================================
class LevelSelectScene : public Scene {
public:
    static Scene* createScene();
    static Scene* createScene(const std::map<int, int>& selectedUnits);
    
    virtual bool init() override;
    CREATE_FUNC(LevelSelectScene);
    
    // 设置已选择的兵种（从训练面板获取）
    void setSelectedUnits(const std::map<int, int>& units);

private:
    // UI组件
    Sprite* _background = nullptr;              // 背景图
    Label* _titleLabel = nullptr;               // 标题
    Node* _levelButtonArea = nullptr;           // 关卡按钮区域
    Node* _unitPreviewArea = nullptr;           // 兵种预览区域
    Button* _exitButton = nullptr;              // 退出按钮
    
    // 关卡数据
    std::vector<LevelInfo> _levels;
    
    // 已选择的兵种 <兵种ID, 数量>
    std::map<int, int> _selectedUnits;
    
    // 当前选中的关卡
    int _selectedLevel = 0;
    
    // 初始化方法
    void setupBackground();
    void setupTitle();
    void setupLevelButtons();
    void setupUnitPreview();
    void setupExitButton();
    
    // 初始化关卡数据
    void initLevelData();
    
    // 创建单个关卡按钮
    Node* createLevelButton(const LevelInfo& level, int index);
    
    // 创建兵种预览图标
    Node* createUnitPreviewIcon(int unitId, int count);
    
    // 更新兵种预览显示
    void updateUnitPreview();
    
    // 关卡选择回调
    void onLevelSelected(int levelId);
    
    // 开始战斗
    void startBattle(int levelId);
    
    // 退出回调
    void onExitButton(Ref* sender);
};

#endif // __LEVEL_SELECT_SCENE_H__
