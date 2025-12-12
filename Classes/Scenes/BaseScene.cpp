#include "BaseScene.h"
#include "MainMenuScene.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Buildings/StorageBuilding.h"

USING_NS_CC;

Scene* BaseScene::createScene() {
    return BaseScene::create();
}

bool BaseScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _isPlacingMode = false;
    _draggingBuilding = nullptr;
    _placementPreview = nullptr;
    _currentGold = 1000;
    _currentDiamond = 100;
    _selectedBuildingType = 0;

    // Create grid map (40x40 grid, 32 pixels per cell)
    _gridMap = GridMap::create(40, 40, 32.0f);
    _gridMap->setPosition(origin);
    this->addChild(_gridMap, 0);

    // Create building layer
    _buildingLayer = Node::create();
    _gridMap->addChild(_buildingLayer, 10);

    // Create grass background
    createGrassBackground();

    // Initialize base building in center
    initBaseBuilding();

    // Create UI
    createUI();

    // Create build shop (hidden initially)
    createBuildShop();
    _buildShopLayer->setVisible(false);

    // Setup touch listener
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BaseScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BaseScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BaseScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void BaseScene::createGrassBackground() {
    // Create grass tile background
    for (int y = 0; y < 160; ++y) {
        for (int x = 0; x < 160; ++x) {
            // Random grass tile (0-2)
            int grassType = rand() % 6;
            char buffer[64];
            sprintf(buffer, "Grass/grass_000%d.png", grassType);
            
            auto grassSprite = Sprite::create(buffer);
            if (grassSprite) {
                Vec2 pos = _gridMap->gridToWorld(x, y);
                grassSprite->setPosition(pos);
                // Scale grass to fit cell size
                grassSprite->setScale(32.0f / grassSprite->getContentSize().width);
                _gridMap->addChild(grassSprite, 0);
            }
        }
    }
}

void BaseScene::createUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _uiLayer = Node::create();
    this->addChild(_uiLayer, 100);

    // Create UI panel on the left side
    float panelX = origin.x + 80;
    float buttonSpacing = 30;
    float startY = visibleSize.height / 2 + 100;

    // Attack button
    auto attackBtn = Button::create("UI/attack.png");
    // attackBtn->setTitleText("Attack");
    // attackBtn->setTitleFontSize(24);
	attackBtn->setScale(4.0f);
    attackBtn->setPosition(Vec2(panelX, startY));
    attackBtn->addClickEventListener([this](Ref* sender) { this->onAttackButton(sender); });
    _uiLayer->addChild(attackBtn);

    // Build button
    auto buildBtn = Button::create("UI/build.png");
    // buildBtn->setTitleText("Build");
    // buildBtn->setTitleFontSize(24);
	buildBtn->setScale(4.0f);
    buildBtn->setPosition(Vec2(panelX, startY - buttonSpacing));
    buildBtn->addClickEventListener([this](Ref* sender) { this->onBuildButton(sender); });
    _uiLayer->addChild(buildBtn);

    // Exit button
    auto exitBtn = Button::create("UI/exit.png");
    // exitBtn->setTitleText("Exit");
    // exitBtn->setTitleFontSize(24);
	exitBtn->setScale(4.0f);
    exitBtn->setPosition(Vec2(panelX, startY - buttonSpacing * 2));
    exitBtn->addClickEventListener([this](Ref* sender) { this->onExitButton(sender); });
    _uiLayer->addChild(exitBtn);

    // Resource display
    _goldLabel = Label::createWithSystemFont("Gold: 1000", "ScienceGothic", 10);
    _goldLabel->setPosition(Vec2(panelX, visibleSize.height - 30));
    _goldLabel->setColor(Color3B::YELLOW);
    _uiLayer->addChild(_goldLabel);

    _diamondLabel = Label::createWithSystemFont("Diamond: 100", "ScienceGothic", 10);
    _diamondLabel->setPosition(Vec2(panelX + 60, visibleSize.height - 30));
    _diamondLabel->setColor(Color3B(0, 255, 255)); // Cyan color 傻逼cocos2d没有这个颜色，只能手打
    _uiLayer->addChild(_diamondLabel);
}

void BaseScene::createBuildShop() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _buildShopLayer = Node::create();
    this->addChild(_buildShopLayer, 101);

    // Create semi-transparent background
    auto bg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    _buildShopLayer->addChild(bg);

    // Create shop panel
    auto shopPanel = LayerColor::create(Color4B(50, 50, 50, 255), 400, 500);
    shopPanel->setPosition(Vec2(visibleSize.width / 2 - 200, visibleSize.height / 2 - 250));
    _buildShopLayer->addChild(shopPanel);

    // Title
    auto titleLabel = Label::createWithSystemFont("Building Shop", "Arial", 28);
    titleLabel->setPosition(Vec2(200, 460));
    shopPanel->addChild(titleLabel);

    float yPos = 400;
    float spacing = 60;

    // Building options
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

    for (int i = 0; i < 5; ++i) {
        auto btn = Button::create("btn_normal.png", "btn_pressed.png.png");
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

    // Close button
    auto closeBtn = Button::create("btn_normal.png", "btn_pressed.png.png");
    closeBtn->setTitleText("Close");
    closeBtn->setTitleFontSize(24);
    closeBtn->setPosition(Vec2(200, 50));
    closeBtn->addClickEventListener([this](Ref* sender) {
        _buildShopLayer->setVisible(false);
    });
    shopPanel->addChild(closeBtn);
}

void BaseScene::initBaseBuilding() {
    // Initialize base building config as member variable
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
        // Place base at center: grid 18,18 for a 4x4 building (so it centers around 20,20)
        BuildingManager::getInstance()->placeBuilding(base, 18, 18, 4, 4);
    }
}

void BaseScene::onAttackButton(Ref* sender) {
    CCLOG("Attack button clicked - Not implemented yet");
}

void BaseScene::onBuildButton(Ref* sender) {
    _buildShopLayer->setVisible(true);
}

void BaseScene::onExitButton(Ref* sender) {
    auto scene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void BaseScene::onBuildingSelected(int buildingType) {
    _selectedBuildingType = buildingType;
    _buildShopLayer->setVisible(false);
    _isPlacingMode = true;

    // Create preview sprite
    if (_placementPreview) {
        _placementPreview->removeFromParent();
        _placementPreview = nullptr;
    }

    std::string spritePath;
    _previewWidth = 1;
    _previewHeight = 1;

    switch (buildingType) {
        case 1: // Arrow Tower
            spritePath = "buildings/ArrowTower.png";
            _previewWidth = 2;
            _previewHeight = 2;
            break;
        case 2: // Boom Tower
            spritePath = "buildings/BoomTower.png";
            _previewWidth = 2;
            _previewHeight = 2;
            break;
        case 3: // Tree
            spritePath = "buildings/Tree/sprite_0000.png";
            _previewWidth = 2;
            _previewHeight = 2;
            break;
        case 4: // Snowman
            spritePath = "buildings/snowman.png";
            _previewWidth = 2;
            _previewHeight = 2;
            break;
        case 5: // Soldier Builder
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
    
    // Show grid lines for easier placement
    _gridMap->showGrid(true);
}

void BaseScene::updatePlacementPreview() {
    // Update preview position and color based on validity
}

void BaseScene::confirmPlacement() {
    // Place the building
    _isPlacingMode = false;
    if (_placementPreview) {
        _placementPreview->removeFromParent();
        _placementPreview = nullptr;
    }
    // Hide grid lines
    _gridMap->showGrid(false);
}

void BaseScene::cancelPlacement() {
    _isPlacingMode = false;
    if (_placementPreview) {
        _placementPreview->removeFromParent();
        _placementPreview = nullptr;
    }
    // Hide grid lines
    _gridMap->showGrid(false);
}

bool BaseScene::onTouchBegan(Touch* touch, Event* event) {
    if (_isPlacingMode) {
        return true;
    }
    return false;
}

void BaseScene::onTouchMoved(Touch* touch, Event* event) {
    if (_isPlacingMode && _placementPreview) {
        Vec2 touchPos = touch->getLocation();
        Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
        Vec2 gridPos = _gridMap->worldToGrid(localPos);
        
        // Snap to grid
        Vec2 worldPos = _gridMap->gridToWorld((int)gridPos.x, (int)gridPos.y);
        _placementPreview->setPosition(worldPos);

        // Check if can place
        if (_gridMap->canPlaceBuilding((int)gridPos.x, (int)gridPos.y, _previewWidth, _previewHeight)) {
            _placementPreview->setColor(Color3B::GREEN);
        } else {
            _placementPreview->setColor(Color3B::RED);
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
            // Create the actual building based on type
            Node* newBuilding = nullptr;
            
            switch (_selectedBuildingType) {
                case 1: { // Arrow Tower
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
                    auto building = DefenceBuilding::create(&config, 0);
                    newBuilding = building;
                    break;
                }
                case 2: { // Boom Tower
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
                    auto building = DefenceBuilding::create(&config, 0);
                    newBuilding = building;
                    break;
                }
                case 3: { // Tree
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
                    auto building = DefenceBuilding::create(&config, 0);
                    newBuilding = building;
                    break;
                }
                case 4: { // Snowman
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
                    auto building = StorageBuilding::create(&config, 0);
                    newBuilding = building;
                    break;
                }
                case 5: { // Soldier Builder
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
                    auto building = ProductionBuilding::create(&config, 0);
                    newBuilding = building;
                    break;
                }
            }
            
            if (newBuilding) {
                _buildingLayer->addChild(newBuilding);
                BuildingManager::getInstance()->placeBuilding(newBuilding, gridX, gridY, _previewWidth, _previewHeight);
                
                // Deduct cost
                int cost = 0;
                switch (_selectedBuildingType) {
                    case 1: cost = 100; break;
                    case 2: cost = 150; break;
                    case 3: cost = 80; break;
                    case 4: cost = 200; break;
                    case 5: cost = 300; break;
                }
                _currentGold -= cost;
                
                // Update UI
                char buffer[64];
                sprintf(buffer, "Gold: %d", _currentGold);
                _goldLabel->setString(buffer);
            }
            
            confirmPlacement();
        } else {
            cancelPlacement();
        }
    }
}
////