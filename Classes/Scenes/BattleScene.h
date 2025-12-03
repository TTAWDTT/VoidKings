// BattleScene.h
// 战斗场景
// 功能: 进攻敌方基地的战斗界面

#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "../Core/GameDefines.h"
#include <vector>
#include <map>

USING_NS_CC;

class Building;
class DefenseBuilding;
class Unit;
class Potion;

/**
 * @class BattleScene
 * @brief 战斗场景类
 * 
 * 战斗界面功能:
 * - 胜利百分比(摧毁敌方建筑占比)
 * - 可放置区域(敌方基地范围)
 * - 未/已放置兵种显示
 * - 逃跑按钮
 * - 资源偷取量实时显示
 * - 结算画面
 */
class BattleScene : public Scene {
public:
    static Scene* createScene();
    
    virtual bool init() override;
    
    // 帧更新
    virtual void update(float dt) override;
    
    CREATE_FUNC(BattleScene);

protected:
    // ==================== 初始化 ====================
    
    // 创建地图层
    void createMapLayer();
    
    // 创建敌方基地
    void createEnemyBase();
    
    // 创建UI层
    void createUILayer();
    
    // 创建兵种选择栏
    void createUnitBar();
    
    // 创建战斗信息显示
    void createBattleInfo();
    
    // ==================== 战斗操作 ====================
    
    // 部署单位
    void deployUnit(UnitType type, const Vec2& position);
    
    // 使用药水
    void usePotion(PotionType type, const Vec2& position);
    
    // 选择单位类型
    void selectUnitType(UnitType type);
    
    // 取消选择
    void deselectUnit();
    
    // ==================== 战斗逻辑 ====================
    
    // 开始战斗
    void startBattle();
    
    // 更新战斗状态
    void updateBattle(float dt);
    
    // 检查胜负条件
    void checkVictoryCondition();
    
    // 战斗结束
    void endBattle(bool victory);
    
    // 显示结算界面
    void showResultPanel(bool victory);
    
    // ==================== UI更新 ====================
    
    // 更新摧毁百分比
    void updateDestructionPercent();
    
    // 更新资源偷取量
    void updateLootDisplay();
    
    // 更新剩余兵种显示
    void updateRemainingUnits();
    
    // ==================== 输入处理 ====================
    
    bool onTouchBegan(Touch* touch, Event* event);
    void onTouchMoved(Touch* touch, Event* event);
    void onTouchEnded(Touch* touch, Event* event);
    
    // ==================== UI回调 ====================
    
    void onRetreat(Ref* sender);
    void onBackToBase(Ref* sender);
    
    // ==================== 辅助函数 ====================
    
    // 检查位置是否可放置单位
    bool canDeployAt(const Vec2& position);
    
    // 屏幕坐标转地图坐标
    Vec2 screenToMap(const Vec2& screenPos);
    
private:
    // 地图层
    Node* _mapLayer;
    Node* _buildingLayer;
    Node* _unitLayer;
    Node* _effectLayer;
    
    // UI层
    Node* _uiLayer;
    Node* _unitBar;
    Node* _battleInfoPanel;
    Node* _resultPanel;
    
    // 敌方建筑列表
    std::vector<Building*> _enemyBuildings;
    std::vector<DefenseBuilding*> _defenseBuildings;
    
    // 玩家单位列表
    std::vector<Unit*> _playerUnits;
    
    // 战斗数据
    int _totalBuildingHP;        // 敌方建筑总血量
    int _destroyedBuildingHP;    // 已摧毁建筑血量
    float _destructionPercent;   // 摧毁百分比
    int _goldLooted;             // 掠夺的金币
    int _elixirLooted;           // 掠夺的圣水
    bool _battleStarted;         // 战斗是否开始
    bool _battleEnded;           // 战斗是否结束
    
    // 可用兵种
    std::map<UnitType, int> _availableUnits;
    UnitType _selectedUnitType;
    bool _isUnitSelected;
    
    // 部署区域
    DrawNode* _deployZone;
    
    // UI标签
    Label* _percentLabel;
    Label* _goldLootLabel;
    Label* _elixirLootLabel;
    Label* _timeLabel;
    
    // 战斗计时
    float _battleTime;
    float _maxBattleTime;
    
    // 相机控制
    Vec2 _cameraOffset;
    Vec2 _lastTouchPos;
    bool _isDragging;
};

#endif // __BATTLE_SCENE_H__
