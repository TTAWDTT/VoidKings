/**
 * @file BattleScene.h
 * @brief ս������ͷ�ļ�
 *
 * ս���������ڲ���ս��ϵͳ��������
 * - �з����غͷ�������
 * - ��Ҳ����ʿ��
 * - ս���߼�����
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
// ս����������
// ===================================================
namespace BattleConfig {
    // ��������
    constexpr int GRID_WIDTH = 40;          // ս����ͼ���ȣ���������
    constexpr int GRID_HEIGHT = 30;         // ս����ͼ�߶ȣ���������
    constexpr float CELL_SIZE = 32.0f;      // ÿ�����ش�С

    // UI����
    constexpr float UI_TOP_HEIGHT = 50.0f;      // ����UI�߶�
    constexpr float UI_BOTTOM_HEIGHT = 80.0f;   // �ײ���������߶�

    // ս������
    constexpr float BATTLE_TIME_LIMIT = 180.0f;  // ս��ʱ�����ƣ��룩
}

// ===================================================
// ս��������
// ===================================================
class BattleScene : public Scene {
public:
    /**
     * @brief ����ս������
     * @param levelId �ؿ�ID
     * @param units �ɲ���ı��� <����ID, ����>
     */
    static Scene* createScene(int levelId = 1, const std::map<int, int>& units = {});

    virtual bool init() override;
    virtual void update(float dt) override;

    CREATE_FUNC(BattleScene);

    // ���ùؿ�ID
    void setLevelId(int levelId) { _levelId = levelId; }

    // ���ÿɲ���ı���
    void setDeployableUnits(const std::map<int, int>& units) { _deployableUnits = units; }

private:
    // ==================== ������� ====================
    GridMap* _gridMap = nullptr;                    // �����ͼ
    Node* _buildingLayer = nullptr;                 // ������
    Node* _soldierLayer = nullptr;                  // ʿ����
    Node* _uiLayer = nullptr;                       // UI��

    // ==================== �ؿ����� ====================
    int _levelId = 1;                               // ��ǰ�ؿ�ID
    std::map<int, int> _deployableUnits;            // �ɲ���ı��� <ID, ����>
    std::map<int, int> _remainingUnits;             // ʣ��ɲ���ı���

    // ==================== ս��״̬ ====================
    std::vector<Soldier*> _soldiers;                // ���ϵ�ʿ��
    std::vector<Node*> _enemyBuildings;             // �з�����
    float _battleTime = 0.0f;                       // ս��ʱ��
    bool _battleEnded = false;                      // ս���Ƿ����
    int _destroyedBuildingCount = 0;                // �ѴݻٵĽ�������
    int _totalBuildingCount = 0;                    // �ܽ�������

    // ==================== UI��� ====================
    Label* _timerLabel = nullptr;                   // ��ʱ��
    Label* _progressLabel = nullptr;                // ������ʾ
    Node* _unitDeployArea = nullptr;                // ���ֲ�������

    // ==================== ��ʼ������ ====================
    void initGridMap();
    void initLevel();
    void initUI();
    void initTouchListener();

    // ==================== 关卡初始化 ====================
    void createLevel1();  // 创建第1关
    void createEnemyBase(int gridX, int gridY);
    void createDefenseTower(int gridX, int gridY, int type);

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