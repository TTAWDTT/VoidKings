/**
 * @file BattleScene.h
 * @brief 战斗场景头文件
 *
 * 战斗场景用于测试战斗系统，包含：
 * - 敌方基地和防御建筑
 * - 玩家部署的士兵
 * - 战斗逻辑处理
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
    constexpr float UI_TOP_HEIGHT = 50.0f;      // 顶部UI高度
    constexpr float UI_BOTTOM_HEIGHT = 80.0f;   // 底部部署区域高度

    // 战斗配置
    constexpr float BATTLE_TIME_LIMIT = 180.0f;  // 战斗时间限制（秒）
}

// ===================================================
// 战斗场景类
// ===================================================
class BattleScene : public Scene {
public:
    /**
     * @brief 创建战斗场景
     * @param levelId 关卡ID
     * @param units 可部署的兵种 <兵种ID, 数量>
     */
    static Scene* createScene(int levelId = 1, const std::map<int, int>& units = {});

    virtual bool init() override;
    virtual void update(float dt) override;

    CREATE_FUNC(BattleScene);

    // 设置关卡ID
    void setLevelId(int levelId) { _levelId = levelId; }

    // 设置可部署的兵种
    void setDeployableUnits(const std::map<int, int>& units) { _deployableUnits = units; }

private:
    // ==================== 核心组件 ====================
    GridMap* _gridMap = nullptr;                    // 网格地图
    Node* _buildingLayer = nullptr;                 // 建筑层
    Node* _soldierLayer = nullptr;                  // 士兵层
    Node* _uiLayer = nullptr;                       // UI层

    // ==================== 关卡数据 ====================
    int _levelId = 1;                               // 当前关卡ID
    std::map<int, int> _deployableUnits;            // 可部署的兵种 <ID, 数量>
    std::map<int, int> _remainingUnits;             // 剩余可部署的兵种

    // ==================== 战斗状态 ====================
    std::vector<Soldier*> _soldiers;                // 场上的士兵
    std::vector<Node*> _enemyBuildings;             // 敌方建筑
    float _battleTime = 0.0f;                       // 战斗时间
    bool _battleEnded = false;                      // 战斗是否结束
    int _destroyedBuildingCount = 0;                // 已摧毁的建筑数量
    int _totalBuildingCount = 0;                    // 总建筑数量

    // ==================== UI组件 ====================
    Label* _timerLabel = nullptr;                   // 计时器
    Label* _progressLabel = nullptr;                // 进度显示
    Node* _unitDeployArea = nullptr;                // 兵种部署区域

    // ==================== 初始化方法 ====================
    void initGridMap();
    void initLevel();
    void initUI();
    void initTouchListener();

    // ==================== 关卡初始化 ====================
    void createLevel1();  // 创建第1关
    void createEnemyBase(int gridX, int gridY);
    void createDefenseTower(int gridX, int gridY, int type);

    // ==================== 部署系统 ====================
    void setupDeployArea();
    void deploySoldier(int unitId, const Vec2& position);
    Node* createDeployButton(int unitId, int count, float x);

    // ==================== 战斗逻辑 ====================
    void updateBattle(float dt);
    void checkBattleEnd();
    void onBattleWin();
    void onBattleLose();

    // ==================== 回调 ====================
    void onExitButton(Ref* sender);
    bool onTouchBegan(Touch* touch, Event* event);
};

#endif // __BATTLE_SCENE_H__