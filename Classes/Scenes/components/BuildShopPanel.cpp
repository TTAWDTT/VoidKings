/**
 * @file BuildShopPanel.cpp
 * @brief �����̵����ʵ��
 *
 * �������񲼾���ʾ����ͼ�꣬ÿ�������������
 * - ����ͼ�꣨logo��
 * - ��������
 * - ��������
 * - ���ӳߴ���Ϣ
 */

#include "BuildShopPanel.h"
#include "Utils/AudioManager.h"
#include <algorithm>
#include <memory>

 // ===================================================
 // �����ͳ�ʼ��
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

    // ��ʼ������ѡ������ͻ�趨��
    initBuildingOptions();

    // ��ʼ������UI���
    setupBackground();
    setupPanel();
    setupTitle();
    setupBuildingGrid();
    setupCloseButton();

    // ��ʼ����
    this->setVisible(false);

    CCLOG("[�����̵�] ����ʼ�����");

    return true;
}

// ===================================================
// ��ʼ������ѡ������ͻ�趨��
// ===================================================
void BuildShopPanel::initBuildingOptions() {
    // ���ݲ����ͻ�Ľ����ߴ��趨��
    // - �������ࣨ�����������ȣ���3x3����
    // - ��Դ�������ֿ�ȣ���3x3����
    // - ��Ӫ��5x5���ӣ����ɽ��죬����Ĭ�ϱ�Ӫ��
    // - ��Ӫ��4x4���ӣ����ɽ��죬����Ĭ�ϻ��أ�
    // - װ������ȣ���2x2����

    _buildingOptions = {
        {1, "Arrow Tower", 100, 3, 3, "buildings/ArrowTower.png", true},      // ���� 3x3
        {2, "Boom Tower", 150, 3, 3, "buildings/BoomTower.png", true},        // ���� 3x3
        {3, "Tree", 80, 2, 2, "buildings/Tree/sprite_0000.png", true},        // װ���� 2x2
        {4, "Storage", 200, 3, 3, "buildings/snowman.png", true},             // �ֿ� 3x3
        {5, "Barracks", 300, 5, 5, "buildings/soldierbuilder.png", false}     // ��Ӫ 5x5�����ɽ��죩
        {6, "GoldMaker", 250, 3, 3, "buildings/GoldMaker.png", true},         // 金币工厂 3x3
        {7, "DiamondMaker", 400, 3, 3, "buildings/DiamondMaker.png", true}    // 钻石工厂 3x3
    };

    CCLOG("[�����̵�] ��ʼ�� %zu ������ѡ��", _buildingOptions.size());
}

// ===================================================
// ���ý����Ƿ�ɽ���
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
// ���ñ��� - ��͸�����ָ���ȫ��
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
// ����������� - �ڰ׼�Լ���
// ===================================================
void BuildShopPanel::setupPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // ����� - ���ɫ����
    _panel = LayerColor::create(
        Color4B(40, 40, 40, 255),
        BuildShopConfig::PANEL_SIZE.width,
        BuildShopConfig::PANEL_SIZE.height
    );

    // ���ж�λ
    float panelX = origin.x + (visibleSize.width - BuildShopConfig::PANEL_SIZE.width) / 2;
    float panelY = origin.y + (visibleSize.height - BuildShopConfig::PANEL_SIZE.height) / 2;
    _panel->setPosition(panelX, panelY);

    // ���ӱ߿�Ч��
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
// �������ã���С�汾��
// ===================================================
void BuildShopPanel::setupTitle() {
    // ���ⱳ��������С�߶ȣ�
    auto titleBg = LayerColor::create(
        Color4B(60, 60, 60, 255),
        BuildShopConfig::PANEL_SIZE.width,
        35.0f
    );
    titleBg->setPosition(Vec2(0, BuildShopConfig::PANEL_SIZE.height - 35));
    _panel->addChild(titleBg);

    // ��������
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
// ���ý������񲼾�
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
// �����������������Сͼ�꣩- ��С�汾
// ===================================================
Node* BuildShopPanel::createBuildingGridItem(const BuildingOption& option, int row, int col) {
    auto itemNode = Node::create();

    // ����λ��
    float itemWidth = BuildShopConfig::GRID_ITEM_SIZE;
    float itemHeight = BuildShopConfig::GRID_ITEM_SIZE;
    float spacing = BuildShopConfig::GRID_SPACING;

    // ������ʼλ��ʹ�������
    float totalWidth = BuildShopConfig::GRID_COLS * itemWidth + (BuildShopConfig::GRID_COLS - 1) * spacing;
    float startX = (BuildShopConfig::PANEL_SIZE.width - totalWidth) / 2 + itemWidth / 2;

    // ��������������У����⼷������/��ť
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

    // �������� - �ڰ׷��
    Color4B bgColor = option.canBuild ? Color4B(60, 60, 60, 255) : Color4B(40, 40, 40, 255);
    auto bg = LayerColor::create(bgColor, itemWidth, itemHeight);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    itemNode->addChild(bg);

    // �߿�
    auto border = DrawNode::create();
    Color4F borderColor = option.canBuild ? Color4F::WHITE : Color4F(0.5f, 0.5f, 0.5f, 1.0f);
    border->drawRect(
        Vec2(-itemWidth / 2, -itemHeight / 2),
        Vec2(itemWidth / 2, itemHeight / 2),
        borderColor
    );
    itemNode->addChild(border, 1);

    // ����ͼ�꣨��С�汾��
    auto icon = Sprite::create(option.spritePath);
    if (icon) {
        // ���Ž���ͼ������Ӧͼ������
        float iconTargetSize = BuildShopConfig::ICON_SIZE;
        float scaleX = iconTargetSize / icon->getContentSize().width;
        float scaleY = iconTargetSize / icon->getContentSize().height;
        float scale = std::min(scaleX, scaleY);
        icon->setScale(scale);
        icon->setPosition(Vec2(0, 8));

        // ������ɽ��죬���û�ɫ
        if (!option.canBuild) {
            icon->setColor(Color3B(100, 100, 100));
        }

        itemNode->addChild(icon, 2);
    }

    // �������ƣ���С���壩
    auto nameLabel = Label::createWithTTF(option.name, "fonts/arial.ttf", 9);
    if (!nameLabel) {
        nameLabel = Label::createWithSystemFont(option.name, "Arial", 9);
    }
    nameLabel->setPosition(Vec2(0, -20));
    nameLabel->setColor(option.canBuild ? Color3B::WHITE : Color3B::GRAY);
    nameLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    itemNode->addChild(nameLabel, 2);
    // ������ʾ
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

    // �ߴ���Ϣ
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

    // ������ɽ��죬����"����"��ǩ
    if (!option.canBuild) {
        auto lockedLabel = Label::createWithTTF("Owned", "fonts/arial.ttf", 8);
        if (!lockedLabel) {
            lockedLabel = Label::createWithSystemFont("Owned", "Arial", 8);
        }
        lockedLabel->setPosition(Vec2(0, 25));
        lockedLabel->setColor(Color3B::RED);
        itemNode->addChild(lockedLabel, 3);
    }

    // ����͸�������ť
    auto touchBtn = Button::create();
    touchBtn->setContentSize(Size(itemWidth, itemHeight));
    touchBtn->setScale9Enabled(true);
    touchBtn->setPosition(Vec2(0, 0));

    // �󶨵���¼�
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
            AudioManager::playButtonCancel();
            CCLOG("[???????] ?y???????��?????????: %s", optionCopy.name.c_str());
            return;
        }

        AudioManager::playButtonClick();
        CCLOG("[???????] ?????: %s (???: %dx%d)",
            optionCopy.name.c_str(), optionCopy.gridWidth, optionCopy.gridHeight);

        if (_onBuildingSelected) {
            _onBuildingSelected(optionCopy);
        }
        this->hide();
    });

    itemNode->addChild(touchBtn, 10);

    // ��ͣ����
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
// �رհ�ť���ã���С�汾��
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
        AudioManager::playButtonCancel();
        this->hide();
        if (_onClose) {
            _onClose();
        }
        });

    _panel->addChild(closeBtn, 5);
}

// ===================================================
// ��ʾ���
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

    CCLOG("[�����̵�] ��ʾ���");
}

// ===================================================
// �������
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

    CCLOG("[�����̵�] �������");
}
