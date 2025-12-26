/**
 * @file BattleScene.h
 * @brief 战斗场景头文件
 *
 * 战斗场景用于测试战斗系统，包括：
 * - 敌方基地和防御建筑
 * - 玩家部署士兵
 * - 战斗逻辑更新
 */

#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Map/GridMap.h"
#include "Soldier/Soldier.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include <vector>
#include <map>

USING_NS_CC;
using namespace cocos2d::ui;

// ===================================================
// 战斗场景配置
// ===================================================
namespace BattleConfig {
    // 网格配置
    constexpr int GRID_WIDTH = 40;          // 战斗地图宽度（格子数）
    constexpr int GRID_HEIGHT = 30;         // 战斗地图高度（格子数）
    constexpr float CELL_SIZE = 32.0f;      // 每格像素大小

    // UI配置
    constexpr float UI_TOP_HEIGHT = 60.0f;      // 顶部UI高度（稍微增加）
    constexpr float UI_BOTTOM_HEIGHT = 100.0f;   // 底部部署区域高度（增加以容纳更大图标）
    constexpr float DEPLOY_BUTTON_SIZE = 50.0f;  // 部署按钮尺寸
    constexpr float DEPLOY_BUTTON_SPACING = 16.0f; // 部署按钮间距

    // 部署范围配置（使用格子坐标）
    constexpr int DEPLOY_MIN_X = 2;             // 允许部署的最小列
    constexpr int DEPLOY_MAX_X = 10;            // 允许部署的最大列
    constexpr int DEPLOY_MIN_Y = 2;             // 允许部署的最小行
    constexpr int DEPLOY_MAX_Y = GRID_HEIGHT - 3; // 允许部署的最大行

    // 战斗配置
    constexpr float BATTLE_TIME_LIMIT = 160.0f;  // 战斗时间限制（秒）
}

enum class BattleMode {
    Attack,
    Defense
};

// ===================================================
// 战斗场景类
// ===================================================
class BattleScene : public Scene {
public:
    /**
     * @brief 创建战斗场景
     * @param levelId 关卡ID
     * @param units 可部署的单位 <单位ID, 数量>
     * @param useDefaultUnits 是否允许使用默认兵种
     */
    static Scene* createScene(int levelId = 1, const std::map<int, int>& units = {}, bool useDefaultUnits = true, bool defenseMode = false);

    virtual bool init() override;
    virtual void update(float dt) override;
    virtual void onExit() override;

    CREATE_FUNC(BattleScene);

    // 设置关卡ID
    void setLevelId(int levelId) { _levelId = levelId; }
    void setBattleMode(BattleMode mode) { _battleMode = mode; }

    // 设置可部署的单位
    void setDeployableUnits(const std::map<int, int>& units) {
        _deployableUnits = units;
    }

private:
    // ==================== 核心组件 ====================
    GridMap* _gridMap = nullptr;                    // 网格地图
    Node* _buildingLayer = nullptr;                 // 建筑层
    Node* _soldierLayer = nullptr;                  // 士兵层
    Node* _uiLayer = nullptr;                       // UI层

    // ==================== 关卡数据 ====================
    int _levelId = 1;                               // 当前关卡ID
    std::map<int, int> _deployableUnits;            // 可部署的单位 <ID, 数量>
    std::map<int, int> _remainingUnits;             // 剩余可部署的单位
    bool _allowDefaultUnits = true;                 // 是否允许使用默认兵种（未传入训练兵种时）
    BattleMode _battleMode = BattleMode::Attack;    // 战斗模式

    struct DefenseSpawn {
        float time = 0.0f;
        int unitId = 0;
    };
    std::vector<DefenseSpawn> _defenseSpawns;
    size_t _nextDefenseSpawnIndex = 0;
    float _defenseSpawnCursor = 0.0f;
    int _defenseTotalUnits = 0;

    // ==================== 战斗状态 ====================
    std::vector<Soldier*> _soldiers;                // 场上的士兵
    std::vector<Node*> _enemyBuildings;             // 敌方建筑
    Node* _enemyBase = nullptr;                     // 敌方基地
    bool _enemyBaseDestroyed = false;               // 敌方基地是否已摧毁
    float _battleTime = 0.0f;                       // 战斗时间
    bool _battleEnded = false;                      // 战斗是否结束
    int _destroyedBuildingCount = 0;                // 已摧毁的建筑数量
    int _totalBuildingCount = 0;                    // 总建筑数量
    int _totalDeployedCount = 0;                    // 已部署的总兵种数
    int _deadSoldierCount = 0;                      // 已死亡的兵种数

    // ==================== UI组件 ====================
    Label* _timerLabel = nullptr;                   // 计时器
    Label* _progressLabel = nullptr;                // 进度显示
    Node* _unitDeployArea = nullptr;                // 单位部署区域
    std::map<int, Node*> _deployButtons;            // 部署按钮缓存
    int _selectedUnitId = -1;                       // 当前选中的单位ID

    // ==================== 悬浮信息 ====================
    Node* _hoverInfoPanel = nullptr;                // 悬浮信息面板
    LayerColor* _hoverInfoBg = nullptr;             // 悬浮信息背景
    Label* _hoverInfoLabel = nullptr;               // 悬浮信息文字
    DrawNode* _hoverRangeNode = nullptr;            // 攻击范围虚线
    DrawNode* _hoverFootprintNode = nullptr;        // 占地实线框
    Node* _hoveredBuilding = nullptr;               // 当前悬浮建筑
    Node* _resultLayer = nullptr;                   // 结算展示层

    // ==================== 初始化方法 ====================
    void initGridMap();
    void initLevel();
    void initUI();
    void initTouchListener();
    void initHoverInfo();

    // ==================== 关卡初始化 ====================
    void createLevel1();  // 创建第1关
    void createLevel2();
    void createLevel3();
    void createLevel4();
    void createLevel5();
    void createLevel6();
    void createLevel7();
    void createLevel8();
    void createLevel9();
    void createLevel10();
    void createLevel11();
    void createLevel12();
    void createDefenseLevel1();
    void createDefenseLevel2();
    void createDefenseLevel3();
    void createDefenseLevel4();
    void createDefenseLevel5();
    void createDefenseLevel6();
    void createDefenseBaseLayout(int towerLevel);
    void createEnemyBase(int gridX, int gridY, int level = 0);
    void createDefenseTower(int gridX, int gridY, int type, int level = 0);
    void createSpikeTrap(int gridX, int gridY);
    void createSnapTrap(int gridX, int gridY);
    void addTrapCluster(int startX, int startY, int width, int height, const std::vector<Vec2>& snapCells);

    // ==================== 辅助方法 ====================
    /**
     * @brief 缩放建筑以适应格子大小
     * @param building 建筑节点
     * @param gridWidth 格子宽度
     * @param gridHeight 格子高度
     * @param cellSize 单个格子的像素尺寸
     */
    void scaleBuildingToFit(Node* building, int gridWidth, int gridHeight, float cellSize);

    // ==================== 部署系统 ====================
    void setupDeployArea();
    void deploySoldier(int unitId, const Vec2& position);
    void spawnEnemySoldier(int unitId, const Vec2& position, int level);
    void spawnDeployEffect(const Vec2& position);
    Node* createDeployButton(int unitId, int count, float x);
    Sprite* createUnitIdleIcon(int unitId, float targetSize, bool forceAnimate = false);
    int getFirstAvailableUnitId() const;
    void setSelectedUnit(int unitId);
    void refreshDeployButton(int unitId);
    // 部署范围提示
    void setupDeployRangeHint();
    // 检查格子是否在允许部署范围内
    bool isDeployGridAllowed(int gridX, int gridY) const;

    // ==================== 悬浮信息 ====================
    void updateHoverInfo(const Vec2& worldPos);
    Node* pickBuildingAt(const Vec2& worldPos) const;
    void showBuildingInfo(Node* building);
    void clearBuildingInfo();
    void updateHoverOverlays(Node* building);
    void updateHoverPanelPosition(const Vec2& worldPos);

    // ==================== 战斗逻辑 ====================
    void updateBattle(float dt);
    void updateDefenseSpawns();
    void checkBattleEnd();
    void onBattleWin();
    void onBattleLose();
    int calculateStarCount() const;
    void showBattleResult(bool isWin, int stars);
    void createResultButtons(Node* parent, bool isWin);

    int getDefenseLevelIndex() const;
    int getRewardLevel() const;
    int getRecordLevelId() const;
    void resetDefenseSpawns();
    void addDefenseWave(int unitId, int count, float interval, float delay);
    Vec2 getDefenseSpawnPosition(int index) const;

    // ==================== 回调 ====================
    void onExitButton(Ref* sender);
    bool onTouchBegan(Touch* touch, Event* event);
};

#endif // __BATTLE_SCENE_H__
