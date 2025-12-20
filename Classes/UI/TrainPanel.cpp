// TrainPanel.cpp
// 兵种训练面板实现
// 黑白网格风格，显示兵种待机动画，含招募和升级按钮

#include "TrainPanel.h"
#include "Core/Core.h"
#include <algorithm>

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
    
    // 初始化可训练兵种列表
    _availableUnits = {101, 102, 103};  // Goblin, Barbarian, Archer
    
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
    
    // 启用update
    this->scheduleUpdate();
    
    CCLOG("[训练面板] 初始化完成");
    
    return true;
}

// ===================================================
// 初始化兵种等级信息
// ===================================================
void TrainPanel::initUnitLevels() {
    for (int unitId : _availableUnits) {
        UnitLevelInfo info;
        info.unitId = unitId;
        info.currentLevel = 0;  // 从0级开始
        info.count = 0;
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
    float panelX = origin.x + (visibleSize.width - TrainPanelConfig::PANEL_SIZE.width) / 2;
    float panelY = origin.y + (visibleSize.height - TrainPanelConfig::PANEL_SIZE.height) / 2;
    _panel->setPosition(panelX, panelY);
    
    // 添加边框
    auto border = DrawNode::create();
    border->drawRect(
        Vec2(0, 0),
        Vec2(TrainPanelConfig::PANEL_SIZE.width, TrainPanelConfig::PANEL_SIZE.height),
        Color4F::WHITE
    );
    _panel->addChild(border, 1);
    
    this->addChild(_panel, 1);
}

// ===================================================
// 标题设置
// ===================================================
void TrainPanel::setupTitle() {
    // 标题背景条
    auto titleBg = LayerColor::create(
        Color4B(50, 50, 50, 255),
        TrainPanelConfig::PANEL_SIZE.width,
        50.0f
    );
    titleBg->setPosition(Vec2(0, TrainPanelConfig::PANEL_SIZE.height - 50));
    _panel->addChild(titleBg);
    
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
        TrainPanelConfig::PANEL_SIZE.width / 2,
        TrainPanelConfig::PANEL_SIZE.height - 25
    );
    _titleLabel->setColor(Color3B::WHITE);
    _panel->addChild(_titleLabel, 2);
}

// ===================================================
// 兵种卡片区域设置
// ===================================================
void TrainPanel::setupUnitCards() {
    _unitCardArea = Node::create();
    _unitCardArea->setPosition(Vec2(0, 0));
    _panel->addChild(_unitCardArea);
    
    refreshUnitCards();
}

// ===================================================
// 刷新兵种卡片显示
// ===================================================
void TrainPanel::refreshUnitCards() {
    _unitCardArea->removeAllChildren();
    
    int row = 0;
    int col = 0;
    
    for (int unitId : _availableUnits) {
        auto card = createUnitCard(unitId, row, col);
        if (card) {
            _unitCardArea->addChild(card);
        }
        
        col++;
        if (col >= TrainPanelConfig::GRID_COLS) {
            col = 0;
            row++;
        }
    }
}

// ===================================================
// 创建兵种卡片（带动画和按钮）
// ===================================================
Node* TrainPanel::createUnitCard(int unitId, int row, int col) {
    auto cardNode = Node::create();
    
    // 获取兵种配置
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) {
        CCLOG("[训练面板] 未找到兵种配置: %d", unitId);
        return nullptr;
    }
    
    // 获取等级信息
    UnitLevelInfo& levelInfo = _unitLevels[unitId];
    
    // 计算位置
    float cardWidth = TrainPanelConfig::UNIT_CARD_WIDTH;
    float cardHeight = TrainPanelConfig::UNIT_CARD_HEIGHT;
    float spacing = TrainPanelConfig::UNIT_CARD_SPACING;
    
    // 计算起始位置使网格居中
    float totalWidth = TrainPanelConfig::GRID_COLS * cardWidth + (TrainPanelConfig::GRID_COLS - 1) * spacing;
    float startX = (TrainPanelConfig::PANEL_SIZE.width - totalWidth) / 2 + cardWidth / 2;
    float startY = TrainPanelConfig::PANEL_SIZE.height - TrainPanelConfig::UNIT_AREA_TOP_OFFSET - cardHeight / 2;
    
    float x = startX + col * (cardWidth + spacing);
    float y = startY - row * (cardHeight + spacing);
    
    cardNode->setPosition(Vec2(x, y));
    
    // 卡片背景 - 黑白网格风格
    auto bg = LayerColor::create(Color4B(50, 50, 50, 255), cardWidth, cardHeight);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    cardNode->addChild(bg);
    
    // 边框
    auto border = DrawNode::create();
    border->drawRect(
        Vec2(-cardWidth / 2, -cardHeight / 2),
        Vec2(cardWidth / 2, cardHeight / 2),
        Color4F::WHITE
    );
    cardNode->addChild(border, 1);
    
    // 待机动画精灵
    auto animSprite = createIdleAnimationSprite(unitId);
    if (animSprite) {
        animSprite->setPosition(Vec2(0, 40));
        cardNode->addChild(animSprite, 2);
    }
    
    // 兵种名称
    auto nameLabel = Label::createWithTTF(config->name, "fonts/arial.ttf", 16);
    if (!nameLabel) {
        nameLabel = Label::createWithSystemFont(config->name, "Arial", 16);
    }
    nameLabel->setPosition(Vec2(0, -10));
    nameLabel->setColor(Color3B::WHITE);
    cardNode->addChild(nameLabel, 2);
    
    // 等级显示
    char levelText[32];
    bool maxLevel = isMaxLevel(unitId);
    if (maxLevel) {
        snprintf(levelText, sizeof(levelText), "Lv.MAX");
    } else {
        snprintf(levelText, sizeof(levelText), "Lv.%d", levelInfo.currentLevel + 1);
    }
    auto levelLabel = Label::createWithTTF(levelText, "fonts/arial.ttf", 12);
    if (!levelLabel) {
        levelLabel = Label::createWithSystemFont(levelText, "Arial", 12);
    }
    levelLabel->setPosition(Vec2(0, -28));
    levelLabel->setColor(maxLevel ? Color3B::YELLOW : Color3B::WHITE);
    cardNode->addChild(levelLabel, 2);
    
    // 数量显示
    char countText[32];
    snprintf(countText, sizeof(countText), "Count: %d", levelInfo.count);
    auto countLabel = Label::createWithTTF(countText, "fonts/arial.ttf", 11);
    if (!countLabel) {
        countLabel = Label::createWithSystemFont(countText, "Arial", 11);
    }
    countLabel->setPosition(Vec2(0, -43));
    countLabel->setColor(Color3B::GREEN);
    cardNode->addChild(countLabel, 2);
    
    // 按钮区域 - 招募和升级
    float btnY = -cardHeight / 2 + 30;
    float btnSpacing = 10.0f;
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
    
    // 使用配置常量替代魔法数字
    int recruitCost = config->COST_COIN > 0 ? config->COST_COIN : TrainPanelConfig::DEFAULT_RECRUIT_COST;
    char recruitText[32];
    snprintf(recruitText, sizeof(recruitText), "Recruit\n%dG", recruitCost);
    auto recruitLabel = Label::createWithTTF(recruitText, "fonts/arial.ttf", 9);
    if (!recruitLabel) {
        recruitLabel = Label::createWithSystemFont(recruitText, "Arial", 9);
    }
    recruitLabel->setColor(Color3B::WHITE);
    recruitNode->addChild(recruitLabel, 2);
    
    auto recruitBtn = Button::create();
    recruitBtn->setContentSize(Size(btnWidth, btnHeight));
    recruitBtn->addClickEventListener([this, unitId](Ref* sender) {
        this->recruitUnit(unitId);
    });
    recruitNode->addChild(recruitBtn, 10);
    
    cardNode->addChild(recruitNode, 3);
    
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
    
    std::string upgradeText;
    if (maxLevel) {
        upgradeText = "MAX";
    } else {
        char buf[32];
        snprintf(buf, sizeof(buf), "Upgrade\n%dD", 10 * (levelInfo.currentLevel + 1));
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
        upgradeBtn->setContentSize(Size(btnWidth, btnHeight));
        upgradeBtn->addClickEventListener([this, unitId](Ref* sender) {
            this->upgradeUnit(unitId);
        });
        upgradeNode->addChild(upgradeBtn, 10);
    }
    
    cardNode->addChild(upgradeNode, 3);
    
    return cardNode;
}

// ===================================================
// 创建待机动画精灵
// ===================================================
Sprite* TrainPanel::createIdleAnimationSprite(int unitId) {
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) return nullptr;
    
    // 根据配置的spriteFrameName构建动画路径
    // 动画帧格式: unit/{folder}/{name}_idle_{frame}.png
    std::string baseName = config->spriteFrameName;
    std::string folderName;
    
    // 根据兵种ID确定文件夹
    if (unitId == 101) {
        folderName = "MiniSpearMan_output";
        baseName = "spearman";
    } else if (unitId == 102) {
        folderName = "MiniSwordMan_output";
        baseName = "swordman";
    } else if (unitId == 103) {
        folderName = "MiniArcherMan_output";
        baseName = "archer";
    }
    
    // 首先尝试加载静态图片
    std::string staticPath = "unit/" + folderName + "/" + baseName + ".png";
    auto sprite = Sprite::create(staticPath);
    
    if (!sprite) {
        // 如果静态图片不存在，尝试加载第一帧idle动画
        std::string idlePath = "unit/" + folderName + "/" + baseName + "_idle_1.png";
        sprite = Sprite::create(idlePath);
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
    
    // 尝试创建待机动画 - 使用Sprite::create加载每帧获取正确尺寸
    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= config->anim_idle_frames; ++i) {
        char framePath[256];
        snprintf(framePath, sizeof(framePath), "unit/%s/%s_idle_%d.png", 
                 folderName.c_str(), baseName.c_str(), i);
        
        // 使用Texture2D获取正确的纹理尺寸
        auto texture = Director::getInstance()->getTextureCache()->addImage(framePath);
        if (texture) {
            Size texSize = texture->getContentSize();
            auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texSize.width, texSize.height));
            if (frame) {
                frames.pushBack(frame);
            }
        }
    }
    
    if (frames.size() > 0) {
        auto animation = Animation::createWithSpriteFrames(frames, config->anim_idle_delay);
        auto animate = Animate::create(animation);
        sprite->runAction(RepeatForever::create(animate));
    }
    
    return sprite;
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
    _unitLevels[unitId].count++;
    _trainedUnits[unitId]++;
    
    CCLOG("[训练面板] 招募成功: %s, 当前数量: %d", config->name.c_str(), _unitLevels[unitId].count);
    
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
    int currentLevel = _unitLevels[unitId].currentLevel;
    int cost = 10 * (currentLevel + 1);
    
    int diamond = Core::getInstance()->getResource(ResourceType::DIAMOND);
    if (diamond < cost) {
        CCLOG("[训练面板] 钻石不足，无法升级: %s", config->name.c_str());
        return;
    }
    
    // 扣除钻石
    Core::getInstance()->consumeResource(ResourceType::DIAMOND, cost);
    
    // 升级
    _unitLevels[unitId].currentLevel++;
    
    CCLOG("[训练面板] 升级成功: %s, 当前等级: %d", config->name.c_str(), _unitLevels[unitId].currentLevel + 1);
    
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
    
    int currentLevel = _unitLevels[unitId].currentLevel;
    return currentLevel >= config->MAXLEVEL;
}

// ===================================================
// 检查是否有足够的金币招募
// ===================================================
bool TrainPanel::canAffordRecruit(int unitId) {
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) return false;
    
    int cost = config->COST_COIN > 0 ? config->COST_COIN : 50;
    int coin = Core::getInstance()->getResource(ResourceType::COIN);
    return coin >= cost;
}

// ===================================================
// 检查是否有足够的钻石升级
// ===================================================
bool TrainPanel::canAffordUpgrade(int unitId) {
    if (isMaxLevel(unitId)) return false;
    
    int currentLevel = _unitLevels[unitId].currentLevel;
    int cost = 10 * (currentLevel + 1);
    int diamond = Core::getInstance()->getResource(ResourceType::DIAMOND);
    return diamond >= cost;
}

// ===================================================
// 关闭按钮设置
// ===================================================
void TrainPanel::setupCloseButton() {
    float btnWidth = 120.0f;
    float btnHeight = 40.0f;
    
    auto closeNode = Node::create();
    closeNode->setPosition(Vec2(
        TrainPanelConfig::PANEL_SIZE.width / 2,
        TrainPanelConfig::CLOSE_BUTTON_BOTTOM
    ));
    
    auto bg = LayerColor::create(Color4B(60, 60, 60, 255), btnWidth, btnHeight);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    closeNode->addChild(bg);
    
    auto border = DrawNode::create();
    border->drawRect(Vec2(-btnWidth / 2, -btnHeight / 2), Vec2(btnWidth / 2, btnHeight / 2), Color4F::WHITE);
    closeNode->addChild(border, 1);
    
    auto label = Label::createWithTTF("CLOSE", "fonts/arial.ttf", 18);
    if (!label) {
        label = Label::createWithSystemFont("CLOSE", "Arial", 18);
    }
    label->setColor(Color3B::WHITE);
    closeNode->addChild(label, 2);
    
    auto closeBtn = Button::create();
    closeBtn->setContentSize(Size(btnWidth, btnHeight));
    closeBtn->addClickEventListener([this](Ref* sender) {
        this->hide();
        if (_onClose) {
            _onClose();
        }
    });
    closeNode->addChild(closeBtn, 10);
    
    _panel->addChild(closeNode, 5);
}

// ===================================================
// 资源显示设置
// ===================================================
void TrainPanel::setupResourceDisplay() {
    _resourceLabel = Label::createWithTTF(
        "",
        "fonts/arial.ttf",
        14
    );
    if (!_resourceLabel) {
        _resourceLabel = Label::createWithSystemFont("", "Arial", 14);
    }
    _resourceLabel->setAnchorPoint(Vec2(1.0f, 1.0f));
    _resourceLabel->setPosition(Vec2(
        TrainPanelConfig::PANEL_SIZE.width - 20,
        TrainPanelConfig::PANEL_SIZE.height - 55
    ));
    _resourceLabel->setColor(Color3B::YELLOW);
    _panel->addChild(_resourceLabel, 3);
    
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
// 每帧更新
// ===================================================
void TrainPanel::update(float dt) {
    // 训练面板不再使用队列模式，而是直接招募
    // 保留此方法以兼容接口
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
