#include "BaseScene.h"
#include "MainMenuScene.h"
#include "LevelSelectScene.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Buildings/StorageBuilding.h"
#include "UI/TrainPanel.h"
#include "Soldier/UnitManager.h"

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
    _selectedLevel = 0; 

	// ���������ͼ
    // �����һ�£��Ķ�һ��ȽϺ�
    // ע��ο�����
    _gridMap = GridMap::create(40, 40, 32.0f);
    _gridMap->setPosition(origin);
    this->addChild(_gridMap, 0);

    // ������
    _buildingLayer = Node::create();
    _gridMap->addChild(_buildingLayer, 10);

    // �ݵر���
    createGrassBackground();

	// ��ʼ���������ؽ���
    initBaseBuilding();

    // ����UI
    createUI();

    // �����̵�
    createBuildShop();
    _buildShopLayer->setVisible(false);
    
    // ����ѵ�����
    createTrainPanel();

    // ���ر������ã�ȷ��UnitManager�ѳ�ʼ����
    UnitManager::getInstance()->loadConfig("res/units_config.json");

    // ����������
	// �ⲿ���Ǹ�������صĴ��룬�̵���ק�Ž���ɶ�Ķ���Ҫ�õ�
    auto listener = EventListenerTouchOneByOne::create(); // ��㴥��������->һ�δ���һ��������
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
            // ����Ĳݵ�����->�ⲿ�ֿ��Խ����㷨�Ż�������������Ȼ�����ӵĲݵطֲ�
            int grassType = rand() % 6;
            char buffer[64];
            sprintf(buffer, "Grass/grass_000%d.png", grassType); // ��ס���������ʽ
            
            auto grassSprite = Sprite::create(buffer);
            if (grassSprite) {
                Vec2 pos = _gridMap->gridToWorld(x, y);
                grassSprite->setPosition(pos);
				// ���Ųݵ���ͼ����Ӧ�����С
                // ֮������Ų���������ʽ�ȽϷ���
                grassSprite->setScale(32.0f / grassSprite->getContentSize().width);
                _gridMap->addChild(grassSprite, 0);
            }
        }
    }
}

// UI��
// �����⸺���޸�
Node* BaseScene::createTooltip(const std::string& text,const Size& size) 
{ 
    auto panel = Node::create(); 
    panel->setContentSize(size); 
    panel->setAnchorPoint(Vec2(0, 0.5f)); 
    auto bg = LayerColor::create( Color4B(0, 0, 0, 180), size.width, size.height ); bg->setPosition(Vec2::ZERO); 
    panel->addChild(bg); 
    auto label = Label::createWithTTF( text, "fonts/ScienceGothic.ttf", 18 ); 
    label->setWidth(size.width - 10); 
    label->setAlignment(TextHAlignment::LEFT); 
    label->setPosition(Vec2(size.width / 2, size.height / 2)); 
    panel->addChild(label);
    return panel; 
}
void BaseScene::bindTooltip(
    Node* owner,
    Node* targetBtn,
    Node* tooltip,
    float offsetX
)
{
    tooltip->setVisible(false);
    owner->addChild(tooltip, 999);

    auto mouseListener = EventListenerMouse::create();

    mouseListener->onMouseMove = [=](EventMouse* event)
    {
        Vec2 mouseWorldPos(
            event->getCursorX(),
            event->getCursorY()
        );

        if (targetBtn->getBoundingBox().containsPoint(mouseWorldPos))
        {
            float btnRight =
                targetBtn->getPositionX() +
                targetBtn->getContentSize().width *
                targetBtn->getScale() / 2;

            tooltip->setPosition(Vec2(
                btnRight + offsetX,
                targetBtn->getPositionY()
            ));

            tooltip->setVisible(true);
        }
        else
        {
            tooltip->setVisible(false);
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(
        mouseListener,
        owner
    );
}

void BaseScene::createUI() {
    // origin��ָ���ɼ����������½ǵ�ȫ������ϵ�м��λ��(0, 0)
	// visibleSize��ָ���ɼ����򡱵Ŀ���
    // �������ַ�ʽ���ؾ�������
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _uiLayer = Node::create();
    this->addChild(_uiLayer, 100);

	// �����ǹ���UI��ť��λ�á��Ķ��壬֮�������õ�������⼸��������λ��
    // ע�⣺������������λ�ã���������ά��
	float panelX = origin.x + 20; // ���Xλ��
    float buttonSpacing = 30; // ��Ť��ļ�϶
    float startY = visibleSize.height / 2 + 100;

    // Attack button
    auto attackBtn = Button::create("UI/attack.png");
	attackBtn->setScale(4.0f);
    attackBtn->setPosition(Vec2(panelX, startY));
    attackBtn->addClickEventListener([this](Ref* sender) { this->onAttackButton(sender); });
    _uiLayer->addChild(attackBtn);
    auto attackTip = createTooltip("Attack", Size(80, 30));
    bindTooltip(this, attackBtn, attackTip);

    // Build button
    auto buildBtn = Button::create("UI/build.png");
	buildBtn->setScale(4.0f);
    buildBtn->setPosition(Vec2(panelX, startY - buttonSpacing));
    buildBtn->addClickEventListener([this](Ref* sender) { this->onBuildButton(sender); });
    _uiLayer->addChild(buildBtn);
    auto buildTip = createTooltip("Build", Size(60, 30));
    bindTooltip(this, buildBtn, buildTip);

    // Exit button
    auto exitBtn = Button::create("UI/exit.png");
	exitBtn->setScale(4.0f);
    exitBtn->setPosition(Vec2(panelX, startY - buttonSpacing * 2));
    exitBtn->addClickEventListener([this](Ref* sender) { this->onExitButton(sender); });
    _uiLayer->addChild(exitBtn);
    auto exitTip = createTooltip("Exit", Size(60, 30));
    bindTooltip(this, exitBtn, exitTip);
    
    // Resource display
    // �ⲿ�ֻ���Ϊͳһ��ID_card���ӽڵ�
    _goldLabel = Label::createWithSystemFont("Gold: 1000", "ScienceGothic", 10);
    _goldLabel->setPosition(Vec2(panelX, visibleSize.height - 30));
    _goldLabel->setColor(Color3B::YELLOW);
    _uiLayer->addChild(_goldLabel);

    _diamondLabel = Label::createWithSystemFont("Diamond: 100", "ScienceGothic", 10);
    _diamondLabel->setPosition(Vec2(panelX + 60, visibleSize.height - 30));
    _diamondLabel->setColor(Color3B(0, 255, 255)); // Cyan color ɵ��cocos2dû�������ɫ��ֻ���ִ�
    _uiLayer->addChild(_diamondLabel);
}
// �����̵����
void BaseScene::createBuildShop() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _buildShopLayer = Node::create();
    this->addChild(_buildShopLayer, 101);

    // �����̵�
    // ���Ҳ�ø�
    auto bg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    _buildShopLayer->addChild(bg);

	// �̵����->�������ذ�ť�ͱ���ȿؼ�
    // ע�⣬���Ǳ�������������˼·��ʵ��ģ�黯
    // ����Ҫ�Ӹ�����
    auto shopPanel = LayerColor::create(Color4B(50, 50, 50, 255), 400, 500);
    shopPanel->setPosition(Vec2(visibleSize.width / 2 - 200, visibleSize.height / 2 - 250));
    _buildShopLayer->addChild(shopPanel);

    // Title
    auto titleLabel = Label::createWithSystemFont("Building Shop", "ScienceGothic", 28);
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

	// �ⲿ�ִ����˽���ѡ��İ�ť�����͵���¼���
    // ����Ű�����۵��OK
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

// ===================================================
// ����ѵ����崴��
// ===================================================
void BaseScene::createTrainPanel() {
    // ����ѵ����壬����ص�����
    _trainPanel = TrainPanel::create(
        // ѵ����ɻص�
        [this](int unitId) {
            this->onUnitTrainComplete(unitId);
        },
        // �رջص�
        [this]() {
            CCLOG("[���س���] ѵ�����ر�");
        }
    );
    
    if (_trainPanel) {
        this->addChild(_trainPanel, 102);
        CCLOG("[���س���] ѵ����崴���ɹ�");
    }
}

// ===================================================
// ��ʾѵ�����
// ===================================================
void BaseScene::showTrainPanel() {
    if (_trainPanel) {
        _trainPanel->show();
        CCLOG("[���س���] ��ѵ�����");
    }
}

// ===================================================
// ����ѵ����ɻص�
// ===================================================
void BaseScene::onUnitTrainComplete(int unitId) {
    CCLOG("[���س���] ����ѵ�����: ID=%d", unitId);
    // �����������ѵ����ɺ���߼�
    // ���磺����UI��������Ч��
}

// ��ʼ������->���غ�һ����Ӫ
// �����֪��Ϊɶ��û�����ó�����Ҫdebug
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
        // �ѻ��ط�������
        BuildingManager::getInstance()->placeBuilding(base, 18, 18, 4, 4);
    }
}

// ===================================================
// ������ť�ص� - ��ת���ؿ�ѡ�񳡾�
// ===================================================
void BaseScene::onAttackButton(Ref* sender) {
    CCLOG("[���س���] ���������ť����ת���ؿ�ѡ��");
    
    // ��ȡ��ѵ���ı���
    std::map<int, int> trainedUnits;
    if (_trainPanel) {
        trainedUnits = _trainPanel->getTrainedUnits();
    }
    
    // ��ת���ؿ�ѡ�񳡾�
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

// ���Ҫչʾ������ϢԤ��
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

// ===================================================
// ������ʼ�ص�
// ===================================================
bool BaseScene::onTouchBegan(Touch* touch, Event* event) {
    // ���ѵ�����������ʾ����������������
    if (_trainPanel && _trainPanel->isShowing()) {
        return false;
    }
    
    // ��������̵�������ʾ����������������
    if (_buildShopLayer && _buildShopLayer->isVisible()) {
        return false;
    }
    
    if (_isPlacingMode) {
        return true;
    }
    
    // ����Ƿ����˱�Ӫ������Soldier Builder��
    Vec2 touchPos = touch->getLocation();
    Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
    
    // ����������������ӽڵ㣬����Ƿ����˱�Ӫ
    auto& buildings = _buildingLayer->getChildren();
    for (auto& child : buildings) {
        // ����Ƿ���ProductionBuilding����
        auto building = dynamic_cast<ProductionBuilding*>(child);
        if (building) {
            // ��ȡ�����ı߽��
            auto boundingBox = building->getBoundingBox();
            if (boundingBox.containsPoint(localPos)) {
                // ֻ�б�Ӫ��SoldierBuilder, ID=3002���Ŵ�ѵ�����
                if (building->getId() == 3002 || building->getName() == "SoldierBuilder") {
                    CCLOG("[BaseScene] Clicked barracks, opening training panel");
                    showTrainPanel();
                    return true;
                } else {
                    CCLOG("[BaseScene] Clicked building: %s (ID: %d)", 
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