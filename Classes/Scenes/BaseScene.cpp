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
    for (int y = 0; y < 40; ++y) {
        for (int x = 0; x < 40; ++x) {
            // Random grass tile (0-2)
            int grassType = rand() % 3;
            char buffer[32];
            sprintf(buffer, "grass/grass_000%d.png", grassType);
            
            auto grassSprite = Sprite::create(buffer);
            if (grassSprite) {
                Vec2 pos = _gridMap->gridToWorld(x, y);
                grassSprite->setPosition(pos);
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
    float buttonSpacing = 80;
    float startY = visibleSize.height / 2 + 100;

    // Attack button
    auto attackBtn = Button::create("btn_normal.png", "btn_pressed.png.png");
    attackBtn->setTitleText("Attack");
    attackBtn->setTitleFontSize(24);
    attackBtn->setPosition(Vec2(panelX, startY));
    attackBtn->addClickEventListener([this](Ref* sender) { this->onAttackButton(sender); });
    _uiLayer->addChild(attackBtn);

    // Build button
    auto buildBtn = Button::create("btn_normal.png", "btn_pressed.png.png");
    buildBtn->setTitleText("Build");
    buildBtn->setTitleFontSize(24);
    buildBtn->setPosition(Vec2(panelX, startY - buttonSpacing));
    buildBtn->addClickEventListener([this](Ref* sender) { this->onBuildButton(sender); });
    _uiLayer->addChild(buildBtn);

    // Exit button
    auto exitBtn = Button::create("btn_normal.png", "btn_pressed.png.png");
    exitBtn->setTitleText("Exit");
    exitBtn->setTitleFontSize(24);
    exitBtn->setPosition(Vec2(panelX, startY - buttonSpacing * 2));
    exitBtn->addClickEventListener([this](Ref* sender) { this->onExitButton(sender); });
    _uiLayer->addChild(exitBtn);

    // Resource display
    _goldLabel = Label::createWithSystemFont("Gold: 1000", "Arial", 20);
    _goldLabel->setPosition(Vec2(panelX, visibleSize.height - 30));
    _goldLabel->setColor(Color3B::YELLOW);
    _uiLayer->addChild(_goldLabel);

    _diamondLabel = Label::createWithSystemFont("Diamond: 100", "Arial", 20);
    _diamondLabel->setPosition(Vec2(panelX, visibleSize.height - 60));
    _diamondLabel->setColor(Color3B::CYAN);
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
    // Create base building in center (grid position 19, 19 - 2x2 size)
    ProductionBuildingConfig baseConfig;
    baseConfig.id = 100;
    baseConfig.name = "Base";
    baseConfig.spriteFrameName = "buildings/base.png";
    baseConfig.HP = {1000, 1500, 2000};
    baseConfig.DP = {0.5f, 0.6f, 0.7f};
    baseConfig.length = 2;
    baseConfig.width = 2;
    baseConfig.MAXLEVEL = 2;
    baseConfig.PRODUCE_GOLD = {10, 15, 20};
    baseConfig.PRODUCE_ELIXIR = {5, 8, 10};
    baseConfig.STORAGE_GOLD_CAPACITY = {1000, 2000, 3000};
    baseConfig.STORAGE_ELIXIR_CAPACITY = {500, 1000, 1500};

    auto base = ProductionBuilding::create(&baseConfig, 0);
    if (base) {
        _buildingLayer->addChild(base);
        BuildingManager::getInstance()->setGridMap(_gridMap);
        BuildingManager::getInstance()->placeBuilding(base, 19, 19, 2, 2);
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
            _previewWidth = 1;
            _previewHeight = 1;
            break;
        case 2: // Boom Tower
            spritePath = "buildings/BoomTower.png";
            _previewWidth = 1;
            _previewHeight = 1;
            break;
        case 3: // Tree
            spritePath = "buildings/Tree/sprite_0000.png";
            _previewWidth = 1;
            _previewHeight = 1;
            break;
        case 4: // Snowman
            spritePath = "buildings/snowman.png";
            _previewWidth = 1;
            _previewHeight = 1;
            break;
        case 5: // Soldier Builder
            spritePath = "buildings/soldierbuilder.png";
            _previewWidth = 2;
            _previewHeight = 2;
            break;
    }

    _placementPreview = Sprite::create(spritePath);
    if (_placementPreview) {
        _placementPreview->setOpacity(180);
        _gridMap->addChild(_placementPreview, 50);
    }
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
}

void BaseScene::cancelPlacement() {
    _isPlacingMode = false;
    if (_placementPreview) {
        _placementPreview->removeFromParent();
        _placementPreview = nullptr;
    }
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

        if (_gridMap->canPlaceBuilding((int)gridPos.x, (int)gridPos.y, _previewWidth, _previewHeight)) {
            // TODO: Create actual building and place it
            // For now, just end placement mode
            confirmPlacement();
        } else {
            cancelPlacement();
        }
    }
}
