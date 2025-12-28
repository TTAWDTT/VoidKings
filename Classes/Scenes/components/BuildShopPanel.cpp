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
#include "Buildings/BuildingManager.h"
#include "Utils/AudioManager.h"
#include <algorithm>
#include <memory>
#include <string>

namespace {
const char* kShopFont = "fonts/ScienceGothic.ttf";

Label* createShopLabel(const std::string& text, float fontSize) {
    auto label = Label::createWithTTF(text, kShopFont, fontSize);
    if (!label) {
        label = Label::create();
        label->setString(text);
        label->setSystemFontSize(fontSize);
    }
    return label;
}

float getFirstFloatValue(const std::vector<float>& values, float fallback) {
    if (values.empty()) {
        return fallback;
    }
    return values.front();
}

int getFirstIntValue(const std::vector<int>& values, int fallback) {
    if (values.empty()) {
        return fallback;
    }
    return values.front();
}

std::string toCategoryName(BuildingCategory category) {
    switch (category) {
    case BuildingCategory::Defence:
        return "Defense";
    case BuildingCategory::Production:
        return "Production";
    case BuildingCategory::Storage:
        return "Storage";
    case BuildingCategory::Trap:
        return "Trap";
    default:
        return "Unknown";
    }
}

std::string buildTargetText(bool ground, bool sky) {
    if (ground && sky) {
        return "Ground/Air";
    }
    if (ground) {
        return "Ground";
    }
    if (sky) {
        return "Air";
    }
    return "-";
}
} // namespace

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
    setupDetailPanel();
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
    auto* manager = BuildingManager::getInstance();
    manager->loadConfigs();
    _buildingOptions = manager->getBuildOptions();

    CCLOG("[BuildShop] Loaded %zu building options", _buildingOptions.size());
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
    _titleLabel = createShopLabel("Building Shop", BuildShopConfig::TITLE_FONT_SIZE);
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
// ===================================================
// 详情提示面板
// ===================================================
void BuildShopPanel::setupDetailPanel() {
    if (!_panel) {
        return;
    }

    _detailPanel = Node::create();
    _detailPanel->setAnchorPoint(Vec2(0.5f, 0.5f));
    _detailPanel->setIgnoreAnchorPointForPosition(false);
    _detailPanel->setVisible(false);
    _panel->addChild(_detailPanel, 6);

    _detailBg = LayerColor::create(
        Color4B(15, 15, 15, 220),
        BuildShopConfig::DETAIL_MIN_WIDTH,
        BuildShopConfig::DETAIL_MIN_HEIGHT
    );
    _detailBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    _detailBg->setIgnoreAnchorPointForPosition(false);
    _detailPanel->addChild(_detailBg, 0);

    auto border = DrawNode::create();
    border->setName("detailBorder");
    _detailPanel->addChild(border, 1);

    _detailLabel = createShopLabel("", 12);
    _detailLabel->setAnchorPoint(Vec2(0.0f, 1.0f));
    _detailLabel->setAlignment(TextHAlignment::LEFT);
    _detailLabel->setTextColor(Color4B::WHITE);
    _detailPanel->addChild(_detailLabel, 2);
}

// ===================================================
// ???y?????????
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
    auto nameLabel = createShopLabel(option.name, 10);
    nameLabel->setPosition(Vec2(0, -20));
    nameLabel->setColor(option.canBuild ? Color3B::WHITE : Color3B::GRAY);
    nameLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    itemNode->addChild(nameLabel, 2);
    // ������ʾ
    char costText[32];
    snprintf(costText, sizeof(costText), "%d", option.cost);
    auto costLabel = createShopLabel(costText, 9);
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
    auto sizeLabel = createShopLabel(sizeText, 9);
    sizeLabel->setPosition(Vec2(0, -40));
    sizeLabel->setColor(option.canBuild ? Color3B(210, 210, 210) : Color3B::GRAY);
    sizeLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    itemNode->addChild(sizeLabel, 2);

    // ������ɽ��죬����"����"��ǩ
    if (!option.canBuild) {
        auto lockedLabel = createShopLabel("Owned", 9);
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
    hoverListener->onMouseMove = [this, itemNode, bg, border, itemWidth, itemHeight, borderColor, bgColor, hoverState, canBuild, optionCopy](EventMouse* event) {
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
            Vec2 worldPos = parent->convertToWorldSpace(itemNode->getPosition());
            showDetailPanel(optionCopy, worldPos);
        }
        else if (!inside && *hoverState) {
            bg->setColor(Color3B(bgColor.r, bgColor.g, bgColor.b));
            border->clear();
            border->drawRect(Vec2(-itemWidth / 2, -itemHeight / 2),
                Vec2(itemWidth / 2, itemHeight / 2),
                borderColor);
            *hoverState = false;
            hideDetailPanel();
        }
        else if (inside && *hoverState) {
            Vec2 worldPos = parent->convertToWorldSpace(itemNode->getPosition());
            showDetailPanel(optionCopy, worldPos);
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
    closeBtn->setTitleFontName(kShopFont);
    closeBtn->setTitleFontSize(14);
    closeBtn->setTitleColor(Color3B::WHITE);
    closeBtn->setPosition(Vec2(
        BuildShopConfig::PANEL_SIZE.width / 2,
        BuildShopConfig::CLOSE_BUTTON_BOTTOM
    ));

    closeBtn->setPressedActionEnabled(true);
    closeBtn->setZoomScale(0.06f);
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
// ===================================================
// 构建详情文本
// ===================================================
std::string BuildShopPanel::buildDetailText(const BuildingOption& option) const {
    std::string text = StringUtils::format("%s\nType: %s\nCost: %dG  Size: %dx%d",
        option.name.c_str(),
        toCategoryName(option.category).c_str(),
        option.cost,
        option.gridWidth,
        option.gridHeight);

    auto* manager = BuildingManager::getInstance();
    manager->loadConfigs();
    int configId = option.configId > 0 ? option.configId : option.type;

    if (option.category == BuildingCategory::Defence) {
        const DefenceBuildingConfig* config = manager->getDefenceConfig(configId);
        if (config) {
            float hp = getFirstFloatValue(config->HP, 0.0f);
            float atk = getFirstFloatValue(config->ATK, 0.0f);
            float range = getFirstFloatValue(config->ATK_RANGE, 0.0f);
            float speed = getFirstFloatValue(config->ATK_SPEED, 0.0f);
            std::string target = buildTargetText(config->GROUND_ABLE, config->SKY_ABLE);
            text += StringUtils::format("\nHP: %.0f\nATK: %.0f  Range: %.0f\nRate: %.2fs  Target: %s",
                hp, atk, range, speed, target.c_str());
        }
    }
    else if (option.category == BuildingCategory::Production) {
        const ProductionBuildingConfig* config = manager->getProductionConfig(configId);
        if (config) {
            float hp = getFirstFloatValue(config->HP, 0.0f);
            int gold = getFirstIntValue(config->PRODUCE_GOLD, 0);
            int elixir = getFirstIntValue(config->PRODUCE_ELIXIR, 0);
            int goldCap = getFirstIntValue(config->STORAGE_GOLD_CAPACITY, 0);
            int elixirCap = getFirstIntValue(config->STORAGE_ELIXIR_CAPACITY, 0);
            text += StringUtils::format("\nHP: %.0f\nOutput: G+%d E+%d\nStorage: G+%d E+%d",
                hp, gold, elixir, goldCap, elixirCap);
        }
    }
    else if (option.category == BuildingCategory::Storage) {
        const StorageBuildingConfig* config = manager->getStorageConfig(configId);
        if (config) {
            float hp = getFirstFloatValue(config->HP, 0.0f);
            int gold = getFirstIntValue(config->ADD_STORAGE_GOLD_CAPACITY, 0);
            int elixir = getFirstIntValue(config->ADD_STORAGE_ELIXIR_CAPACITY, 0);
            text += StringUtils::format("\nHP: %.0f\nCapacity: G+%d E+%d",
                hp, gold, elixir);
        }
    }
    else if (option.category == BuildingCategory::Trap) {
        text += "\nTrigger: Contact\nEffect: Single-use damage";
    }

    return text;
}

// ===================================================
// 显示/隐藏详情提示
// ===================================================
void BuildShopPanel::showDetailPanel(const BuildingOption& option, const Vec2& worldPos) {
    if (!_detailPanel || !_detailBg || !_detailLabel || !_panel) {
        return;
    }

    std::string detailText = buildDetailText(option);
    _detailLabel->setString(detailText);
    _detailLabel->setWidth(BuildShopConfig::DETAIL_MIN_WIDTH - BuildShopConfig::DETAIL_PADDING * 2);

    Size textSize = _detailLabel->getContentSize();
    float width = std::max(BuildShopConfig::DETAIL_MIN_WIDTH, textSize.width + BuildShopConfig::DETAIL_PADDING * 2);
    float height = std::max(BuildShopConfig::DETAIL_MIN_HEIGHT, textSize.height + BuildShopConfig::DETAIL_PADDING * 2);
    _detailBg->setContentSize(Size(width, height));
    _detailLabel->setPosition(Vec2(-width * 0.5f + BuildShopConfig::DETAIL_PADDING,
        height * 0.5f - BuildShopConfig::DETAIL_PADDING));

    auto border = dynamic_cast<DrawNode*>(_detailPanel->getChildByName("detailBorder"));
    if (border) {
        border->clear();
        border->drawRect(Vec2(-width * 0.5f, -height * 0.5f),
            Vec2(width * 0.5f, height * 0.5f),
            Color4F(0.8f, 0.8f, 0.8f, 1.0f));
    }

    Vec2 localPos = _panel->convertToNodeSpace(worldPos);
    float offsetX = BuildShopConfig::GRID_ITEM_SIZE * 0.6f + width * 0.5f;
    Vec2 desired = localPos + Vec2(offsetX, 0.0f);

    float minX = width * 0.5f + 8.0f;
    float maxX = BuildShopConfig::PANEL_SIZE.width - width * 0.5f - 8.0f;
    float minY = height * 0.5f + 8.0f;
    float maxY = BuildShopConfig::PANEL_SIZE.height - height * 0.5f - 8.0f;

    if (desired.x > maxX) {
        desired.x = localPos.x - offsetX;
    }

    float clampedX = std::max(minX, std::min(maxX, desired.x));
    float clampedY = std::max(minY, std::min(maxY, desired.y));
    _detailPanel->setPosition(Vec2(clampedX, clampedY));
    _detailPanel->setVisible(true);
}

void BuildShopPanel::hideDetailPanel() {
    if (_detailPanel) {
        _detailPanel->setVisible(false);
    }
}

// ===================================================
// ??????
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
    hideDetailPanel();

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

