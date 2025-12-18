#ifndef __BASE_SCENE_H__
#define __BASE_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Map/GridMap.h"
#include "Buildings/BuildingManager.h"
#include "UI/TrainPanel.h"

USING_NS_CC;
using namespace cocos2d::ui;

// ǰ������
class TrainPanel;

class BaseScene : public Scene {
public:
    static Scene* createScene();
    // ����������һ���µĳ���ʵ����ͨ����Ϊ�����л�ʱ����ں���ʹ�á�
    virtual bool init() override;
    // ��ʼ��������������ͼ��UI���������ӽڵ㲢�����¼������������Ƿ��ʼ���ɹ���
    CREATE_FUNC(BaseScene);

    void onAttackButton(Ref* sender);
    // ������ť�ص�����Ӧ���������ť���ռ���ѵ�����Ӳ��л����ؿ�ѡ�񳡾���
    void onBuildButton(Ref* sender);
    // ���찴ť�ص�����ʾ�����̵�����Թ����ѡ����Ľ�����
    void onExitButton(Ref* sender);
    // �˳���ť�ص����������˵����˳���ǰ������

private:
    GridMap* _gridMap = nullptr;
    Node* _buildingLayer = nullptr;
    Node* _uiLayer = nullptr;
    Node* _buildShopLayer = nullptr;
    
    // ѵ����壨�����Ӫ����ʱ��ʾ��
    TrainPanel* _trainPanel = nullptr;
    
    bool _isPlacingMode = false;
    Node* _draggingBuilding = nullptr;
    Sprite* _placementPreview = nullptr;
    int _previewWidth = 0;
    int _previewHeight = 0;
    int _selectedBuildingType = 0;
    int _selectedLevel = 0;
    
    int _currentGold = 0;
    int _currentDiamond = 0;
    Label* _goldLabel = nullptr;
    Label* _diamondLabel = nullptr;
    
    // Store building configs as member variables
    ProductionBuildingConfig _baseConfig;
    
    void createGrassBackground();
    // �����ݵر�����ͼ�����������ͼ��������Ϊ����ͼ�㡣
    void createUI();
    Node* createTooltip(const std::string& text,const Size& size) ;
    void bindTooltip(
        Node* owner,
        Node* targetBtn,
        Node* tooltip,
        float offsetX = 10.0f
    );
    // ������������Ϸ��UI����ť����Դ��ʾ�ȣ�����UI�ڵ����ӵ������С�
    void createBuildShop();
    // ���������̵���沢��ʼ������ѡ���ص�����Ĭ�ϲ���ʾ��
    void createTrainPanel();
    // ��������ѵ����岢����ѵ����ɺ͹رյĻص��������������Ϊ�ӽڵ����ӵ�������
    void initBaseBuilding();
    // ��ʼ������Ĭ�Ͻ�����������غͳ�ʼ��Ӫ��������������ڵ�ͼָ��λ�á�

    void onBuildingSelected(int buildingType);
    // ��������̵���ѡ��ĳ�ֽ���ʱ���ã��������ģʽ����������Ԥ�����顣
    void updatePlacementPreview();
    // ���·���Ԥ��λ�ú���ɫ�����ڵ�ǰλ���Ƿ�ɷ������ı��Ӿ�������
    void confirmPlacement();
    // ȷ�Ϸ��ý�������Ԥ���л�Ϊʵ�ʽ������۳���Դ������������Ԥ����
    void cancelPlacement();
    // ȡ������ģʽ���Ƴ�Ԥ�����ָ��������������������ߡ�
    
    // ��ѵ����壨�����Ӫʱ���ã�
    void showTrainPanel();
    // ��/��ʾѵ����壨�������Ӫ����ʱ���ã���
    
    // ѵ����ɻص�
    void onUnitTrainComplete(int unitId);
    // ��������ѵ������¼���unitId ��ʾѵ����ɵı���ID���������ڸ���UI����С�
    
    bool onTouchBegan(Touch* touch, Event* event);
    // ������ʼ�¼����������������������ѵ������������ģʽ�Ƚ����߼���
    void onTouchMoved(Touch* touch, Event* event);
    // �����ƶ��¼��������ڷ���ģʽ���ƶ�Ԥ����ʵʱ����Ƿ�ɷ��á�
    void onTouchEnded(Touch* touch, Event* event);
    // ���������¼��������ڷ���ģʽ�½�������λ�ü�Ⲣȷ�ϻ�ȡ�����á�
};

#endif // __BASE_SCENE_H__
