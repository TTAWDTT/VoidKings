// TrainPanel.cpp
// 兵种训练面板实现
// 黑白网格风格，显示兵种待机动画，含招募和升级按钮

#include "TrainPanel.h"
#include "Core/Core.h"
#include "Utils/AnimationUtils.h"
#include <algorithm>
#include <unordered_map>

namespace {
// 按钮按下缩放反馈（用于无贴图按钮）
void bindPressScale(Button* button, Node* target, const std::function<void()>& onClick) {
    if (!button || !target) {
        return;
    }

    const float originScale = target->getScale();
    button->setSwallowTouches(true);
    button->addTouchEventListener([target, onClick, originScale](Ref*, Widget::TouchEventType type) {
        switch (type) {
        case Widget::TouchEventType::BEGAN:
            target->setScale(originScale * 0.96f);
            break;
        case Widget::TouchEventType::CANCELED:
            target->setScale(originScale);
            break;
        case Widget::TouchEventType::ENDED:
            target->setScale(originScale);
            if (onClick) {
                onClick();
            }
            break;
        default:
            break;
        }
    });
}

struct ContentCenterInfo {
    bool valid = false;
    Vec2 center = Vec2::ZERO;
    Size size = Size::ZERO;
};

ContentCenterInfo getContentCenterInfo(const std::string& path) {
    static std::unordered_map<std::string, ContentCenterInfo> cache;
    auto it = cache.find(path);
    if (it != cache.end()) {
        return it->second;
    }

    ContentCenterInfo info;
    if (path.empty()) {
        cache.emplace(path, info);
        return info;
    }

    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(path);
    const std::string& loadPath = fullPath.empty() ? path : fullPath;

    Image image;
    if (!image.initWithImageFile(loadPath)) {
        cache.emplace(path, info);
        return info;
    }

    int width = image.getWidth();
    int height = image.getHeight();
    if (width <= 0 || height <= 0) {
        cache.emplace(path, info);
        return info;
    }

    unsigned char* data = image.getData();
    if (!data) {
        cache.emplace(path, info);
        return info;
    }

    const int bytesPerPixel = image.getBitPerPixel() / 8;
    if (bytesPerPixel < 3) {
        cache.emplace(path, info);
        return info;
    }

    double sumX = 0.0;
    double sumY = 0.0;
    double sumWeight = 0.0;

    const bool hasAlpha = bytesPerPixel >= 4;
    for (int y = 0; y < height; ++y) {
        int row = y * width * bytesPerPixel;
        for (int x = 0; x < width; ++x) {
            int idx = row + x * bytesPerPixel;
            float alpha = hasAlpha ? (data[idx + 3] / 255.0f) : 1.0f;
            if (alpha <= 0.0f) {
                continue;
            }
            sumX += static_cast<double>(x) * alpha;
            sumY += static_cast<double>(height - 1 - y) * alpha;
            sumWeight += alpha;
        }
    }

    if (sumWeight > 0.0) {
        info.valid = true;
        info.center = Vec2(static_cast<float>(sumX / sumWeight), static_cast<float>(sumY / sumWeight));
        info.size = Size(static_cast<float>(width), static_cast<float>(height));
    }

    cache.emplace(path, info);
    return info;
}
} // namespace

// ===================================================
// 创建与初始化
// ===================================================

TrainPanel* TrainPanel::create(
    const std::function<void(int)>& onTrainComplete,
    const std::function<void()>& onClose
) {
    TrainPanel* ret = new (std::nothrow) TrainPanel();
    if (ret && ret->init(onTrainComplete, onClose)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool TrainPanel::init(
    const std::function<void(int)>& onTrainComplete,
    const std::function<void()>& onClose
) {
    if (!Node::init()) {
        return false;
    }

    _onTrainComplete = onTrainComplete;
    _onClose = onClose;
    _isShowing = false;

    // 初始化可训练兵种列表（从配置读取）
    _availableUnits = UnitManager::getInstance()->getAllUnitIds();
    if (_availableUnits.empty()) {
        UnitManager::getInstance()->loadConfig("res/units_config.json");
        _availableUnits = UnitManager::getInstance()->getAllUnitIds();
    }
    std::sort(_availableUnits.begin(), _availableUnits.end());
    // 不过滤资源缺失的兵种，缺失时使用占位图保证列表完整

    // 初始化兵种等级信息
    initUnitLevels();

    // 初始化各个UI组件
    setupBackground();
    setupPanel();
    setupTitle();
    setupUnitCards();
    setupCloseButton();
    setupResourceDisplay();

    // 初始隐藏
    this->setVisible(false);

    CCLOG("[训练面板] 初始化完成");

    return true;
}

// ===================================================
// 初始化兵种等级信息
// ===================================================
void TrainPanel::initUnitLevels() {
    _unitLevels.clear();
    auto manager = UnitManager::getInstance();
    for (int unitId : _availableUnits) {
        UnitLevelInfo info;
        info.unitId = unitId;
        info.currentLevel = manager->getUnitLevel(unitId);
        info.count = manager->getUnitCount(unitId);
        _unitLevels[unitId] = info;
    }
}

// ===================================================
// 背景设置 - 半透明遮罩覆盖全屏
// ===================================================
void TrainPanel::setupBackground() {
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
void TrainPanel::setupPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 主面板 - 深灰色背景
    _panel = LayerColor::create(
        Color4B(30, 30, 30, 255),
        TrainPanelConfig::PANEL_SIZE.width,
        TrainPanelConfig::PANEL_SIZE.height
    );

    // 居中定位
    _panel->setAnchorPoint(Vec2(0.5f, 0.5f));
    _panel->setIgnoreAnchorPointForPosition(false);
    Vec2 panelCenter(
        origin.x + visibleSize.width * 0.5f,
        origin.y + visibleSize.height * 0.5f
    );
    _panel->setPosition(panelCenter);
    // 根据屏幕尺寸自适应缩放，保证完整显示不被遮挡
    const float margin = 16.0f;
    float maxWidth = visibleSize.width - margin * 2.0f;
    float maxHeight = visibleSize.height - margin * 2.0f;
    float scale = 1.0f;
    if (TrainPanelConfig::PANEL_SIZE.width > maxWidth || TrainPanelConfig::PANEL_SIZE.height > maxHeight) {
        scale = std::min(maxWidth / TrainPanelConfig::PANEL_SIZE.width,
            maxHeight / TrainPanelConfig::PANEL_SIZE.height);
    }
    _panel->setScale(scale);

    // 根据实际包围盒重新校准中心，避免缩放/锚点导致偏移
    Rect panelBounds = _panel->getBoundingBox();
    Vec2 boundsCenter = panelBounds.origin + Vec2(panelBounds.size.width * 0.5f, panelBounds.size.height * 0.5f);
    _panel->setPosition(_panel->getPosition() + (panelCenter - boundsCenter));

    // 添加边框
    auto border = DrawNode::create();
    border->drawRect(
        Vec2(0, 0),
        Vec2(TrainPanelConfig::PANEL_SIZE.width, TrainPanelConfig::PANEL_SIZE.height),
        Color4F::WHITE
    );
    _panel->addChild(border, 1);

    _contentRoot = Node::create();
    // 统一使用内边距，确保内容居中对齐
    float contentWidth = TrainPanelConfig::PANEL_SIZE.width - TrainPanelConfig::PANEL_PADDING * 2;
    float contentHeight = TrainPanelConfig::PANEL_SIZE.height - TrainPanelConfig::PANEL_PADDING * 2;
    _contentRoot->setContentSize(Size(contentWidth, contentHeight));
    _contentRoot->setAnchorPoint(Vec2(0.0f, 0.0f));
    _contentRoot->setIgnoreAnchorPointForPosition(false);
    _contentRoot->setPosition(Vec2(TrainPanelConfig::PANEL_PADDING, TrainPanelConfig::PANEL_PADDING));
    _panel->addChild(_contentRoot, 2);

    this->addChild(_panel, 1);
}

// ===================================================
// 标题设置
// ===================================================
void TrainPanel::setupTitle() {
    if (!_contentRoot) {
        return;
    }
    Size contentSize = _contentRoot->getContentSize();

    // 标题背景条
    auto titleBg = LayerColor::create(
        Color4B(50, 50, 50, 255),
        contentSize.width,
        TrainPanelConfig::TITLE_BAR_HEIGHT
    );
    float titleY = contentSize.height - TrainPanelConfig::TITLE_BAR_HEIGHT;
    titleBg->setPosition(Vec2(0.0f, titleY));
    _contentRoot->addChild(titleBg);

    // 标题文字
    _titleLabel = Label::createWithTTF(
        "TRAINING CENTER",
        "fonts/arial.ttf",
        TrainPanelConfig::TITLE_FONT_SIZE
    );
    if (!_titleLabel) {
        _titleLabel = Label::createWithSystemFont("TRAINING CENTER", "Arial", TrainPanelConfig::TITLE_FONT_SIZE);
    }
    _titleLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    _titleLabel->setPosition(
        contentSize.width / 2,
        titleY + TrainPanelConfig::TITLE_BAR_HEIGHT * 0.5f
    );
    _titleLabel->setColor(Color3B::WHITE);
    _contentRoot->addChild(_titleLabel, 2);
}

// ===================================================
// 兵种卡片区域设置
// ===================================================
void TrainPanel::setupUnitCards() {
    if (!_contentRoot) {
        return;
    }
    Size contentSize = _contentRoot->getContentSize();
    float topY = contentSize.height
        - TrainPanelConfig::TITLE_BAR_HEIGHT
        - TrainPanelConfig::CARD_AREA_TOP_GAP;
    float closeButtonTop = TrainPanelConfig::CLOSE_BUTTON_BOTTOM
        + TrainPanelConfig::CLOSE_BUTTON_HEIGHT * 0.5f;
    float bottomY = closeButtonTop + TrainPanelConfig::CARD_AREA_BOTTOM_GAP;
    float scrollHeight = topY - bottomY;
    if (scrollHeight < 0.0f) {
        scrollHeight = 0.0f;
    }
    float cardAreaWidth = contentSize.width;

    _unitCardArea = ScrollView::create();
    _unitCardArea->setDirection(ScrollView::Direction::VERTICAL);
    _unitCardArea->setBounceEnabled(true);
    _unitCardArea->setScrollBarEnabled(true);
    _unitCardArea->setContentSize(Size(cardAreaWidth, scrollHeight));
    _unitCardArea->setAnchorPoint(Vec2(0.0f, 0.0f));
    _unitCardArea->setPosition(Vec2(0.0f, bottomY));
    _contentRoot->addChild(_unitCardArea);

    refreshUnitCards();
}

// ===================================================
// 刷新兵种卡片显示
// ===================================================
void TrainPanel::refreshUnitCards() {
    if (!_unitCardArea) {
        return;
    }

    auto inner = _unitCardArea->getInnerContainer();
    if (!inner) {
        return;
    }

    initUnitLevels();

    inner->removeAllChildren();
    _unitCardNodes.clear();

    int totalUnits = static_cast<int>(_availableUnits.size());
    if (totalUnits == 0) {
        return;
    }

    float cardWidth = TrainPanelConfig::UNIT_CARD_WIDTH;
    float cardHeight = TrainPanelConfig::UNIT_CARD_HEIGHT;
    float spacing = TrainPanelConfig::UNIT_CARD_SPACING;
    float cardAreaWidth = _unitCardArea->getContentSize().width;
    // 根据可见宽度动态计算列数，避免卡片整体偏左
    int maxCols = TrainPanelConfig::GRID_COLS;
    int maxByWidth = static_cast<int>((cardAreaWidth + spacing) / (cardWidth + spacing));
    int columns = std::max(1, std::min(maxCols, std::min(totalUnits, maxByWidth)));

    int rows = (totalUnits + columns - 1) / columns;
    float gridHeight = rows * cardHeight + (rows - 1) * spacing;
    float scrollHeight = _unitCardArea->getContentSize().height;
    float innerHeight = std::max(gridHeight, scrollHeight);

    _unitCardArea->setInnerContainerSize(Size(cardAreaWidth, innerHeight));

    float totalWidth = columns * cardWidth + (columns - 1) * spacing;
    float startX = (cardAreaWidth - totalWidth) / 2 + cardWidth / 2;
    float verticalPadding = std::max(0.0f, (innerHeight - gridHeight) * 0.5f);
    float startY = innerHeight - verticalPadding - cardHeight / 2;

    int index = 0;
    bool hasCard = false;
    float minX = 0.0f;
    float maxX = 0.0f;
    float minY = 0.0f;
    float maxY = 0.0f;
    for (int unitId : _availableUnits) {
        int row = index / columns;
        int col = index % columns;
        auto card = createUnitCard(unitId);
        if (card) {
            float x = startX + col * (cardWidth + spacing);
            float y = startY - row * (cardHeight + spacing);
            card->setPosition(Vec2(x, y));
            inner->addChild(card);
            _unitCardNodes[unitId] = card;
            Rect bounds = card->getBoundingBox();
            float left = bounds.getMinX();
            float right = bounds.getMaxX();
            float bottom = bounds.getMinY();
            float top = bounds.getMaxY();
            if (!hasCard) {
                minX = left;
                maxX = right;
                minY = bottom;
                maxY = top;
                hasCard = true;
            }
            else {
                minX = std::min(minX, left);
                maxX = std::max(maxX, right);
                minY = std::min(minY, bottom);
                maxY = std::max(maxY, top);
            }
        }
        index++;
    }

    if (_selectedUnitId == -1 && !_availableUnits.empty()) {
        selectUnit(_availableUnits.front());
    }

    _unitCardArea->jumpToTop();

    if (hasCard) {
        Vec2 innerPos = inner->getPosition();
        Size viewSize = _unitCardArea->getContentSize();
        float gridCenterX = (minX + maxX) * 0.5f;
        float gridCenterY = (minY + maxY) * 0.5f;
        float viewCenterX = viewSize.width * 0.5f - innerPos.x;
        float viewCenterY = viewSize.height * 0.5f - innerPos.y;
        float gridWidth = maxX - minX;
        float gridHeight = maxY - minY;
        float offsetX = (gridWidth <= viewSize.width + 0.5f) ? (viewCenterX - gridCenterX) : 0.0f;
        float offsetY = (gridHeight <= viewSize.height + 0.5f) ? (viewCenterY - gridCenterY) : 0.0f;
        if (offsetX != 0.0f || offsetY != 0.0f) {
            // 根据可视区域中心做二次居中，修正整体偏移
            Vec2 offset(offsetX, offsetY);
            for (auto& pair : _unitCardNodes) {
                auto node = pair.second;
                if (node) {
                    node->setPosition(node->getPosition() + offset);
                }
            }
        }
    }
}

// ===================================================
// 创建兵种卡片（带动画和按钮）
// ===================================================
Node* TrainPanel::createUnitCard(int unitId) {
    auto cardNode = Node::create();

    // 获取兵种配置
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) {
        CCLOG("[训练面板] 未找到兵种配置: %d", unitId);
        return nullptr;
    }

    // 获取等级信息
    UnitLevelInfo& levelInfo = _unitLevels[unitId];

    // 计算尺寸
    float cardWidth = TrainPanelConfig::UNIT_CARD_WIDTH;
    float cardHeight = TrainPanelConfig::UNIT_CARD_HEIGHT;

    cardNode->setContentSize(Size(cardWidth, cardHeight));
    cardNode->setAnchorPoint(Vec2(0.5f, 0.5f));
    cardNode->setIgnoreAnchorPointForPosition(false);

    Vec2 cardCenter(cardWidth * 0.5f, cardHeight * 0.5f);
    auto cardRoot = Node::create();
    cardRoot->setPosition(cardCenter);
    cardRoot->setName("cardRoot");
    cardNode->addChild(cardRoot);

    // 卡片背景 - 黑白网格风格
    auto bg = LayerColor::create(Color4B(50, 50, 50, 255), cardWidth, cardHeight);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    cardRoot->addChild(bg);

    // 边框
    auto border = DrawNode::create();
    border->drawRect(
        Vec2(-cardWidth / 2, -cardHeight / 2),
        Vec2(cardWidth / 2, cardHeight / 2),
        Color4F::WHITE
    );
    cardRoot->addChild(border, 1);

    // 选中边框
    auto selectBorder = DrawNode::create();
    selectBorder->drawRect(
        Vec2(-cardWidth / 2, -cardHeight / 2),
        Vec2(cardWidth / 2, cardHeight / 2),
        Color4F(1.0f, 0.85f, 0.3f, 1.0f)
    );
    selectBorder->setName("selectBorder");
    selectBorder->setVisible(false);
    cardRoot->addChild(selectBorder, 2);

    // 待机动画精灵（调整位置适应紧凑布局）
    auto animSprite = createIdleAnimationSprite(unitId, true);
    if (animSprite) {
        animSprite->setPosition(Vec2(0, 30));
        animSprite->setName("animSprite");
        cardRoot->addChild(animSprite, 2);
    }

    // 兵种名称（缩小字体）
    auto nameLabel = Label::createWithTTF(config->name, "fonts/arial.ttf", 13);
    if (!nameLabel) {
        nameLabel = Label::createWithSystemFont(config->name, "Arial", 13);
    }
    nameLabel->setPosition(Vec2(0, -10));
    nameLabel->setColor(Color3B::WHITE);
    cardRoot->addChild(nameLabel, 2);

    // 等级显示（缩小字体）
    char levelText[32];
    bool maxLevel = isMaxLevel(unitId);
    if (maxLevel) {
        snprintf(levelText, sizeof(levelText), "Lv.MAX");
    }
    else {
        snprintf(levelText, sizeof(levelText), "Lv.%d", levelInfo.currentLevel + 1);
    }
    auto levelLabel = Label::createWithTTF(levelText, "fonts/arial.ttf", 10);
    if (!levelLabel) {
        levelLabel = Label::createWithSystemFont(levelText, "Arial", 10);
    }
    levelLabel->setPosition(Vec2(0, -23));
    levelLabel->setColor(maxLevel ? Color3B::YELLOW : Color3B::WHITE);
    cardRoot->addChild(levelLabel, 2);

    // 数量显示（缩小字体）
    char countText[32];
    snprintf(countText, sizeof(countText), "x%d", levelInfo.count);
    auto countLabel = Label::createWithTTF(countText, "fonts/arial.ttf", 10);
    if (!countLabel) {
        countLabel = Label::createWithSystemFont(countText, "Arial", 10);
    }
    countLabel->setPosition(Vec2(0, -35));
    countLabel->setColor(Color3B::GREEN);
    cardRoot->addChild(countLabel, 2);

    // 按钮区域 - 招募和升级（调整位置）
    float btnY = -cardHeight / 2 + 22;
    float btnSpacing = 6.0f;
    float btnWidth = TrainPanelConfig::BUTTON_WIDTH;
    float btnHeight = TrainPanelConfig::BUTTON_HEIGHT;

    // 招募按钮（消耗金币）
    auto recruitNode = Node::create();
    recruitNode->setPosition(Vec2(-btnWidth / 2 - btnSpacing / 2, btnY));

    auto recruitBg = LayerColor::create(Color4B(40, 80, 40, 255), btnWidth, btnHeight);
    recruitBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    recruitBg->setIgnoreAnchorPointForPosition(false);
    recruitNode->addChild(recruitBg);

    auto recruitBorder = DrawNode::create();
    recruitBorder->drawRect(Vec2(-btnWidth / 2, -btnHeight / 2), Vec2(btnWidth / 2, btnHeight / 2), Color4F::WHITE);
    recruitNode->addChild(recruitBorder, 1);

    // 使用配置常量替代魔法数字（缩小字体）
    int recruitCost = config->COST_COIN > 0 ? config->COST_COIN : TrainPanelConfig::DEFAULT_RECRUIT_COST;
    char recruitText[32];
    snprintf(recruitText, sizeof(recruitText), "%dG", recruitCost);
    auto recruitLabel = Label::createWithTTF(recruitText, "fonts/arial.ttf", 9);
    if (!recruitLabel) {
        recruitLabel = Label::createWithSystemFont(recruitText, "Arial", 9);
    }
    recruitLabel->setColor(Color3B::WHITE);
    recruitNode->addChild(recruitLabel, 2);

    auto recruitBtn = Button::create();
    recruitBtn->setScale9Enabled(true);
    recruitBtn->setContentSize(Size(btnWidth, btnHeight));
    bindPressScale(recruitBtn, recruitNode, [this, unitId]() {
        this->recruitUnit(unitId);
    });
    recruitNode->addChild(recruitBtn, 10);

    cardRoot->addChild(recruitNode, 3);

    // 升级按钮（消耗钻石）
    auto upgradeNode = Node::create();
    upgradeNode->setPosition(Vec2(btnWidth / 2 + btnSpacing / 2, btnY));

    Color4B upgradeBgColor = maxLevel ? Color4B(60, 60, 60, 255) : Color4B(80, 40, 80, 255);
    auto upgradeBg = LayerColor::create(upgradeBgColor, btnWidth, btnHeight);
    upgradeBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    upgradeBg->setIgnoreAnchorPointForPosition(false);
    upgradeNode->addChild(upgradeBg);

    auto upgradeBorder = DrawNode::create();
    Color4F upgradeBorderColor = maxLevel ? Color4F(0.4f, 0.4f, 0.4f, 1.0f) : Color4F::WHITE;
    upgradeBorder->drawRect(Vec2(-btnWidth / 2, -btnHeight / 2), Vec2(btnWidth / 2, btnHeight / 2), upgradeBorderColor);
    upgradeNode->addChild(upgradeBorder, 1);

    // 升级按钮文字（缩小字体）
    std::string upgradeText;
    if (maxLevel) {
        upgradeText = "MAX";
    }
    else {
        char buf[32];
        snprintf(buf, sizeof(buf), "%dD", 10 * (levelInfo.currentLevel + 1));
        upgradeText = buf;
    }
    auto upgradeLabel = Label::createWithTTF(upgradeText, "fonts/arial.ttf", 9);
    if (!upgradeLabel) {
        upgradeLabel = Label::createWithSystemFont(upgradeText, "Arial", 9);
    }
    upgradeLabel->setColor(maxLevel ? Color3B::GRAY : Color3B::WHITE);
    upgradeNode->addChild(upgradeLabel, 2);

    if (!maxLevel) {
        auto upgradeBtn = Button::create();
        upgradeBtn->setScale9Enabled(true);
        upgradeBtn->setContentSize(Size(btnWidth, btnHeight));
        bindPressScale(upgradeBtn, upgradeNode, [this, unitId]() {
            this->upgradeUnit(unitId);
        });
        upgradeNode->addChild(upgradeBtn, 10);
    }

    cardRoot->addChild(upgradeNode, 3);

    // 选中回调
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    listener->onTouchBegan = [cardNode](Touch* touch, Event* event) {
        auto parent = cardNode->getParent();
        if (!parent) {
            return false;
        }
        Vec2 localPos = parent->convertToNodeSpace(touch->getLocation());
        return cardNode->getBoundingBox().containsPoint(localPos);
    };
    listener->onTouchEnded = [this, unitId](Touch* touch, Event* event) {
        this->selectUnit(unitId);
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, cardNode);

    return cardNode;
}

// ===================================================
// 创建待机动画精灵
// ===================================================
Sprite* TrainPanel::createIdleAnimationSprite(int unitId, bool forceAnimate) {
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) return nullptr;

    // 根据配置的spriteFrameName构建动画路径
    // 动画帧格式: {folder}/{name}_{anim_idle}_{frame}.png
    std::string baseName;
    std::string folderPath;
    std::string spritePath = config->spriteFrameName;
    if (spritePath.size() > 4 && spritePath.substr(spritePath.size() - 4) == ".png") {
        spritePath = spritePath.substr(0, spritePath.size() - 4);
    }

    auto slashPos = spritePath.find_last_of('/');
    if (slashPos != std::string::npos) {
        folderPath = spritePath.substr(0, slashPos);
        baseName = spritePath.substr(slashPos + 1);
    }
    else {
        switch (unitId) {
        case 101:
            folderPath = "unit/MiniSpearMan_output";
            baseName = "spearman";
            break;
        case 102:
            folderPath = "unit/MiniSwordMan_output";
            baseName = "swordman";
            break;
        case 103:
            folderPath = "unit/MiniArcherMan_output";
            baseName = "archer";
            break;
        case 104:
            folderPath = "unit/MiniMage_output";
            baseName = "mage";
            break;
        case 105:
            folderPath = "unit/MiniArchMage_output";
            baseName = "archmage";
            break;
        default:
            break;
        }
    }

    if (folderPath.empty() || baseName.empty()) {
        auto placeholder = Sprite::create();
        auto marker = DrawNode::create();
        marker->drawSolidRect(Vec2(-30, -30), Vec2(30, 30), Color4F(0.3f, 0.3f, 0.3f, 1.0f));
        placeholder->addChild(marker);
        return placeholder;
    }

    // 使用配置中的待机动画名，优先保证与配置一致
    std::string animKey = config->anim_idle.empty() ? "idle" : config->anim_idle;

    // 首先尝试加载静态图片
    std::string staticPath = folderPath + "/" + baseName + ".png";
    std::string referencePath;
    auto sprite = Sprite::create(staticPath);

    if (!sprite) {
        // 如果静态图片不存在，尝试加载第一帧idle动画
        std::string idlePath = folderPath + "/" + baseName + "_" + animKey + "_1.png";
        sprite = Sprite::create(idlePath);
        if (sprite) {
            referencePath = idlePath;
        }
    }
    else {
        referencePath = staticPath;
    }

    if (!sprite) {
        // 如果都失败，创建占位精灵
        sprite = Sprite::create();
        auto placeholder = DrawNode::create();
        placeholder->drawSolidRect(Vec2(-30, -30), Vec2(30, 30), Color4F(0.3f, 0.3f, 0.3f, 1.0f));
        sprite->addChild(placeholder);
        return sprite;  // 占位精灵不需要动画
    }

    // 缩放到合适尺寸
    float targetSize = TrainPanelConfig::ANIM_SIZE;
    Size contentSize = sprite->getContentSize();
    if (contentSize.width > 0 && contentSize.height > 0) {
        float scale = targetSize / std::max(contentSize.width, contentSize.height);
        sprite->setScale(scale);
    }

    ContentCenterInfo referenceInfo = getContentCenterInfo(referencePath);
    bool hasReferenceCenter = referenceInfo.valid;
    Vec2 referenceFromGeo = Vec2::ZERO;
    if (hasReferenceCenter) {
        referenceFromGeo = referenceInfo.center
            - Vec2(referenceInfo.size.width * 0.5f, referenceInfo.size.height * 0.5f);
    }

    // 尝试创建待机动画 - 使用Sprite::create加载每帧获取正确尺寸
    auto buildFrames = [&](const std::string& key, int frameCount) {
        Vector<SpriteFrame*> frames;
        for (int i = 1; i <= frameCount; ++i) {
            char framePath[256];
            snprintf(framePath, sizeof(framePath), "%s/%s_%s_%d.png",
                folderPath.c_str(), baseName.c_str(), key.c_str(), i);
            auto texture = Director::getInstance()->getTextureCache()->addImage(framePath);
            if (texture) {
                Size texSize = texture->getContentSize();
                SpriteFrame* frame = nullptr;
                if (hasReferenceCenter) {
                    ContentCenterInfo frameInfo = getContentCenterInfo(framePath);
                    if (frameInfo.valid) {
                        Vec2 frameFromGeo = frameInfo.center
                            - Vec2(frameInfo.size.width * 0.5f, frameInfo.size.height * 0.5f);
                        Vec2 offset = referenceFromGeo - frameFromGeo;
                        frame = SpriteFrame::createWithTexture(
                            texture,
                            Rect(0, 0, texSize.width, texSize.height),
                            false,
                            offset,
                            frameInfo.size
                        );
                    }
                }
                if (!frame) {
                    frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texSize.width, texSize.height));
                }
                if (frame) {
                    frames.pushBack(frame);
                }
            }
        }
        return frames;
    };

    Vector<SpriteFrame*> frames = buildFrames(animKey, config->anim_idle_frames);
    if (forceAnimate && frames.size() <= 1) {
        frames = buildFrames(config->anim_walk, config->anim_walk_frames);
        if (frames.size() > 1) {
            auto animation = Animation::createWithSpriteFrames(frames, config->anim_walk_delay);
            auto animate = Animate::create(animation);
            sprite->runAction(RepeatForever::create(animate));
        }
    }
    else if (frames.size() > 1) {
        auto animation = Animation::createWithSpriteFrames(frames, config->anim_idle_delay);
        auto animate = Animate::create(animation);
        sprite->runAction(RepeatForever::create(animate));
    }

    return sprite;
}

// ===================================================
// 选中逻辑
// ===================================================
void TrainPanel::selectUnit(int unitId) {
    if (_selectedUnitId == unitId) {
        return;
    }
    _selectedUnitId = unitId;

    for (const auto& pair : _unitCardNodes) {
        auto node = pair.second;
        if (!node) continue;
        Node* selectBorder = node->getChildByName("selectBorder");
        if (!selectBorder) {
            auto cardRoot = node->getChildByName("cardRoot");
            if (cardRoot) {
                selectBorder = cardRoot->getChildByName("selectBorder");
            }
        }
        if (selectBorder) {
            selectBorder->setVisible(pair.first == _selectedUnitId);
        }
    }

}

// ===================================================
// 招募兵种（消耗金币）
// ===================================================
void TrainPanel::recruitUnit(int unitId) {
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) return;

    // 使用金币招募（如果COST_COIN为0，使用默认值）
    int cost = config->COST_COIN > 0 ? config->COST_COIN : TrainPanelConfig::DEFAULT_RECRUIT_COST;

    int coin = Core::getInstance()->getResource(ResourceType::COIN);
    if (coin < cost) {
        CCLOG("[训练面板] 金币不足，无法招募: %s", config->name.c_str());
        return;
    }

    // 扣除金币
    Core::getInstance()->consumeResource(ResourceType::COIN, cost);

    // 增加兵种数量
    UnitManager::getInstance()->addTrainedUnit(unitId, 1);

    CCLOG("[训练面板] 招募成功: %s, 当前数量: %d",
        config->name.c_str(), UnitManager::getInstance()->getUnitCount(unitId));

    // 触发回调
    if (_onTrainComplete) {
        _onTrainComplete(unitId);
    }

    // 刷新显示
    refreshUnitCards();
    updateResourceDisplay();
}

// ===================================================
// 升级兵种（消耗钻石）
// ===================================================
void TrainPanel::upgradeUnit(int unitId) {
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) return;

    // 检查是否已满级
    if (isMaxLevel(unitId)) {
        CCLOG("[训练面板] 兵种已满级，无法升级: %s", config->name.c_str());
        return;
    }

    // 计算升级费用（钻石）
    int currentLevel = UnitManager::getInstance()->getUnitLevel(unitId);
    int cost = 10 * (currentLevel + 1);

    int diamond = Core::getInstance()->getResource(ResourceType::DIAMOND);
    if (diamond < cost) {
        CCLOG("[训练面板] 钻石不足，无法升级: %s", config->name.c_str());
        return;
    }

    // 扣除钻石
    Core::getInstance()->consumeResource(ResourceType::DIAMOND, cost);

    // 升级
    UnitManager::getInstance()->setUnitLevel(unitId, currentLevel + 1);

    CCLOG("[训练面板] 升级成功: %s, 当前等级: %d",
        config->name.c_str(), UnitManager::getInstance()->getUnitLevel(unitId) + 1);

    // 刷新显示
    refreshUnitCards();
    updateResourceDisplay();
}

// ===================================================
// 检查兵种是否已满级
// ===================================================
bool TrainPanel::isMaxLevel(int unitId) {
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) return true;

    int currentLevel = UnitManager::getInstance()->getUnitLevel(unitId);
    return currentLevel >= config->MAXLEVEL;
}

// ===================================================
// 关闭按钮设置（缩小尺寸）
// ===================================================
void TrainPanel::setupCloseButton() {
    if (!_contentRoot) {
        return;
    }
    float btnWidth = TrainPanelConfig::CLOSE_BUTTON_WIDTH;
    float btnHeight = TrainPanelConfig::CLOSE_BUTTON_HEIGHT;

    auto closeNode = Node::create();
    Size contentSize = _contentRoot->getContentSize();
    closeNode->setPosition(Vec2(
        contentSize.width / 2,
        TrainPanelConfig::CLOSE_BUTTON_BOTTOM
    ));

    auto bg = LayerColor::create(Color4B(60, 60, 60, 255), btnWidth, btnHeight);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    closeNode->addChild(bg);

    auto border = DrawNode::create();
    border->drawRect(Vec2(-btnWidth / 2, -btnHeight / 2), Vec2(btnWidth / 2, btnHeight / 2), Color4F::WHITE);
    closeNode->addChild(border, 1);

    auto label = Label::createWithTTF("CLOSE", "fonts/arial.ttf", 13);
    if (!label) {
        label = Label::createWithSystemFont("CLOSE", "Arial", 13);
    }
    label->setColor(Color3B::WHITE);
    closeNode->addChild(label, 2);

    auto closeBtn = Button::create();
    closeBtn->setScale9Enabled(true);
    closeBtn->setContentSize(Size(btnWidth, btnHeight));
    bindPressScale(closeBtn, closeNode, [this]() {
        this->hide();
        if (_onClose) {
            _onClose();
        }
    });
    closeNode->addChild(closeBtn, 10);

    _contentRoot->addChild(closeNode, 5);
}

// ===================================================
// 资源显示设置（缩小字体）
// ===================================================
void TrainPanel::setupResourceDisplay() {
    if (!_contentRoot) {
        return;
    }
    _resourceLabel = Label::createWithTTF(
        "",
        "fonts/arial.ttf",
        10
    );
    if (!_resourceLabel) {
        _resourceLabel = Label::createWithSystemFont("", "Arial", 11);
    }
    _resourceLabel->setAnchorPoint(Vec2(1.0f, 0.5f));
    Size contentSize = _contentRoot->getContentSize();
    float titleCenterY = contentSize.height - TrainPanelConfig::TITLE_BAR_HEIGHT * 0.5f;
    _resourceLabel->setPosition(Vec2(
        contentSize.width,
        titleCenterY
    ));
    _resourceLabel->setColor(Color3B::YELLOW);
    _contentRoot->addChild(_resourceLabel, 3);

    updateResourceDisplay();
}

// ===================================================
// 显示面板
// ===================================================
void TrainPanel::show() {
    this->setVisible(true);
    _isShowing = true;
    updateResourceDisplay();
    refreshUnitCards();
    if (_unitCardArea) {
        _unitCardArea->jumpToTop();
    }
    CCLOG("[训练面板] 显示面板");
}

// ===================================================
// 隐藏面板
// ===================================================
void TrainPanel::hide() {
    this->setVisible(false);
    _isShowing = false;
    CCLOG("[训练面板] 隐藏面板");
}

// ===================================================
// 更新资源显示
// ===================================================
void TrainPanel::updateResourceDisplay() {
    int coin = Core::getInstance()->getResource(ResourceType::COIN);
    int diamond = Core::getInstance()->getResource(ResourceType::DIAMOND);

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Gold: %d  Diamond: %d", coin, diamond);
    if (_resourceLabel) {
        _resourceLabel->setString(buffer);
    }
}
