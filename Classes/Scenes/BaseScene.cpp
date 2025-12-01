// BaseScene.cpp
// 基地场景实现文件
// 功能: 实现基地建设和管理的主要逻辑

#include "BaseScene.h"
#include "MainMenuScene.h"
#include "BattleScene.h"
#include "../Core/ResourceManager.h"
#include "../Buildings/Building.h"
#include "../Buildings/DefenseBuilding.h"
#include "../Buildings/ProductionBuilding.h"
#include "../Buildings/StorageBuilding.h"
#include "../Buildings/Barracks.h"

// ==================== 场景创建 ====================

Scene* BaseScene::createScene() {
    return BaseScene::create();
}

bool BaseScene::init() {
    if (!Scene::init()) {
        return false;
    }
    
    // 初始化变量
    _selectedBuilding = nullptr;
    _movingBuilding = nullptr;
    _isMovingBuilding = false;
    _isBuildMode = false;
    _buildPreview = nullptr;
    _cameraOffset = Vec2::ZERO;
    _cameraZoom = 1.0f;
    _isDragging = false;
    
    // 创建地图层
    createMapLayer();
    
    // 创建UI层
    createUILayer();
    
    // 初始化默认建筑
    initDefaultBuildings();
    
    // 注册资源变化回调
    ResourceManager::getInstance()->registerCallback(
        std::bind(&BaseScene::onResourceChanged, this, 
                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    
    // 添加触摸监听
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(false);
    touchListener->onTouchBegan = CC_CALLBACK_2(BaseScene::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(BaseScene::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(BaseScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    // 添加鼠标滚轮监听
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseScroll = CC_CALLBACK_1(BaseScene::onMouseScroll, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
    
    // 启用帧更新
    scheduleUpdate();
    
    return true;
}

// ==================== 地图创建 ====================

void BaseScene::createMapLayer() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _mapLayer = Node::create();
    this->addChild(_mapLayer, Z_GROUND);
    
    // 创建地面背景
    auto ground = DrawNode::create();
    float mapWidth = MAP_WIDTH * GRID_SIZE;
    float mapHeight = MAP_HEIGHT * GRID_SIZE;
    
    // 绘制草地背景
    ground->drawSolidRect(Vec2(0, 0), Vec2(mapWidth, mapHeight), 
                          Color4F(0.3f, 0.6f, 0.3f, 1.0f));
    
    // 添加一些草地纹理变化
    for (int i = 0; i < 50; i++) {
        float x = rand() % (int)mapWidth;
        float y = rand() % (int)mapHeight;
        float size = 10 + rand() % 20;
        Color4F color = Color4F(0.25f + (rand() % 20) / 100.0f, 
                               0.55f + (rand() % 20) / 100.0f, 
                               0.25f + (rand() % 20) / 100.0f, 0.3f);
        ground->drawSolidCircle(Vec2(x, y), size, 0, 12, color);
    }
    
    _mapLayer->addChild(ground, 0);
    
    // 创建网格层
    createGrid();
    
    // 创建建筑层
    _buildingLayer = Node::create();
    _mapLayer->addChild(_buildingLayer, Z_BUILDING);
    
    // 居中地图
    _cameraOffset = Vec2(origin.x + visibleSize.width / 2 - mapWidth / 2,
                        origin.y + visibleSize.height / 2 - mapHeight / 2);
    _mapLayer->setPosition(_cameraOffset);
}

void BaseScene::createGrid() {
    _gridLayer = Node::create();
    
    auto gridDraw = DrawNode::create();
    float mapWidth = MAP_WIDTH * GRID_SIZE;
    float mapHeight = MAP_HEIGHT * GRID_SIZE;
    
    // 绘制网格线
    Color4F gridColor = Color4F(0.2f, 0.4f, 0.2f, 0.3f);
    
    // 垂直线
    for (int x = 0; x <= MAP_WIDTH; x++) {
        gridDraw->drawLine(Vec2(x * GRID_SIZE, 0), 
                          Vec2(x * GRID_SIZE, mapHeight), gridColor);
    }
    
    // 水平线
    for (int y = 0; y <= MAP_HEIGHT; y++) {
        gridDraw->drawLine(Vec2(0, y * GRID_SIZE), 
                          Vec2(mapWidth, y * GRID_SIZE), gridColor);
    }
    
    _gridLayer->addChild(gridDraw);
    _gridLayer->setVisible(false);  // 默认隐藏网格
    _mapLayer->addChild(_gridLayer, 1);
}

// ==================== UI创建 ====================

void BaseScene::createUILayer() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _uiLayer = Node::create();
    this->addChild(_uiLayer, Z_UI);
    
    // 创建资源显示
    createResourceDisplay();
    
    // 创建底部工具栏
    createToolbar();
    
    // 创建建筑菜单(默认隐藏)
    createBuildMenu();
}

void BaseScene::createResourceDisplay() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _resourcePanel = Node::create();
    
    // 统一的布局常量
    const float panelWidth = 480;
    const float panelHeight = 55;
    const float iconSize = 12;
    const float iconSpacing = 115;  // 每个资源项的间距
    const float startX = 25;
    const float centerY = panelHeight / 2;
    const float labelOffsetX = 25;  // 标签相对图标的偏移
    
    // 背景 - 圆角矩形效果
    auto bgDraw = DrawNode::create();
    bgDraw->drawSolidRect(Vec2(0, 0), Vec2(panelWidth, panelHeight), Color4F(0.08f, 0.08f, 0.12f, 0.85f));
    bgDraw->drawRect(Vec2(0, 0), Vec2(panelWidth, panelHeight), Color4F(0.4f, 0.4f, 0.5f, 1.0f));
    // 添加顶部高亮线
    bgDraw->drawLine(Vec2(2, panelHeight - 1), Vec2(panelWidth - 2, panelHeight - 1), 
                     Color4F(0.5f, 0.5f, 0.6f, 0.5f));
    _resourcePanel->addChild(bgDraw);
    
    // 金币图标和数量 - 位置1
    auto goldIcon = DrawNode::create();
    goldIcon->drawSolidCircle(Vec2(0, 0), iconSize, 0, 16, Color4F(1.0f, 0.84f, 0.0f, 1.0f));
    goldIcon->drawCircle(Vec2(0, 0), iconSize, 0, 16, false, Color4F(0.8f, 0.6f, 0.0f, 1.0f));
    goldIcon->setPosition(Vec2(startX, centerY));
    _resourcePanel->addChild(goldIcon);
    
    _goldLabel = Label::createWithSystemFont("500", "Arial", 16);
    _goldLabel->setPosition(Vec2(startX + labelOffsetX, centerY));
    _goldLabel->setAnchorPoint(Vec2(0, 0.5f));
    _goldLabel->setColor(Color3B(255, 215, 0));
    _resourcePanel->addChild(_goldLabel);
    
    // 圣水图标和数量 - 位置2
    float pos2X = startX + iconSpacing;
    auto elixirIcon = DrawNode::create();
    elixirIcon->drawSolidCircle(Vec2(0, 0), iconSize, 0, 16, Color4F(0.6f, 0.2f, 0.8f, 1.0f));
    elixirIcon->drawCircle(Vec2(0, 0), iconSize, 0, 16, false, Color4F(0.4f, 0.1f, 0.6f, 1.0f));
    elixirIcon->setPosition(Vec2(pos2X, centerY));
    _resourcePanel->addChild(elixirIcon);
    
    _elixirLabel = Label::createWithSystemFont("500", "Arial", 16);
    _elixirLabel->setPosition(Vec2(pos2X + labelOffsetX, centerY));
    _elixirLabel->setAnchorPoint(Vec2(0, 0.5f));
    _elixirLabel->setColor(Color3B(180, 80, 220));
    _resourcePanel->addChild(_elixirLabel);
    
    // 工人图标和数量 - 位置3
    float pos3X = startX + iconSpacing * 2;
    auto workerIcon = DrawNode::create();
    // 绘制工人图标（小人形状）
    workerIcon->drawSolidCircle(Vec2(0, 4), 6, 0, 12, Color4F(0.7f, 0.5f, 0.3f, 1.0f));  // 头
    workerIcon->drawSolidRect(Vec2(-5, -10), Vec2(5, 2), Color4F(0.5f, 0.3f, 0.1f, 1.0f));  // 身体
    workerIcon->setPosition(Vec2(pos3X, centerY));
    _resourcePanel->addChild(workerIcon);
    
    _workerLabel = Label::createWithSystemFont("2/2", "Arial", 16);
    _workerLabel->setPosition(Vec2(pos3X + labelOffsetX, centerY));
    _workerLabel->setAnchorPoint(Vec2(0, 0.5f));
    _workerLabel->setColor(Color3B(200, 150, 100));
    _resourcePanel->addChild(_workerLabel);
    
    // 人口图标和数量 - 位置4
    float pos4X = startX + iconSpacing * 3;
    auto popIcon = DrawNode::create();
    // 绘制人口图标（小组人形状）
    popIcon->drawSolidCircle(Vec2(-4, 2), 5, 0, 10, Color4F(0.4f, 0.6f, 0.8f, 1.0f));
    popIcon->drawSolidCircle(Vec2(4, 2), 5, 0, 10, Color4F(0.4f, 0.6f, 0.8f, 1.0f));
    popIcon->drawSolidCircle(Vec2(0, -4), 4, 0, 10, Color4F(0.4f, 0.6f, 0.8f, 1.0f));
    popIcon->setPosition(Vec2(pos4X, centerY));
    _resourcePanel->addChild(popIcon);
    
    _populationLabel = Label::createWithSystemFont("0/20", "Arial", 16);
    _populationLabel->setPosition(Vec2(pos4X + labelOffsetX, centerY));
    _populationLabel->setAnchorPoint(Vec2(0, 0.5f));
    _populationLabel->setColor(Color3B(150, 200, 255));
    _resourcePanel->addChild(_populationLabel);
    
    // 设置面板位置 - 左上角，保持边距一致
    const float margin = 10;
    _resourcePanel->setPosition(Vec2(origin.x + margin, origin.y + visibleSize.height - panelHeight - margin));
    _uiLayer->addChild(_resourcePanel);
    
    updateResourceDisplay();
}

void BaseScene::createToolbar() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _toolbar = Node::create();
    
    // 工具栏常量
    const float toolbarHeight = 65;
    const float buttonWidth = 105;
    const float buttonHeight = 45;
    const float buttonSpacing = 125;
    const float startX = 75;
    const float buttonCenterY = toolbarHeight / 2;
    
    // 工具栏背景
    auto bgDraw = DrawNode::create();
    bgDraw->drawSolidRect(Vec2(0, 0), Vec2(visibleSize.width, toolbarHeight), 
                          Color4F(0.08f, 0.08f, 0.1f, 0.95f));
    bgDraw->drawLine(Vec2(0, toolbarHeight), Vec2(visibleSize.width, toolbarHeight), 
                     Color4F(0.4f, 0.4f, 0.5f, 1.0f));
    // 添加底部阴影效果
    bgDraw->drawLine(Vec2(0, toolbarHeight - 1), Vec2(visibleSize.width, toolbarHeight - 1),
                     Color4F(0.3f, 0.3f, 0.4f, 0.5f));
    _toolbar->addChild(bgDraw);
    
    Vector<MenuItem*> menuItems;
    
    // 辅助函数：创建工具栏按钮
    auto createToolButton = [=](const std::string& text, const Color4F& bgColor,
                                const Color4F& borderColor, const ccMenuCallback& callback) -> MenuItem* {
        auto label = Label::createWithSystemFont(text, "Arial", 18);
        label->setColor(Color3B::WHITE);
        auto button = MenuItemLabel::create(label, callback);
        
        auto bg = DrawNode::create();
        bg->drawSolidRect(Vec2(-buttonWidth/2, -buttonHeight/2), 
                          Vec2(buttonWidth/2, buttonHeight/2), bgColor);
        bg->drawRect(Vec2(-buttonWidth/2, -buttonHeight/2), 
                     Vec2(buttonWidth/2, buttonHeight/2), borderColor);
        // 顶部高亮
        bg->drawLine(Vec2(-buttonWidth/2 + 2, buttonHeight/2 - 2),
                     Vec2(buttonWidth/2 - 2, buttonHeight/2 - 2),
                     Color4F(1.0f, 1.0f, 1.0f, 0.2f));
        button->addChild(bg, -1);
        
        return button;
    };
    
    // 建造按钮
    auto buildButton = createToolButton("Build", 
                                        Color4F(0.2f, 0.5f, 0.3f, 1.0f),
                                        Color4F(0.3f, 0.7f, 0.4f, 1.0f),
                                        CC_CALLBACK_1(BaseScene::onBuildMenuOpen, this));
    buildButton->setPosition(Vec2(startX, buttonCenterY));
    menuItems.pushBack(buildButton);
    
    // 攻击按钮
    auto attackButton = createToolButton("Attack",
                                         Color4F(0.6f, 0.2f, 0.2f, 1.0f),
                                         Color4F(0.8f, 0.3f, 0.3f, 1.0f),
                                         CC_CALLBACK_1(BaseScene::onAttack, this));
    attackButton->setPosition(Vec2(startX + buttonSpacing, buttonCenterY));
    menuItems.pushBack(attackButton);
    
    // 收集按钮
    auto collectButton = createToolButton("Collect",
                                          Color4F(0.5f, 0.5f, 0.2f, 1.0f),
                                          Color4F(0.7f, 0.7f, 0.3f, 1.0f),
                                          CC_CALLBACK_1(BaseScene::onCollectAll, this));
    collectButton->setPosition(Vec2(startX + buttonSpacing * 2, buttonCenterY));
    menuItems.pushBack(collectButton);
    
    // 帮助按钮
    auto helpButton = createToolButton("Help",
                                       Color4F(0.3f, 0.4f, 0.5f, 1.0f),
                                       Color4F(0.4f, 0.6f, 0.7f, 1.0f),
                                       CC_CALLBACK_1(BaseScene::onHelp, this));
    helpButton->setPosition(Vec2(startX + buttonSpacing * 3, buttonCenterY));
    menuItems.pushBack(helpButton);
    
    // 返回菜单按钮 - 右侧对齐
    auto backButton = createToolButton("Menu",
                                       Color4F(0.3f, 0.3f, 0.4f, 1.0f),
                                       Color4F(0.5f, 0.5f, 0.6f, 1.0f),
                                       CC_CALLBACK_1(BaseScene::onBackToMenu, this));
    backButton->setPosition(Vec2(visibleSize.width - startX, buttonCenterY));
    menuItems.pushBack(backButton);
    
    auto menu = Menu::createWithArray(menuItems);
    menu->setPosition(Vec2::ZERO);
    _toolbar->addChild(menu);
    
    _toolbar->setPosition(origin);
    _uiLayer->addChild(_toolbar);
}

void BaseScene::createBuildMenu() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _buildMenu = Node::create();
    _buildMenu->setVisible(false);
    
    // 背景遮罩
    auto mask = DrawNode::create();
    mask->drawSolidRect(Vec2(0, 0), Vec2(visibleSize.width, visibleSize.height), 
                        Color4F(0, 0, 0, 0.5f));
    _buildMenu->addChild(mask);
    
    // 菜单面板
    float panelWidth = 600;
    float panelHeight = 400;
    float panelX = (visibleSize.width - panelWidth) / 2;
    float panelY = (visibleSize.height - panelHeight) / 2;
    
    auto panel = DrawNode::create();
    panel->drawSolidRect(Vec2(panelX, panelY), Vec2(panelX + panelWidth, panelY + panelHeight),
                         Color4F(0.15f, 0.15f, 0.2f, 0.95f));
    panel->drawRect(Vec2(panelX, panelY), Vec2(panelX + panelWidth, panelY + panelHeight),
                    Color4F(0.5f, 0.5f, 0.6f, 1.0f));
    _buildMenu->addChild(panel);
    
    // 标题
    auto titleLabel = Label::createWithSystemFont("Build Menu", "Arial", 28);
    titleLabel->setPosition(Vec2(visibleSize.width / 2, panelY + panelHeight - 30));
    titleLabel->setColor(Color3B(255, 215, 0));
    _buildMenu->addChild(titleLabel);
    
    // 建筑选项
    Vector<MenuItem*> buildItems;
    
    struct BuildOption {
        BuildingType type;
        std::string name;
        Color4F color;
    };
    
    std::vector<BuildOption> options = {
        {BuildingType::CANNON, "Cannon", Color4F(0.6f, 0.2f, 0.2f, 1.0f)},
        {BuildingType::ARCHER_TOWER, "Archer Tower", Color4F(0.2f, 0.6f, 0.2f, 1.0f)},
        {BuildingType::GOLD_MINE, "Gold Mine", Color4F(1.0f, 0.84f, 0.0f, 1.0f)},
        {BuildingType::ELIXIR_COLLECTOR, "Elixir Collector", Color4F(0.6f, 0.2f, 0.8f, 1.0f)},
        {BuildingType::GOLD_STORAGE, "Gold Storage", Color4F(0.8f, 0.6f, 0.0f, 1.0f)},
        {BuildingType::ELIXIR_STORAGE, "Elixir Storage", Color4F(0.5f, 0.2f, 0.6f, 1.0f)},
        {BuildingType::BARRACKS, "Barracks", Color4F(0.4f, 0.4f, 0.6f, 1.0f)},
        {BuildingType::WALL, "Wall", Color4F(0.5f, 0.5f, 0.5f, 1.0f)}
    };
    
    int cols = 4;
    float itemWidth = 120;
    float itemHeight = 100;
    float startX = panelX + 50;
    float startY = panelY + panelHeight - 100;
    
    for (size_t i = 0; i < options.size(); i++) {
        int col = i % cols;
        int row = i / cols;
        
        auto& opt = options[i];
        
        auto itemNode = Node::create();
        
        // 建筑图标
        auto icon = DrawNode::create();
        icon->drawSolidRect(Vec2(-30, -30), Vec2(30, 30), opt.color);
        icon->drawRect(Vec2(-30, -30), Vec2(30, 30), Color4F::WHITE);
        icon->setPosition(Vec2(0, 20));
        itemNode->addChild(icon);
        
        // 名称
        auto nameLabel = Label::createWithSystemFont(opt.name, "Arial", 14);
        nameLabel->setPosition(Vec2(0, -30));
        nameLabel->setColor(Color3B::WHITE);
        itemNode->addChild(nameLabel);
        
        // 创建按钮
        auto button = MenuItemLabel::create(Label::createWithSystemFont("", "Arial", 1),
            [this, opt](Ref* sender) {
                this->onSelectBuildingType(opt.type);
            });
        button->setContentSize(Size(itemWidth, itemHeight));
        button->addChild(itemNode);
        
        float x = startX + col * (itemWidth + 20) + itemWidth / 2;
        float y = startY - row * (itemHeight + 20);
        button->setPosition(Vec2(x, y));
        
        buildItems.pushBack(button);
    }
    
    // 关闭按钮
    auto closeLabel = Label::createWithSystemFont("X", "Arial", 24);
    closeLabel->setColor(Color3B::WHITE);
    auto closeButton = MenuItemLabel::create(closeLabel, CC_CALLBACK_1(BaseScene::onBuildMenuClose, this));
    closeButton->setPosition(Vec2(panelX + panelWidth - 25, panelY + panelHeight - 25));
    buildItems.pushBack(closeButton);
    
    auto menu = Menu::createWithArray(buildItems);
    menu->setPosition(Vec2::ZERO);
    _buildMenu->addChild(menu);
    
    _buildMenu->setPosition(origin);
    _uiLayer->addChild(_buildMenu, 10);
}

// ==================== 初始化默认建筑 ====================

void BaseScene::initDefaultBuildings() {
    // 添加大本营
    addBuilding(BuildingType::TOWN_HALL, MAP_WIDTH/2 - 2, MAP_HEIGHT/2 - 2);
    
    // 添加初始金矿
    addBuilding(BuildingType::GOLD_MINE, MAP_WIDTH/2 - 6, MAP_HEIGHT/2);
    
    // 添加初始圣水收集器
    addBuilding(BuildingType::ELIXIR_COLLECTOR, MAP_WIDTH/2 + 4, MAP_HEIGHT/2);
    
    // 添加初始兵营
    addBuilding(BuildingType::BARRACKS, MAP_WIDTH/2 - 6, MAP_HEIGHT/2 - 5);
}

// ==================== 建筑操作 ====================

bool BaseScene::addBuilding(BuildingType type, int gridX, int gridY) {
    // 创建建筑
    Building* building = nullptr;
    
    switch (type) {
        case BuildingType::CANNON:
        case BuildingType::ARCHER_TOWER:
        case BuildingType::MORTAR:
            building = DefenseBuilding::create(type, Faction::PLAYER);
            break;
        case BuildingType::GOLD_MINE:
        case BuildingType::ELIXIR_COLLECTOR:
            building = ProductionBuilding::create(type, Faction::PLAYER);
            break;
        case BuildingType::GOLD_STORAGE:
        case BuildingType::ELIXIR_STORAGE:
            building = StorageBuilding::create(type, Faction::PLAYER);
            break;
        case BuildingType::BARRACKS:
            building = Barracks::create(Faction::PLAYER);
            break;
        default:
            building = Building::create(type, Faction::PLAYER);
            break;
    }
    
    if (!building) return false;
    
    // 检查格子是否可用
    int width = building->getGridWidth();
    int height = building->getGridHeight();
    
    if (!isGridAvailable(gridX, gridY, width, height)) {
        building->release();
        return false;
    }
    
    // 设置位置
    building->setGridPosition(gridX, gridY);
    
    // 设置点击回调
    building->setClickCallback([this](Building* b) {
        this->selectBuilding(b);
    });
    
    // 添加到建筑层
    _buildingLayer->addChild(building);
    _buildings.push_back(building);
    
    // 标记格子为占用
    for (int x = gridX; x < gridX + width; x++) {
        for (int y = gridY; y < gridY + height; y++) {
            int key = y * MAP_WIDTH + x;
            _gridOccupied[key] = true;
        }
    }
    
    return true;
}

void BaseScene::removeBuilding(Building* building) {
    if (!building) return;
    
    // 释放格子
    Vec2 gridPos = building->getGridPosition();
    int width = building->getGridWidth();
    int height = building->getGridHeight();
    
    for (int x = (int)gridPos.x; x < (int)gridPos.x + width; x++) {
        for (int y = (int)gridPos.y; y < (int)gridPos.y + height; y++) {
            int key = y * MAP_WIDTH + x;
            _gridOccupied.erase(key);
        }
    }
    
    // 从列表移除
    auto it = std::find(_buildings.begin(), _buildings.end(), building);
    if (it != _buildings.end()) {
        _buildings.erase(it);
    }
    
    // 从场景移除
    building->removeFromParent();
}

void BaseScene::selectBuilding(Building* building) {
    // 取消之前的选中
    if (_selectedBuilding) {
        _selectedBuilding->setSelected(false);
    }
    
    _selectedBuilding = building;
    
    if (_selectedBuilding) {
        _selectedBuilding->setSelected(true);
        showBuildingInfoPanel(building);
    }
}

void BaseScene::deselectBuilding() {
    if (_selectedBuilding) {
        _selectedBuilding->setSelected(false);
        _selectedBuilding = nullptr;
    }
    hideBuildingInfoPanel();
}

void BaseScene::showBuildingInfoPanel(Building* building) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    hideBuildingInfoPanel();
    
    _buildingInfoPanel = Node::create();
    
    float panelWidth = 250;
    float panelHeight = 200;
    
    // 背景
    auto bg = DrawNode::create();
    bg->drawSolidRect(Vec2(0, 0), Vec2(panelWidth, panelHeight), Color4F(0.1f, 0.1f, 0.15f, 0.9f));
    bg->drawRect(Vec2(0, 0), Vec2(panelWidth, panelHeight), Color4F(0.5f, 0.5f, 0.6f, 1.0f));
    _buildingInfoPanel->addChild(bg);
    
    // 建筑名称
    auto nameLabel = Label::createWithSystemFont(building->getName(), "Arial", 20);
    nameLabel->setPosition(Vec2(panelWidth/2, panelHeight - 25));
    nameLabel->setColor(Color3B(255, 215, 0));
    _buildingInfoPanel->addChild(nameLabel);
    
    // 等级
    char levelStr[32];
    snprintf(levelStr, sizeof(levelStr), "Level: %d/%d", building->getLevel(), building->getMaxLevel());
    auto levelLabel = Label::createWithSystemFont(levelStr, "Arial", 16);
    levelLabel->setPosition(Vec2(panelWidth/2, panelHeight - 50));
    levelLabel->setColor(Color3B::WHITE);
    _buildingInfoPanel->addChild(levelLabel);
    
    // 血量
    char hpStr[32];
    snprintf(hpStr, sizeof(hpStr), "HP: %d/%d", building->getCurrentHP(), building->getMaxHP());
    auto hpLabel = Label::createWithSystemFont(hpStr, "Arial", 16);
    hpLabel->setPosition(Vec2(panelWidth/2, panelHeight - 75));
    hpLabel->setColor(Color3B(100, 255, 100));
    _buildingInfoPanel->addChild(hpLabel);
    
    // 按钮
    Vector<MenuItem*> buttons;
    
    // 升级按钮
    auto upgradeLabel = Label::createWithSystemFont("Upgrade", "Arial", 16);
    upgradeLabel->setColor(Color3B::WHITE);
    auto upgradeButton = MenuItemLabel::create(upgradeLabel, [this](Ref* sender) {
        if (_selectedBuilding) {
            upgradeBuilding(_selectedBuilding);
        }
    });
    auto upgradeBg = DrawNode::create();
    upgradeBg->drawSolidRect(Vec2(-50, -15), Vec2(50, 15), Color4F(0.2f, 0.5f, 0.3f, 1.0f));
    upgradeButton->addChild(upgradeBg, -1);
    upgradeButton->setPosition(Vec2(70, 40));
    buttons.pushBack(upgradeButton);
    
    // 拆除按钮
    auto demolishLabel = Label::createWithSystemFont("Demolish", "Arial", 16);
    demolishLabel->setColor(Color3B::WHITE);
    auto demolishButton = MenuItemLabel::create(demolishLabel, [this](Ref* sender) {
        if (_selectedBuilding) {
            demolishBuilding(_selectedBuilding);
        }
    });
    auto demolishBg = DrawNode::create();
    demolishBg->drawSolidRect(Vec2(-50, -15), Vec2(50, 15), Color4F(0.5f, 0.2f, 0.2f, 1.0f));
    demolishButton->addChild(demolishBg, -1);
    demolishButton->setPosition(Vec2(180, 40));
    buttons.pushBack(demolishButton);
    
    auto menu = Menu::createWithArray(buttons);
    menu->setPosition(Vec2::ZERO);
    _buildingInfoPanel->addChild(menu);
    
    // 设置位置(右上角)
    _buildingInfoPanel->setPosition(Vec2(origin.x + visibleSize.width - panelWidth - 20,
                                        origin.y + visibleSize.height - panelHeight - 70));
    _uiLayer->addChild(_buildingInfoPanel);
}

void BaseScene::hideBuildingInfoPanel() {
    if (_buildingInfoPanel) {
        _buildingInfoPanel->removeFromParent();
        _buildingInfoPanel = nullptr;
    }
}

void BaseScene::upgradeBuilding(Building* building) {
    if (!building) return;
    
    if (building->startUpgrade()) {
        // 升级成功
        showBuildingInfoPanel(building);  // 刷新面板
        updateResourceDisplay();
    }
}

void BaseScene::demolishBuilding(Building* building) {
    if (!building) return;
    
    // 大本营不能拆除
    if (building->getBuildingType() == BuildingType::TOWN_HALL) {
        return;
    }
    
    // 返还资源
    auto refund = building->demolish();
    ResourceManager::getInstance()->addResource(ResourceType::GOLD, refund.first);
    ResourceManager::getInstance()->addResource(ResourceType::ELIXIR, refund.second);
    
    // 移除建筑
    deselectBuilding();
    removeBuilding(building);
    updateResourceDisplay();
}

// ==================== 地图操作 ====================

bool BaseScene::isGridAvailable(int gridX, int gridY, int width, int height, Building* exclude) {
    // 检查边界
    if (gridX < 0 || gridY < 0 || 
        gridX + width > MAP_WIDTH || gridY + height > MAP_HEIGHT) {
        return false;
    }
    
    // 检查是否被占用
    for (int x = gridX; x < gridX + width; x++) {
        for (int y = gridY; y < gridY + height; y++) {
            int key = y * MAP_WIDTH + x;
            if (_gridOccupied.find(key) != _gridOccupied.end() && _gridOccupied[key]) {
                // 如果是排除的建筑占用的格子，跳过检查
                if (exclude) {
                    Vec2 exPos = exclude->getGridPosition();
                    int exW = exclude->getGridWidth();
                    int exH = exclude->getGridHeight();
                    if (x >= exPos.x && x < exPos.x + exW &&
                        y >= exPos.y && y < exPos.y + exH) {
                        continue;
                    }
                }
                return false;
            }
        }
    }
    
    return true;
}

Vec2 BaseScene::screenToGrid(const Vec2& screenPos) {
    Vec2 mapPos = screenPos - _cameraOffset;
    int gridX = (int)(mapPos.x / GRID_SIZE);
    int gridY = (int)(mapPos.y / GRID_SIZE);
    return Vec2(gridX, gridY);
}

Vec2 BaseScene::gridToScreen(const Vec2& gridPos) {
    float x = gridPos.x * GRID_SIZE + _cameraOffset.x;
    float y = gridPos.y * GRID_SIZE + _cameraOffset.y;
    return Vec2(x, y);
}

// ==================== 输入处理 ====================

bool BaseScene::onTouchBegan(Touch* touch, Event* event) {
    Vec2 touchPos = touch->getLocation();
    _lastTouchPos = touchPos;
    _isDragging = false;
    
    // 检查是否在UI区域
    if (touchPos.y < 60) {  // 工具栏区域
        return false;
    }
    
    // 如果在建造模式
    if (_isBuildMode) {
        Vec2 gridPos = screenToGrid(touchPos);
        
        // 尝试放置建筑
        if (addBuilding(_buildingTypeToPlace, (int)gridPos.x, (int)gridPos.y)) {
            // 建造成功，退出建造模式
            _isBuildMode = false;
            _gridLayer->setVisible(false);
            if (_buildPreview) {
                _buildPreview->removeFromParent();
                _buildPreview = nullptr;
            }
        }
        return true;
    }
    
    return true;
}

void BaseScene::onTouchMoved(Touch* touch, Event* event) {
    Vec2 touchPos = touch->getLocation();
    Vec2 delta = touchPos - _lastTouchPos;
    
    // 如果移动距离超过阈值，认为是拖动
    if (delta.length() > 5) {
        _isDragging = true;
    }
    
    if (_isDragging && !_isBuildMode) {
        // 拖动地图
        _cameraOffset += delta;
        _mapLayer->setPosition(_cameraOffset);
    }
    
    _lastTouchPos = touchPos;
}

void BaseScene::onTouchEnded(Touch* touch, Event* event) {
    if (!_isDragging && !_isBuildMode) {
        // 点击，检查是否点击了建筑
        Vec2 touchPos = touch->getLocation();
        Vec2 mapPos = touchPos - _cameraOffset;
        
        bool hitBuilding = false;
        for (auto building : _buildings) {
            if (!building) continue;
            
            Rect rect = building->getBoundingBox();
            if (rect.containsPoint(mapPos)) {
                selectBuilding(building);
                hitBuilding = true;
                break;
            }
        }
        
        if (!hitBuilding) {
            deselectBuilding();
        }
    }
    
    _isDragging = false;
}

void BaseScene::onMouseScroll(Event* event) {
    auto mouseEvent = static_cast<EventMouse*>(event);
    float scrollY = mouseEvent->getScrollY();
    
    // 缩放地图
    float oldZoom = _cameraZoom;
    _cameraZoom += scrollY * 0.1f;
    _cameraZoom = std::max(0.5f, std::min(2.0f, _cameraZoom));
    
    if (_cameraZoom != oldZoom) {
        _mapLayer->setScale(_cameraZoom);
    }
}

// ==================== UI回调 ====================

void BaseScene::onBuildMenuOpen(Ref* sender) {
    _buildMenu->setVisible(true);
}

void BaseScene::onBuildMenuClose(Ref* sender) {
    _buildMenu->setVisible(false);
}

void BaseScene::onSelectBuildingType(BuildingType type) {
    _buildMenu->setVisible(false);
    _isBuildMode = true;
    _buildingTypeToPlace = type;
    _gridLayer->setVisible(true);
    
    // 创建建筑预览
    // TODO: 显示建筑预览跟随鼠标
}

void BaseScene::onAttack(Ref* sender) {
    // 切换到战斗场景
    auto scene = BattleScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
}

void BaseScene::onHelp(Ref* sender) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建帮助弹窗
    auto helpPanel = Node::create();
    helpPanel->setName("helpPanel");
    
    // 背景遮罩
    auto mask = DrawNode::create();
    mask->drawSolidRect(Vec2(0, 0), Vec2(visibleSize.width, visibleSize.height),
                        Color4F(0, 0, 0, 0.7f));
    helpPanel->addChild(mask);
    
    // 面板尺寸
    const float panelWidth = 650;
    const float panelHeight = 450;
    const float panelX = (visibleSize.width - panelWidth) / 2;
    const float panelY = (visibleSize.height - panelHeight) / 2;
    
    // 面板背景
    auto panel = DrawNode::create();
    panel->drawSolidRect(Vec2(panelX, panelY), Vec2(panelX + panelWidth, panelY + panelHeight),
                         Color4F(0.12f, 0.12f, 0.18f, 0.98f));
    panel->drawRect(Vec2(panelX, panelY), Vec2(panelX + panelWidth, panelY + panelHeight),
                    Color4F(0.4f, 0.6f, 0.8f, 1.0f));
    helpPanel->addChild(panel);
    
    // 标题
    auto titleLabel = Label::createWithSystemFont("Base Building Help", "Arial", 28);
    titleLabel->setPosition(Vec2(visibleSize.width / 2, panelY + panelHeight - 35));
    titleLabel->setColor(Color3B(255, 215, 0));
    helpPanel->addChild(titleLabel);
    
    // 帮助内容
    std::string helpText = 
        "=== Building Your Base ===\n"
        "- Click [Build] to open the building menu\n"
        "- Select a building type and place it on the map\n"
        "- Drag to move the camera around your base\n"
        "- Scroll wheel to zoom in/out\n\n"
        
        "=== Building Management ===\n"
        "- Click on a building to select it\n"
        "- Upgrade buildings to increase their stats\n"
        "- Gold Mine/Elixir Collector produce resources over time\n"
        "- Click [Collect] to gather all produced resources\n\n"
        
        "=== Starting Battle ===\n"
        "- Click [Attack] to enter battle mode\n"
        "- You will attack an enemy base to earn resources\n"
        "- Train troops in Barracks before attacking";
    
    auto helpLabel = Label::createWithSystemFont(helpText, "Arial", 15);
    helpLabel->setPosition(Vec2(visibleSize.width / 2, panelY + panelHeight / 2 - 10));
    helpLabel->setColor(Color3B(220, 220, 220));
    helpLabel->setAlignment(TextHAlignment::LEFT, TextVAlignment::TOP);
    helpLabel->setMaxLineWidth(panelWidth - 50);
    helpPanel->addChild(helpLabel);
    
    // 关闭按钮
    auto closeLabel = Label::createWithSystemFont("Close", "Arial", 20);
    closeLabel->setColor(Color3B::WHITE);
    auto closeButton = MenuItemLabel::create(closeLabel, [helpPanel](Ref* sender) {
        helpPanel->removeFromParent();
    });
    auto closeBg = DrawNode::create();
    closeBg->drawSolidRect(Vec2(-55, -18), Vec2(55, 18), Color4F(0.5f, 0.3f, 0.3f, 1.0f));
    closeBg->drawRect(Vec2(-55, -18), Vec2(55, 18), Color4F(0.7f, 0.4f, 0.4f, 1.0f));
    closeButton->addChild(closeBg, -1);
    closeButton->setPosition(Vec2(visibleSize.width / 2, panelY + 40));
    
    auto menu = Menu::create(closeButton, nullptr);
    menu->setPosition(Vec2::ZERO);
    helpPanel->addChild(menu);
    
    helpPanel->setPosition(origin);
    this->addChild(helpPanel, Z_POPUP);
}

void BaseScene::onBackToMenu(Ref* sender) {
    auto scene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
}

void BaseScene::onCollectAll(Ref* sender) {
    // 收集所有生产建筑的资源
    for (auto building : _buildings) {
        if (auto prodBuilding = dynamic_cast<ProductionBuilding*>(building)) {
            prodBuilding->collectResource();
        }
    }
    updateResourceDisplay();
}

// ==================== 资源更新 ====================

void BaseScene::updateResourceDisplay() {
    auto rm = ResourceManager::getInstance();
    
    char buf[64];
    
    // 金币
    snprintf(buf, sizeof(buf), "%d/%d", rm->getResource(ResourceType::GOLD), 
             rm->getMaxResource(ResourceType::GOLD));
    if (_goldLabel) _goldLabel->setString(buf);
    
    // 圣水
    snprintf(buf, sizeof(buf), "%d/%d", rm->getResource(ResourceType::ELIXIR), 
             rm->getMaxResource(ResourceType::ELIXIR));
    if (_elixirLabel) _elixirLabel->setString(buf);
    
    // 工人
    snprintf(buf, sizeof(buf), "%d/%d", rm->getAvailableWorkers(), rm->getTotalWorkers());
    if (_workerLabel) _workerLabel->setString(buf);
    
    // 人口
    snprintf(buf, sizeof(buf), "%d/%d", rm->getCurrentPopulation(), rm->getMaxPopulation());
    if (_populationLabel) _populationLabel->setString(buf);
}

void BaseScene::onResourceChanged(ResourceType type, int oldValue, int newValue) {
    updateResourceDisplay();
}

// ==================== 帧更新 ====================

void BaseScene::update(float dt) {
    // 更新所有建筑
    // (建筑自己会在update中更新，这里可以添加其他逻辑)
}
