/**
 * @file BaseScene.cpp
 * @brief 基地场景实现文件
 */

#include "BaseScene.h"
#include "MainMenuScene.h"
#include "LevelSelectScene.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Buildings/StorageBuilding.h"
#include "UI/TrainPanel.h"
#include "Soldier/UnitManager.h"
#include "UI/IDCardPanel.h"

USING_NS_CC;

// ==================== 场景创建与初始化 ====================

Scene* BaseScene::createScene() {
    return BaseScene::create();
}

bool BaseScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 初始化状态变量
    _isPlacingMode = false;
    _draggingBuilding = nullptr;
    _placementPreview = nullptr;
    _currentGold = 1000;
    _currentDiamond = 100;
    _selectedBuildingType = 0;
    _selectedLevel = 0; 

    // 创建网格地图（40x40格子，每格32像素）
    _gridMap = GridMap::create(40, 40, 32.0f);
    _gridMap->setPosition(origin);
    this->addChild(_gridMap, 0);

    // 创建建筑层（作为GridMap的子节点）
    _buildingLayer = Node::create();
    _gridMap->addChild(_buildingLayer, 10);

    // 创建网格背景（黑白虚线风格）
    createGridBackground();

    // 初始化基地核心建筑
    initBaseBuilding();

    // 创建UI界面
    createUI();

    // 创建建筑商店（初始隐藏）
    createBuildShop();
    _buildShopLayer->setVisible(false);
    
    // 创建训练面板
    createTrainPanel();

    // 加载兵种配置
    UnitManager::getInstance()->loadConfig("res/units_config.json");

    // 注册触摸事件监听器
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BaseScene::onTouchBegan, this); 
    listener->onTouchMoved = CC_CALLBACK_2(BaseScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BaseScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

// ==================== 网格背景创建 ====================

void BaseScene::createGridBackground() {
    // 创建黑色底色背景
    auto bgColor = LayerColor::create(Color4B(30, 30, 30, 255));
    _gridMap->addChild(bgColor, -2);
    
    // 创建网格线绘制节点
    _gridBackgroundNode = DrawNode::create();
    _gridMap->addChild(_gridBackgroundNode, -1);
    
    float cellSize = _gridMap->getCellSize();
    int gridWidth = _gridMap->getGridWidth();
    int gridHeight = _gridMap->getGridHeight();
    
    // 绘制虚线网格
    // 虚线参数：线段长度和间隔
    float dashLength = 4.0f;
    float gapLength = 4.0f;
    Color4F lineColor(0.5f, 0.5f, 0.5f, 0.6f);  // 灰白色
    
    // 绘制垂直虚线
    for (int x = 0; x <= gridWidth; ++x) {
        float xPos = x * cellSize;
        float y = 0;
        while (y < gridHeight * cellSize) {
            float endY = std::min(y + dashLength, gridHeight * cellSize);
            _gridBackgroundNode->drawLine(
                Vec2(xPos, y), 
                Vec2(xPos, endY), 
                lineColor
            );
            y += dashLength + gapLength;
        }
    }
    
    // 绘制水平虚线
    for (int y = 0; y <= gridHeight; ++y) {
        float yPos = y * cellSize;
        float x = 0;
        while (x < gridWidth * cellSize) {
            float endX = std::min(x + dashLength, gridWidth * cellSize);
            _gridBackgroundNode->drawLine(
                Vec2(x, yPos), 
                Vec2(endX, yPos), 
                lineColor
            );
            x += dashLength + gapLength;
        }
    }
    
    // 创建放置状态网格显示节点（初始隐藏）
    _placementGridNode = DrawNode::create();
    _gridMap->addChild(_placementGridNode, 5);
    _placementGridNode->setVisible(false);
}

// ==================== UI创建 ====================

Node* BaseScene::createTooltip(const std::string& text, const Size& size) { 
    auto panel = Node::create(); 
    panel->setContentSize(size); 
    panel->setAnchorPoint(Vec2(0, 0.5f)); 
    
    auto bg = LayerColor::create(Color4B(0, 0, 0, 180), size.width, size.height); 
    bg->setPosition(Vec2::ZERO); 
    panel->addChild(bg); 
    
    auto label = Label::createWithTTF(text, "fonts/ScienceGothic.ttf", 18); 
    label->setWidth(size.width - 10); 
    label->setAlignment(TextHAlignment::LEFT); 
    label->setPosition(Vec2(size.width / 2, size.height / 2)); 
    panel->addChild(label);
    
    return panel; 
}

void BaseScene::bindTooltip(Node* owner, Node* targetBtn, Node* tooltip, float offsetX) {
    tooltip->setVisible(false);
    owner->addChild(tooltip, 999);

    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = [=](EventMouse* event) {
        Vec2 mouseWorldPos(event->getCursorX(), event->getCursorY());

        if (targetBtn->getBoundingBox().containsPoint(mouseWorldPos)) {
            float btnRight = targetBtn->getPositionX() +
                targetBtn->getContentSize().width * targetBtn->getScale() / 2;
            tooltip->setPosition(Vec2(btnRight + offsetX, targetBtn->getPositionY()));
            tooltip->setVisible(true);
        } else {
            tooltip->setVisible(false);
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, owner);
}

void BaseScene::createUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _uiLayer = Node::create();
    this->addChild(_uiLayer, 100);

    // UI按钮布局参数
    float panelX = origin.x + 20;
    float buttonSpacing = 30;
    float startY = visibleSize.height / 2 + 100;

    // 进攻按钮
    auto attackBtn = Button::create("UI/attack.png");
    attackBtn->setScale(4.0f);
    attackBtn->setPosition(Vec2(panelX, startY - buttonSpacing));
    attackBtn->addClickEventListener([this](Ref* sender) { this->onAttackButton(sender); });
    _uiLayer->addChild(attackBtn);
    auto attackTip = createTooltip("Attack", Size(80, 30));
    bindTooltip(this, attackBtn, attackTip);

    // 建造按钮
    auto buildBtn = Button::create("UI/build.png");
    buildBtn->setScale(4.0f);
    buildBtn->setPosition(Vec2(panelX, startY - buttonSpacing * 2));
    buildBtn->addClickEventListener([this](Ref* sender) { this->onBuildButton(sender); });
    _uiLayer->addChild(buildBtn);
    auto buildTip = createTooltip("Build", Size(60, 30));
    bindTooltip(this, buildBtn, buildTip);

    // 退出按钮
    auto exitBtn = Button::create("UI/exit.png");
    exitBtn->setScale(4.0f);
    exitBtn->setPosition(Vec2(panelX, startY - buttonSpacing * 3));
    exitBtn->addClickEventListener([this](Ref* sender) { this->onExitButton(sender); });
    _uiLayer->addChild(exitBtn);
    auto exitTip = createTooltip("Exit", Size(60, 30));
    bindTooltip(this, exitBtn, exitTip);
    
    // 资源显示面板
    auto idCard = IDCardPanel::createPanel(_uiLayer);
    idCard->setPosition(Vec2(panelX + 50, startY + 20));
}

// ==================== 建筑商店创建 ====================

void BaseScene::createBuildShop() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _buildShopLayer = Node::create();
    this->addChild(_buildShopLayer, 101);

    // 半透明黑色背景遮罩
    auto bg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    _buildShopLayer->addChild(bg);

    // 商店面板
    auto shopPanel = LayerColor::create(Color4B(50, 50, 50, 255), 400, 500);
    shopPanel->setPosition(Vec2(visibleSize.width / 2 - 200, visibleSize.height / 2 - 250));
    _buildShopLayer->addChild(shopPanel);

    // 标题
    auto titleLabel = Label::createWithSystemFont("Building Shop", "ScienceGothic", 28);
    titleLabel->setPosition(Vec2(200, 460));
    shopPanel->addChild(titleLabel);

    float yPos = 400;
    float spacing = 60;

    // 建筑选项配置
    struct BuildingOption {
        const char* name;
        int type;
        int cost;
    };

    BuildingOption buildings[] = {
        {"Arrow Tower (100G)", 1, 100},
        {"Boom Tower (150G)", 2, 150},
        {"Tree (80G)", 3, 80},
        {"Snowman Storage (200G)", 4, 200},
        {"Soldier Builder (300G)", 5, 300}
    };

    // 创建建筑选择按钮
    for (int i = 0; i < 5; ++i) {
        auto btn = Button::create("btn_normal.png", "btn_pressed.png");
        btn->setTitleText(buildings[i].name);
        btn->setTitleFontSize(18);
        btn->setScale(1.5f);
        btn->setPosition(Vec2(200, yPos));
        int buildingType = buildings[i].type;
        btn->addClickEventListener([this, buildingType](Ref* sender) {
            this->onBuildingSelected(buildingType);
        });
        shopPanel->addChild(btn);
        yPos -= spacing;
    }

    // 关闭按钮
    auto closeBtn = Button::create("btn_normal.png", "btn_pressed.png");
    closeBtn->setTitleText("Close");
    closeBtn->setTitleFontSize(24);
    closeBtn->setPosition(Vec2(200, 50));
    closeBtn->addClickEventListener([this](Ref* sender) {
        _buildShopLayer->setVisible(false);
    });
    shopPanel->addChild(closeBtn);
}

// ==================== 训练面板创建 ====================

void BaseScene::createTrainPanel() {
    _trainPanel = TrainPanel::create(
        // 训练完成回调
        [this](int unitId) {
            this->onUnitTrainComplete(unitId);
        },
        // 关闭回调
        [this]() {
            CCLOG("[基地场景] 训练面板关闭");
        }
    );
    
    if (_trainPanel) {
        this->addChild(_trainPanel, 102);
        CCLOG("[基地场景] 训练面板创建成功");
    }
}

void BaseScene::showTrainPanel() {
    if (_trainPanel) {
        _trainPanel->show();
        CCLOG("[基地场景] 打开训练面板");
    }
}

void BaseScene::onUnitTrainComplete(int unitId) {
    CCLOG("[基地场景] 兵种训练完成: ID=%d", unitId);
}

// ==================== 基地建筑初始化 ====================

void BaseScene::initBaseBuilding() {
    // 初始化基地建筑配置
    _baseConfig.id = 3001;
    _baseConfig.name = "Base";
    _baseConfig.spriteFrameName = "buildings/base.png";
    _baseConfig.HP = {2000, 2500, 3000};
    _baseConfig.DP = {0, 0.05f, 0.1f};
    _baseConfig.length = 4;
    _baseConfig.width = 4;
    _baseConfig.MAXLEVEL = 2;
    _baseConfig.PRODUCE_GOLD = {0, 0, 0};
    _baseConfig.PRODUCE_ELIXIR = {0, 0, 0};
    _baseConfig.STORAGE_GOLD_CAPACITY = {1000, 2000, 4000};
    _baseConfig.STORAGE_ELIXIR_CAPACITY = {500, 1000, 2000};

    auto base = ProductionBuilding::create(&_baseConfig, 0);
    if (base) {
        _buildingLayer->addChild(base);
        BuildingManager::getInstance()->setGridMap(_gridMap);
        // 将基地放置在地图中心位置
        BuildingManager::getInstance()->placeBuilding(base, 18, 18, 4, 4);
    }
}

// ==================== 按钮回调 ====================

void BaseScene::onAttackButton(Ref* sender) {
    CCLOG("[基地场景] 点击进攻按钮，跳转到关卡选择");
    
    // 获取已训练的兵种
    std::map<int, int> trainedUnits;
    if (_trainPanel) {
        trainedUnits = _trainPanel->getTrainedUnits();
    }
    
    // 跳转到关卡选择场景
    auto scene = LevelSelectScene::createScene(trainedUnits);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void BaseScene::onBuildButton(Ref* sender) {
    _buildShopLayer->setVisible(true);
}

void BaseScene::onExitButton(Ref* sender) {
    auto scene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// ==================== 建筑放置相关 ====================

Vec2 BaseScene::calculateBuildingPosition(int gridX, int gridY, int width, int height) {
    // 计算建筑中心位置：左下角格子位置 + 建筑尺寸的一半
    float cellSize = _gridMap->getCellSize();
    float centerX = (gridX + width * 0.5f) * cellSize;
    float centerY = (gridY + height * 0.5f) * cellSize;
    return Vec2(centerX, centerY);
}

void BaseScene::updatePlacementGrid(int gridX, int gridY, bool canPlace) {
    if (!_placementGridNode) return;
    
    _placementGridNode->clear();
    _placementGridNode->setVisible(true);
    
    float cellSize = _gridMap->getCellSize();
    
    // 根据可放置状态选择颜色
    Color4F fillColor = canPlace ? 
        Color4F(0.0f, 0.8f, 0.0f, 0.4f) :   // 绿色（可放置）
        Color4F(0.8f, 0.0f, 0.0f, 0.4f);    // 红色（不可放置）
    
    Color4F borderColor = canPlace ?
        Color4F(0.0f, 1.0f, 0.0f, 0.8f) :
        Color4F(1.0f, 0.0f, 0.0f, 0.8f);
    
    // 绘制建筑占用的每个格子
    for (int dy = 0; dy < _previewHeight; ++dy) {
        for (int dx = 0; dx < _previewWidth; ++dx) {
            int cellX = gridX + dx;
            int cellY = gridY + dy;
            
            Vec2 bottomLeft(cellX * cellSize, cellY * cellSize);
            Vec2 topRight((cellX + 1) * cellSize, (cellY + 1) * cellSize);
            
            // 绘制填充矩形
            _placementGridNode->drawSolidRect(bottomLeft, topRight, fillColor);
            
            // 绘制边框
            Vec2 vertices[] = {
                bottomLeft,
                Vec2(topRight.x, bottomLeft.y),
                topRight,
                Vec2(bottomLeft.x, topRight.y)
            };
            _placementGridNode->drawPolygon(vertices, 4, Color4F(0, 0, 0, 0), 1.0f, borderColor);
        }
    }
}

void BaseScene::onBuildingSelected(int buildingType) {
    _selectedBuildingType = buildingType;
    _buildShopLayer->setVisible(false);
    _isPlacingMode = true;

    // 移除旧的预览
    if (_placementPreview) {
        _placementPreview->removeFromParent();
        _placementPreview = nullptr;
    }

    std::string spritePath;
    _previewWidth = 1;
    _previewHeight = 1;

    // 根据建筑类型设置预览图和尺寸
    switch (buildingType) {
        case 1: // 箭塔
            spritePath = "buildings/ArrowTower.png";
            _previewWidth = 2;
            _previewHeight = 2;
            break;
        case 2: // 炮塔
            spritePath = "buildings/BoomTower.png";
            _previewWidth = 2;
            _previewHeight = 2;
            break;
        case 3: // 树
            spritePath = "buildings/Tree/sprite_0000.png";
            _previewWidth = 2;
            _previewHeight = 2;
            break;
        case 4: // 雪人仓库
            spritePath = "buildings/snowman.png";
            _previewWidth = 2;
            _previewHeight = 2;
            break;
        case 5: // 兵营
            spritePath = "buildings/soldierbuilder.png";
            _previewWidth = 3;
            _previewHeight = 3;
            break;
    }

    _placementPreview = Sprite::create(spritePath);
    if (_placementPreview) {
        _placementPreview->setOpacity(180);
        _gridMap->addChild(_placementPreview, 50);
    }
    
    // 显示网格辅助线
    _gridMap->showGrid(true);
}

void BaseScene::updatePlacementPreview() {
    // 预览更新在onTouchMoved中处理
}

void BaseScene::confirmPlacement() {
    _isPlacingMode = false;
    if (_placementPreview) {
        _placementPreview->removeFromParent();
        _placementPreview = nullptr;
    }
    // 隐藏网格辅助线和放置网格
    _gridMap->showGrid(false);
    if (_placementGridNode) {
        _placementGridNode->setVisible(false);
    }
}

void BaseScene::cancelPlacement() {
    _isPlacingMode = false;
    if (_placementPreview) {
        _placementPreview->removeFromParent();
        _placementPreview = nullptr;
    }
    // 隐藏网格辅助线和放置网格
    _gridMap->showGrid(false);
    if (_placementGridNode) {
        _placementGridNode->setVisible(false);
    }
}

// ==================== 触摸事件处理 ====================

bool BaseScene::onTouchBegan(Touch* touch, Event* event) {
    // 如果训练面板正在显示，不处理触摸
    if (_trainPanel && _trainPanel->isShowing()) {
        return false;
    }
    
    // 如果建筑商店正在显示，不处理触摸
    if (_buildShopLayer && _buildShopLayer->isVisible()) {
        return false;
    }
    
    if (_isPlacingMode) {
        return true;
    }
    
    // 检查是否点击了兵营建筑
    Vec2 touchPos = touch->getLocation();
    Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
    
    auto& buildings = _buildingLayer->getChildren();
    for (auto& child : buildings) {
        auto building = dynamic_cast<ProductionBuilding*>(child);
        if (building) {
            auto boundingBox = building->getBoundingBox();
            if (boundingBox.containsPoint(localPos)) {
                // 只有兵营（SoldierBuilder, ID=3002）才打开训练面板
                if (building->getId() == 3002 || building->getName() == "SoldierBuilder") {
                    CCLOG("[基地场景] 点击兵营，打开训练面板");
                    showTrainPanel();
                    return true;
                } else {
                    CCLOG("[基地场景] 点击建筑: %s (ID: %d)", 
                          building->getName().c_str(), building->getId());
                }
            }
        }
    }
    
    return false;
}

void BaseScene::onTouchMoved(Touch* touch, Event* event) {
    if (_isPlacingMode && _placementPreview) {
        Vec2 touchPos = touch->getLocation();
        Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
        Vec2 gridPos = _gridMap->worldToGrid(localPos);
        
        int gridX = (int)gridPos.x;
        int gridY = (int)gridPos.y;
        
        // 计算建筑中心位置（修复偏移问题）
        Vec2 worldPos = calculateBuildingPosition(gridX, gridY, _previewWidth, _previewHeight);
        _placementPreview->setPosition(worldPos);

        // 检查是否可以放置并更新网格颜色
        bool canPlace = _gridMap->canPlaceBuilding(gridX, gridY, _previewWidth, _previewHeight);
        updatePlacementGrid(gridX, gridY, canPlace);
        
        // 更新预览颜色
        if (canPlace) {
            _placementPreview->setColor(Color3B::WHITE);
        } else {
            _placementPreview->setColor(Color3B::GRAY);
        }
    }
}

void BaseScene::onTouchEnded(Touch* touch, Event* event) {
    if (_isPlacingMode && _placementPreview) {
        Vec2 touchPos = touch->getLocation();
        Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
        Vec2 gridPos = _gridMap->worldToGrid(localPos);
        
        int gridX = (int)gridPos.x;
        int gridY = (int)gridPos.y;

        if (_gridMap->canPlaceBuilding(gridX, gridY, _previewWidth, _previewHeight)) {
            // 创建实际建筑
            Node* newBuilding = nullptr;
            
            switch (_selectedBuildingType) {
                case 1: { // 箭塔
                    DefenceBuildingConfig config;
                    config.id = 2001;
                    config.name = "ArrowTower";
                    config.spriteFrameName = "buildings/ArrowTower.png";
                    config.HP = {500, 750, 1000};
                    config.DP = {0, 0.05f, 0.1f};
                    config.ATK = {30, 45, 60};
                    config.ATK_RANGE = {200, 250, 300};
                    config.ATK_SPEED = {1.0f, 0.9f, 0.8f};
                    config.SKY_ABLE = true;
                    config.GROUND_ABLE = true;
                    config.length = 2;
                    config.width = 2;
                    config.MAXLEVEL = 2;
                    newBuilding = DefenceBuilding::create(&config, 0);
                    break;
                }
                case 2: { // 炮塔
                    DefenceBuildingConfig config;
                    config.id = 2002;
                    config.name = "BoomTower";
                    config.spriteFrameName = "buildings/BoomTower.png";
                    config.HP = {600, 900, 1200};
                    config.DP = {0.05f, 0.1f, 0.15f};
                    config.ATK = {50, 75, 100};
                    config.ATK_RANGE = {180, 220, 260};
                    config.ATK_SPEED = {1.5f, 1.4f, 1.3f};
                    config.SKY_ABLE = false;
                    config.GROUND_ABLE = true;
                    config.length = 2;
                    config.width = 2;
                    config.MAXLEVEL = 2;
                    newBuilding = DefenceBuilding::create(&config, 0);
                    break;
                }
                case 3: { // 树
                    DefenceBuildingConfig config;
                    config.id = 2003;
                    config.name = "Tree";
                    config.spriteFrameName = "buildings/Tree/sprite_0000.png";
                    config.HP = {450, 650, 850};
                    config.DP = {0, 0.05f, 0.1f};
                    config.ATK = {25, 40, 55};
                    config.ATK_RANGE = {220, 270, 320};
                    config.ATK_SPEED = {0.8f, 0.7f, 0.6f};
                    config.SKY_ABLE = false;
                    config.GROUND_ABLE = true;
                    config.length = 2;
                    config.width = 2;
                    config.MAXLEVEL = 2;
                    newBuilding = DefenceBuilding::create(&config, 0);
                    break;
                }
                case 4: { // 雪人仓库
                    StorageBuildingConfig config;
                    config.id = 4001;
                    config.name = "Snowman";
                    config.spriteFrameName = "buildings/snowman.png";
                    config.HP = {800, 1000, 1200};
                    config.DP = {0.1f, 0.15f, 0.2f};
                    config.length = 2;
                    config.width = 2;
                    config.MAXLEVEL = 2;
                    config.ADD_STORAGE_ELIXIR_CAPACITY = {1000, 2000, 4000};
                    config.ADD_STORAGE_GOLD_CAPACITY = {1000, 2000, 4000};
                    newBuilding = StorageBuilding::create(&config, 0);
                    break;
                }
                case 5: { // 兵营
                    ProductionBuildingConfig config;
                    config.id = 3002;
                    config.name = "SoldierBuilder";
                    config.spriteFrameName = "buildings/soldierbuilder.png";
                    config.HP = {600, 750, 900};
                    config.DP = {0, 0, 0};
                    config.length = 3;
                    config.width = 3;
                    config.MAXLEVEL = 2;
                    config.PRODUCE_ELIXIR = {0, 0, 0};
                    config.STORAGE_ELIXIR_CAPACITY = {0, 0, 0};
                    config.PRODUCE_GOLD = {0, 0, 0};
                    config.STORAGE_GOLD_CAPACITY = {0, 0, 0};
                    newBuilding = ProductionBuilding::create(&config, 0);
                    break;
                }
            }
            
            if (newBuilding) {
                _buildingLayer->addChild(newBuilding);
                
                // 使用统一的位置计算方法放置建筑
                Vec2 buildingPos = calculateBuildingPosition(gridX, gridY, _previewWidth, _previewHeight);
                newBuilding->setPosition(buildingPos);
                
                // 标记网格为已占用
                _gridMap->occupyCell(gridX, gridY, _previewWidth, _previewHeight, newBuilding);
                
                // 扣除费用
                int cost = 0;
                switch (_selectedBuildingType) {
                    case 1: cost = 100; break;
                    case 2: cost = 150; break;
                    case 3: cost = 80; break;
                    case 4: cost = 200; break;
                    case 5: cost = 300; break;
                }
                _currentGold -= cost;
            }
            
            confirmPlacement();
        } else {
            cancelPlacement();
        }
    }
}