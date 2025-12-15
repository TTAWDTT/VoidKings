#ifndef __BASE_SCENE_H__
#define __BASE_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Map/GridMap.h"
#include "Buildings/BuildingManager.h"

USING_NS_CC;
using namespace cocos2d::ui;

class BaseScene : public Scene {
public:
    static Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(BaseScene);

    void onAttackButton(Ref* sender);
    void onBuildButton(Ref* sender);
    void onExitButton(Ref* sender);

private:
    GridMap* _gridMap;
    Node* _buildingLayer;
    Node* _uiLayer;
    Node* _buildShopLayer;
    Node* _selectionLayer;
    
    bool _isPlacingMode;
    Node* _draggingBuilding;
    Sprite* _placementPreview;
    int _previewWidth;
    int _previewHeight;
    int _selectedBuildingType;
    int _selectedLevel;
    
    int _currentGold;
    int _currentDiamond;
    Label* _goldLabel;
    Label* _diamondLabel;
    
    // Store building configs as member variables
    ProductionBuildingConfig _baseConfig;
    
    void createGrassBackground();
    void createUI();
    void createBuildShop();
    void initBaseBuilding();
    void createSelection();

    void onLevelSelected(int level);
    void onBuildingSelected(int buildingType);
    void updatePlacementPreview();
    void confirmPlacement();
    void cancelPlacement();
    
    bool onTouchBegan(Touch* touch, Event* event);
    void onTouchMoved(Touch* touch, Event* event);
    void onTouchEnded(Touch* touch, Event* event);
};

#endif // __BASE_SCENE_H__
