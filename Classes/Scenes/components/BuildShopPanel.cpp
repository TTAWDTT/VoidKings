/**
 * @file BuildShopPanel.cpp
 * @brief 建筑商店面板实现
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

    // 初始化建筑选项（按照部落冲突设定）
    initBuildingOptions();

    // 初始化各个UI组件
    setupBackground();
    setupPanel();
    setupTitle();
    setupBuildingButtons();
    setupCloseButton();

    // 初始隐藏
    this->setVisible(false);

    CCLOG("[建筑商店] 面板初始化完成");

    return true;
}

// ===================================================
// 初始化建筑选项（按照部落冲突设定）
// ===================================================
void BuildShopPanel::initBuildingOptions() {
    // 按照部落冲突的建筑尺寸设定：
    // - 防御塔类（箭塔、炮塔等）：3x3格子
    // - 资源建筑（仓库等）：3x3格子
    // - 兵营：5x5格子
    // - 大本营：4x4格子
    // - 装饰物（树等）：2x2格子

    _buildingOptions = {
        {1, "Arrow Tower (100G)", 100, 3, 3, "buildings/ArrowTower.png"},     // 箭塔 3x3
        {2, "Boom Tower (150G)", 150, 3, 3, "buildings/BoomTower.png"},       // 炮塔 3x3
        {3, "Tree (80G)", 80, 2, 2, "buildings/Tree/sprite_0000.png"},        // 装饰树 2x2
        {4, "Snowman Storage (200G)", 200, 3, 3, "buildings/snowman.png"},    // 仓库 3x3
        {5, "Soldier Builder (300G)", 300, 5, 5, "buildings/soldierbuilder.png"} // 兵营 5x5
    };

    CCLOG("[建筑商店] 初始化了 %zu 个建筑选项", _buildingOptions.size());
}

// ===================================================
// 背景设置 - 半透明遮罩覆盖全屏
// ===================================================
void BuildShopPanel::setupBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _background = LayerColor::create(
        Color4B(0, 0, 0, 180),
        visibleSize.width,
        visibleSize.height
    );
    _background->setPosition(origin);
    this->addChild(_background, 0);
}

// ===================================================
// 面板主体设置 - 居中显示的主面板
// ===================================================
void BuildShopPanel::setupPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _panel = LayerColor::create(
        Color4B(50, 50, 50, 255),
        BuildShopConfig::PANEL_SIZE.width,
        BuildShopConfig::PANEL_SIZE.height
    );

    // 居中定位
    float panelX = origin.x + (visibleSize.width - BuildShopConfig::PANEL_SIZE.width) / 2;
    float panelY = origin.y + (visibleSize.height - BuildShopConfig::PANEL_SIZE.height) / 2;
    _panel->setPosition(panelX, panelY);

    this->addChild(_panel, 1);
}

// ===================================================
// 标题设置
// ===================================================
void BuildShopPanel::setupTitle() {
    _titleLabel = Label::createWithSystemFont(
        "Building Shop",
        "ScienceGothic",
        BuildShopConfig::TITLE_FONT_SIZE
    );
    _titleLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    _titleLabel->setPosition(
        BuildShopConfig::PANEL_SIZE.width / 2,
        BuildShopConfig::PANEL_SIZE.height - 20
    );
    _titleLabel->setColor(Color3B::WHITE);
    _panel->addChild(_titleLabel);
}

// ===================================================
// 建筑按钮设置
// ===================================================
void BuildShopPanel::setupBuildingButtons() {
    float yPos = BuildShopConfig::BUTTON_START_Y;

    for (const auto& option : _buildingOptions) {
        auto btn = createBuildingButton(option, yPos);
        if (btn) {
            _panel->addChild(btn);
        }
        yPos -= BuildShopConfig::BUTTON_SPACING;
    }
}

// ===================================================
// 创建单个建筑按钮
// ===================================================
Button* BuildShopPanel::createBuildingButton(const BuildingOption& option, float yPos) {
    auto btn = Button::create("btn_normal.png", "btn_pressed.png");
    if (!btn) {
        btn = Button::create();
    }

    // 设置按钮文字（显示建筑名称和尺寸信息）
    // 使用snprintf避免缓冲区溢出
    char buttonText[128];
    snprintf(buttonText, sizeof(buttonText), "%s (%dx%d)", option.name.c_str(), option.gridWidth, option.gridHeight);
    btn->setTitleText(buttonText);
    btn->setTitleFontSize(BuildShopConfig::BUTTON_FONT_SIZE);
    btn->setScale(1.5f);
    btn->setPosition(Vec2(BuildShopConfig::PANEL_SIZE.width / 2, yPos));

    // 绑定点击事件
    BuildingOption optionCopy = option;  // 复制一份用于lambda
    btn->addClickEventListener([this, optionCopy](Ref* sender) {
        CCLOG("[建筑商店] 选择建筑: %s (尺寸: %dx%d)",
            optionCopy.name.c_str(), optionCopy.gridWidth, optionCopy.gridHeight);

        if (_onBuildingSelected) {
            _onBuildingSelected(optionCopy);
        }
        this->hide();
        });

    return btn;
}

// ===================================================
// 关闭按钮设置
// ===================================================
void BuildShopPanel::setupCloseButton() {
    auto closeBtn = Button::create("btn_normal.png", "btn_pressed.png");
    if (!closeBtn) {
        closeBtn = Button::create();
    }

    closeBtn->setTitleText("Close");
    closeBtn->setTitleFontSize(24);
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

    _panel->addChild(closeBtn);
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