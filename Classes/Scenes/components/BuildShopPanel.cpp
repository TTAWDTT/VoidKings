/**
 * @file BuildShopPanel.cpp
 * @brief 建筑商店面板实现
 *
 * 采用网格布局显示建筑图标，每个建筑项包含：
 * - 建筑图标（logo）
 * - 建筑名称
 * - 建筑费用
 * - 格子尺寸信息
 */

#include "BuildShopPanel.h"
#include <algorithm>
#include <memory>

 // ===================================================
 // 创建和初始化
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

    // 初始化建筑选项（部落冲突设定）
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
// 初始化建筑选项（部落冲突设定）
// ===================================================
void BuildShopPanel::initBuildingOptions() {
    // 根据部落冲突的建筑尺寸设定：
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
// 设置背景 - 半透明遮罩覆盖全屏
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
// 设置面板主体 - 黑白简约风格
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
// 标题设置（缩小版本）
// ===================================================
void BuildShopPanel::setupTitle() {
    // 标题背景条（缩小高度）
    auto titleBg = LayerColor::create(
        Color4B(60, 60, 60, 255),
        BuildShopConfig::PANEL_SIZE.width,
        35.0f
    );
    titleBg->setPosition(Vec2(0, BuildShopConfig::PANEL_SIZE.height - 35));
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
        BuildShopConfig::PANEL_SIZE.height - 18
    );
    _titleLabel->setColor(Color3B::WHITE);
    _panel->addChild(_titleLabel, 2);
}

// ===================================================
// 设置建筑网格布局
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
// 创建建筑网格项（包含小图标）- 缩小版本
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

    // 网格区域纵向居中，避免挤到标题/按钮
    int totalItems = static_cast<int>(_buildingOptions.size());
    int totalRows = (totalItems + BuildShopConfig::GRID_COLS - 1) / BuildShopConfig::GRID_COLS;
    float gridHeight = totalRows * itemHeight + (totalRows - 1) * spacing;
    float topY = BuildShopConfig::PANEL_SIZE.height - 35.0f - 10.0f;
    float bottomY = BuildShopConfig::CLOSE_BUTTON_BOTTOM + 40.0f;
    float availableHeight = topY - bottomY;
    float startY = topY - (availableHeight - gridHeight) / 2 - itemHeight / 2;

    float x = startX + col * (itemWidth + spacing);
    float y = startY - row * (itemHeight + spacing);

    itemNode->setPosition(Vec2(x, y));

    // 背景方块 - 黑白风格
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

    // 建筑图标（缩小版本）
    auto icon = Sprite::create(option.spritePath);
    if (icon) {
        // 缩放建筑图标以适应图标区域
        float iconTargetSize = BuildShopConfig::ICON_SIZE;
        float scaleX = iconTargetSize / icon->getContentSize().width;
        float scaleY = iconTargetSize / icon->getContentSize().height;
        float scale = std::min(scaleX, scaleY);
        icon->setScale(scale);
        icon->setPosition(Vec2(0, 8));

        // 如果不可建造，设置灰色
        if (!option.canBuild) {
            icon->setColor(Color3B(100, 100, 100));
        }

        itemNode->addChild(icon, 2);
    }

    // 建筑名称（缩小字体）
    auto nameLabel = Label::createWithTTF(option.name, "fonts/arial.ttf", 9);
    if (!nameLabel) {
        nameLabel = Label::createWithSystemFont(option.name, "Arial", 9);
    }
    nameLabel->setPosition(Vec2(0, -20));
    nameLabel->setColor(option.canBuild ? Color3B::WHITE : Color3B::GRAY);
    nameLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    itemNode->addChild(nameLabel, 2);
    // 费用显示
    char costText[32];
    snprintf(costText, sizeof(costText), "%d", option.cost);
    auto costLabel = Label::createWithTTF(costText, "fonts/arial.ttf", 8);
    if (!costLabel) {
        costLabel = Label::createWithSystemFont(costText, "Arial", 8);
    }
    costLabel->setPosition(Vec2(8, -28));
    costLabel->setColor(option.canBuild ? Color3B::YELLOW : Color3B::GRAY);
    costLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    itemNode->addChild(costLabel, 2);

    auto coinIcon = Sprite::create("source/coin/coin_0001.png");
    if (coinIcon) {
        float targetSize = 10.0f;
        float scale = targetSize / std::max(coinIcon->getContentSize().width, coinIcon->getContentSize().height);
        coinIcon->setScale(scale);
        coinIcon->setPosition(Vec2(-10, -28));
        coinIcon->setColor(option.canBuild ? Color3B::WHITE : Color3B::GRAY);
        itemNode->addChild(coinIcon, 2);
    }

    // 尺寸信息
    char sizeText[32];
    snprintf(sizeText, sizeof(sizeText), "%dx%d", option.gridWidth, option.gridHeight);
    auto sizeLabel = Label::createWithTTF(sizeText, "fonts/arial.ttf", 8);
    if (!sizeLabel) {
        sizeLabel = Label::createWithSystemFont(sizeText, "Arial", 8);
    }
    sizeLabel->setPosition(Vec2(0, -40));
    sizeLabel->setColor(option.canBuild ? Color3B(210, 210, 210) : Color3B::GRAY);
    sizeLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    itemNode->addChild(sizeLabel, 2);

    // 如果不可建造，添加"已有"标签
    if (!option.canBuild) {
        auto lockedLabel = Label::createWithTTF("Owned", "fonts/arial.ttf", 8);
        if (!lockedLabel) {
            lockedLabel = Label::createWithSystemFont("Owned", "Arial", 8);
        }
        lockedLabel->setPosition(Vec2(0, 25));
        lockedLabel->setColor(Color3B::RED);
        itemNode->addChild(lockedLabel, 3);
    }

    // 创建透明点击按钮
    auto touchBtn = Button::create();
    touchBtn->setContentSize(Size(itemWidth, itemHeight));
    touchBtn->setScale9Enabled(true);
    touchBtn->setPosition(Vec2(0, 0));

    // 绑定点击事件
    BuildingOption optionCopy = option;
    touchBtn->setSwallowTouches(true);
    const float originScale = itemNode->getScale();
    touchBtn->addTouchEventListener([this, optionCopy, itemNode, originScale](Ref*, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::BEGAN) {
            if (optionCopy.canBuild) {
                itemNode->setScale(originScale * 0.97f);
            }
            return;
        }
        if (type == Widget::TouchEventType::CANCELED) {
            itemNode->setScale(originScale);
            return;
        }
        if (type != Widget::TouchEventType::ENDED) {
            return;
        }

        itemNode->setScale(originScale);

        if (!optionCopy.canBuild) {
            CCLOG("[???????] ?y???????У?????????: %s", optionCopy.name.c_str());
            return;
        }

        CCLOG("[???????] ?????: %s (???: %dx%d)",
            optionCopy.name.c_str(), optionCopy.gridWidth, optionCopy.gridHeight);

        if (_onBuildingSelected) {
            _onBuildingSelected(optionCopy);
        }
        this->hide();
    });

    itemNode->addChild(touchBtn, 10);

    // 悬停高亮
    bool canBuild = option.canBuild;
    auto hoverState = std::make_shared<bool>(false);
    auto hoverListener = EventListenerMouse::create();
    hoverListener->onMouseMove = [this, itemNode, bg, border, itemWidth, itemHeight, borderColor, bgColor, hoverState, canBuild](EventMouse* event) {
        if (!canBuild) {
            return;
        }
        Node* parent = itemNode->getParent();
        if (!parent) {
            return;
        }
        Vec2 localPos = parent->convertToNodeSpace(Vec2(event->getCursorX(), event->getCursorY()));
        bool inside = itemNode->getBoundingBox().containsPoint(localPos);

        if (inside && !*hoverState) {
            bg->setColor(Color3B(75, 75, 75));
            border->clear();
            border->drawRect(Vec2(-itemWidth / 2, -itemHeight / 2),
                Vec2(itemWidth / 2, itemHeight / 2),
                Color4F(0.9f, 0.9f, 0.9f, 1.0f));
            *hoverState = true;
        }
        else if (!inside && *hoverState) {
            bg->setColor(Color3B(bgColor.r, bgColor.g, bgColor.b));
            border->clear();
            border->drawRect(Vec2(-itemWidth / 2, -itemHeight / 2),
                Vec2(itemWidth / 2, itemHeight / 2),
                borderColor);
            *hoverState = false;
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(hoverListener, itemNode);

    return itemNode;
}

// ===================================================
// 关闭按钮设置（缩小版本）
// ===================================================
void BuildShopPanel::setupCloseButton() {
    auto closeBtn = Button::create("btn_normal.png", "btn_pressed.png");
    if (!closeBtn) {
        closeBtn = Button::create();
        closeBtn->setScale9Enabled(true);
        closeBtn->setContentSize(Size(80, 30));
    }

    closeBtn->setTitleText("Close");
    closeBtn->setTitleFontSize(14);
    closeBtn->setTitleColor(Color3B::WHITE);
    closeBtn->setPosition(Vec2(
        BuildShopConfig::PANEL_SIZE.width / 2,
        BuildShopConfig::CLOSE_BUTTON_BOTTOM
    ));

    closeBtn->setPressedActionEnabled(true);
    closeBtn->setZoomScale(0.05f);
    closeBtn->setSwallowTouches(true);

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

    if (_background) {
        _background->stopAllActions();
        _background->setOpacity(0);
        _background->runAction(FadeTo::create(0.18f, 200));
    }
    if (_panel) {
        _panel->stopAllActions();
        _panel->setScale(0.94f);
        _panel->runAction(EaseBackOut::create(ScaleTo::create(0.22f, 1.0f)));
    }

    CCLOG("[建筑商店] 显示面板");
}

// ===================================================
// 隐藏面板
// ===================================================
void BuildShopPanel::hide() {
    _isShowing = false;

    if (_background) {
        _background->stopAllActions();
        _background->runAction(FadeTo::create(0.12f, 0));
    }
    if (_panel) {
        _panel->stopAllActions();
        _panel->runAction(EaseBackIn::create(ScaleTo::create(0.12f, 0.94f)));
    }

    this->runAction(Sequence::create(
        DelayTime::create(0.14f),
        CallFunc::create([this]() {
            this->setVisible(false);
        }),
        nullptr));

    CCLOG("[建筑商店] 隐藏面板");
}
