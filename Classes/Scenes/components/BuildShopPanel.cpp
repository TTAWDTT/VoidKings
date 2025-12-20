/**
 * @file BuildShopPanel.cpp
 * @brief 建筑商店面板实现
 * 
 * 采用网格布局显示建筑图标，每个建筑项包含：
 * - 建筑图标（logo）
 * - 建筑名称
 * - 建造费用
 * - 格子尺寸信息
 */

#include "BuildShopPanel.h"

// ===================================================
// 创建与初始化
// ===================================================

BuildShopPanel* BuildShopPanel::create(
    const std::function<void(const BuildingOption&)>& onBuildingSelected,
    const std::function<void()>& onClose
) {
    BuildShopPanel* ret = new (std::nothrow) BuildShopPanel();
    if (ret && ret->init(onBuildingSelected, onClose)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool BuildShopPanel::init(
    const std::function<void(const BuildingOption&)>& onBuildingSelected,
    const std::function<void()>& onClose
) {
    if (!Node::init()) {
        return false;
    }

    _onBuildingSelected = onBuildingSelected;
    _onClose = onClose;
    _isShowing = false;

    // 初始化建筑选项（按部落冲突设定）
    initBuildingOptions();

    // 初始化各个UI组件
    setupBackground();
    setupPanel();
    setupTitle();
    setupBuildingGrid();
    setupCloseButton();

    // 初始隐藏
    this->setVisible(false);

    CCLOG("[建筑商店] 面板初始化完成");

    return true;
}

// ===================================================
// 初始化建筑选项（按部落冲突设定）
// ===================================================
void BuildShopPanel::initBuildingOptions() {
    // 按照部落冲突的建筑尺寸设定：
    // - 防御塔类（箭塔、炮塔等）：3x3格子
    // - 资源建筑（仓库等）：3x3格子
    // - 兵营：5x5格子（不可建造，已有默认兵营）
    // - 大本营：4x4格子（不可建造，已有默认基地）
    // - 装饰物（树等）：2x2格子

    _buildingOptions = {
        {1, "Arrow Tower", 100, 3, 3, "buildings/ArrowTower.png", true},      // 箭塔 3x3
        {2, "Boom Tower", 150, 3, 3, "buildings/BoomTower.png", true},        // 炮塔 3x3
        {3, "Tree", 80, 2, 2, "buildings/Tree/sprite_0000.png", true},        // 装饰树 2x2
        {4, "Storage", 200, 3, 3, "buildings/snowman.png", true},             // 仓库 3x3
        {5, "Barracks", 300, 5, 5, "buildings/soldierbuilder.png", false}     // 兵营 5x5（不可建造）
    };

    CCLOG("[建筑商店] 初始化 %zu 个建筑选项", _buildingOptions.size());
}

// ===================================================
// 设置建筑是否可建造
// ===================================================
void BuildShopPanel::setBuildingCanBuild(int type, bool canBuild) {
    for (auto& option : _buildingOptions) {
        if (option.type == type) {
            option.canBuild = canBuild;
            break;
        }
    }
}

// ===================================================
// 背景设置 - 半透明遮罩覆盖全屏
// ===================================================
void BuildShopPanel::setupBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _background = LayerColor::create(
        Color4B(0, 0, 0, 200),
        visibleSize.width,
        visibleSize.height
    );
    _background->setPosition(origin);
    this->addChild(_background, 0);
}

// ===================================================
// 面板主体设置 - 黑白网格风格
// ===================================================
void BuildShopPanel::setupPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 主面板 - 深灰色背景
    _panel = LayerColor::create(
        Color4B(40, 40, 40, 255),
        BuildShopConfig::PANEL_SIZE.width,
        BuildShopConfig::PANEL_SIZE.height
    );

    // 居中定位
    float panelX = origin.x + (visibleSize.width - BuildShopConfig::PANEL_SIZE.width) / 2;
    float panelY = origin.y + (visibleSize.height - BuildShopConfig::PANEL_SIZE.height) / 2;
    _panel->setPosition(panelX, panelY);

    // 添加边框效果
    auto border = DrawNode::create();
    border->drawRect(
        Vec2(0, 0),
        Vec2(BuildShopConfig::PANEL_SIZE.width, BuildShopConfig::PANEL_SIZE.height),
        Color4F::WHITE
    );
    _panel->addChild(border, 1);

    this->addChild(_panel, 1);
}

// ===================================================
// 标题设置
// ===================================================
void BuildShopPanel::setupTitle() {
    // 标题背景条
    auto titleBg = LayerColor::create(
        Color4B(60, 60, 60, 255),
        BuildShopConfig::PANEL_SIZE.width,
        50.0f
    );
    titleBg->setPosition(Vec2(0, BuildShopConfig::PANEL_SIZE.height - 50));
    _panel->addChild(titleBg);

    // 标题文字
    _titleLabel = Label::createWithTTF(
        "Building Shop",
        "fonts/arial.ttf",
        BuildShopConfig::TITLE_FONT_SIZE
    );
    if (!_titleLabel) {
        _titleLabel = Label::createWithSystemFont("Building Shop", "Arial", BuildShopConfig::TITLE_FONT_SIZE);
    }
    _titleLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    _titleLabel->setPosition(
        BuildShopConfig::PANEL_SIZE.width / 2,
        BuildShopConfig::PANEL_SIZE.height - 25
    );
    _titleLabel->setColor(Color3B::WHITE);
    _panel->addChild(_titleLabel, 2);
}

// ===================================================
// 建筑网格布局
// ===================================================
void BuildShopPanel::setupBuildingGrid() {
    _gridContainer = Node::create();
    _panel->addChild(_gridContainer);

    int row = 0;
    int col = 0;

    for (const auto& option : _buildingOptions) {
        auto gridItem = createBuildingGridItem(option, row, col);
        if (gridItem) {
            _gridContainer->addChild(gridItem);
        }

        col++;
        if (col >= BuildShopConfig::GRID_COLS) {
            col = 0;
            row++;
        }
    }
}

// ===================================================
// 创建单个建筑网格项（带图标）
// ===================================================
Node* BuildShopPanel::createBuildingGridItem(const BuildingOption& option, int row, int col) {
    auto itemNode = Node::create();

    // 计算位置
    float itemWidth = BuildShopConfig::GRID_ITEM_SIZE;
    float itemHeight = BuildShopConfig::GRID_ITEM_SIZE;
    float spacing = BuildShopConfig::GRID_SPACING;

    // 计算起始位置使网格居中
    float totalWidth = BuildShopConfig::GRID_COLS * itemWidth + (BuildShopConfig::GRID_COLS - 1) * spacing;
    float startX = (BuildShopConfig::PANEL_SIZE.width - totalWidth) / 2 + itemWidth / 2;

    float x = startX + col * (itemWidth + spacing);
    float y = BuildShopConfig::GRID_START_Y - row * (itemHeight + spacing);

    itemNode->setPosition(Vec2(x, y));

    // 网格项背景 - 黑白网格风格
    Color4B bgColor = option.canBuild ? Color4B(60, 60, 60, 255) : Color4B(40, 40, 40, 255);
    auto bg = LayerColor::create(bgColor, itemWidth, itemHeight);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    itemNode->addChild(bg);

    // 边框
    auto border = DrawNode::create();
    Color4F borderColor = option.canBuild ? Color4F::WHITE : Color4F(0.5f, 0.5f, 0.5f, 1.0f);
    border->drawRect(
        Vec2(-itemWidth / 2, -itemHeight / 2),
        Vec2(itemWidth / 2, itemHeight / 2),
        borderColor
    );
    itemNode->addChild(border, 1);

    // 建筑图标
    auto icon = Sprite::create(option.spritePath);
    if (icon) {
        // 计算缩放以适应图标区域
        float iconTargetSize = BuildShopConfig::ICON_SIZE;
        float scaleX = iconTargetSize / icon->getContentSize().width;
        float scaleY = iconTargetSize / icon->getContentSize().height;
        float scale = std::min(scaleX, scaleY);
        icon->setScale(scale);
        icon->setPosition(Vec2(0, 15));

        // 如果不可建造，设置灰色
        if (!option.canBuild) {
            icon->setColor(Color3B(100, 100, 100));
        }

        itemNode->addChild(icon, 2);
    }

    // 建筑名称
    auto nameLabel = Label::createWithTTF(option.name, "fonts/arial.ttf", 14);
    if (!nameLabel) {
        nameLabel = Label::createWithSystemFont(option.name, "Arial", 14);
    }
    nameLabel->setPosition(Vec2(0, -35));
    nameLabel->setColor(option.canBuild ? Color3B::WHITE : Color3B::GRAY);
    nameLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    itemNode->addChild(nameLabel, 2);

    // 费用和尺寸信息
    char infoText[64];
    snprintf(infoText, sizeof(infoText), "%dG %dx%d", option.cost, option.gridWidth, option.gridHeight);
    auto infoLabel = Label::createWithTTF(infoText, "fonts/arial.ttf", 11);
    if (!infoLabel) {
        infoLabel = Label::createWithSystemFont(infoText, "Arial", 11);
    }
    infoLabel->setPosition(Vec2(0, -52));
    infoLabel->setColor(option.canBuild ? Color3B::YELLOW : Color3B::GRAY);
    infoLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    itemNode->addChild(infoLabel, 2);

    // 如果不可建造，添加"已有"标签
    if (!option.canBuild) {
        auto lockedLabel = Label::createWithTTF("Owned", "fonts/arial.ttf", 12);
        if (!lockedLabel) {
            lockedLabel = Label::createWithSystemFont("Owned", "Arial", 12);
        }
        lockedLabel->setPosition(Vec2(0, 40));
        lockedLabel->setColor(Color3B::RED);
        itemNode->addChild(lockedLabel, 3);
    }

    // 创建透明点击区域
    auto touchBtn = Button::create();
    touchBtn->setContentSize(Size(itemWidth, itemHeight));
    touchBtn->setScale9Enabled(true);
    touchBtn->setPosition(Vec2(0, 0));

    // 绑定点击事件
    BuildingOption optionCopy = option;
    touchBtn->addClickEventListener([this, optionCopy](Ref* sender) {
        if (!optionCopy.canBuild) {
            CCLOG("[建筑商店] 该建筑已拥有，无法再建造: %s", optionCopy.name.c_str());
            return;
        }

        CCLOG("[建筑商店] 选择建筑: %s (尺寸: %dx%d)",
            optionCopy.name.c_str(), optionCopy.gridWidth, optionCopy.gridHeight);

        if (_onBuildingSelected) {
            _onBuildingSelected(optionCopy);
        }
        this->hide();
    });

    itemNode->addChild(touchBtn, 10);

    return itemNode;
}

// ===================================================
// 关闭按钮设置
// ===================================================
void BuildShopPanel::setupCloseButton() {
    auto closeBtn = Button::create("btn_normal.png", "btn_pressed.png");
    if (!closeBtn) {
        closeBtn = Button::create();
        closeBtn->setScale9Enabled(true);
        closeBtn->setContentSize(Size(120, 40));
    }

    closeBtn->setTitleText("Close");
    closeBtn->setTitleFontSize(18);
    closeBtn->setTitleColor(Color3B::WHITE);
    closeBtn->setPosition(Vec2(
        BuildShopConfig::PANEL_SIZE.width / 2,
        BuildShopConfig::CLOSE_BUTTON_BOTTOM
    ));

    closeBtn->addClickEventListener([this](Ref* sender) {
        this->hide();
        if (_onClose) {
            _onClose();
        }
    });

    _panel->addChild(closeBtn, 5);
}

// ===================================================
// 显示面板
// ===================================================
void BuildShopPanel::show() {
    this->setVisible(true);
    _isShowing = true;
    CCLOG("[建筑商店] 显示面板");
}

// ===================================================
// 隐藏面板
// ===================================================
void BuildShopPanel::hide() {
    this->setVisible(false);
    _isShowing = false;
    CCLOG("[建筑商店] 隐藏面板");
}
