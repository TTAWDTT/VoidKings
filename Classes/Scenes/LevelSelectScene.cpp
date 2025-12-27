// LevelSelectScene.cpp
// 关卡选择场景实现
// 采用黑白网格风格设计

#include "LevelSelectScene.h"
#include "BaseScene.h"
#include "BattleScene.h"
#include "Soldier/UnitManager.h"
#include "Soldier/UnitData.h"
#include "Utils/AudioManager.h"
#include "Utils/AnimationUtils.h"
#include "Utils/GameSettings.h"
#include "Core/Core.h"
#include <algorithm>

namespace {
Node* createLevelNumberNode(int levelId, float targetWidth, float targetHeight) {
    std::string levelText = std::to_string(levelId);
    if (levelText.empty()) {
        return nullptr;
    }

    std::vector<Sprite*> digits;
    digits.reserve(levelText.size());

    float totalWidth = 0.0f;
    float maxHeight = 0.0f;
    constexpr float spacing = 2.0f;

    for (size_t i = 0; i < levelText.size(); ++i) {
        char c = levelText[i];
        if (c < '0' || c > '9') {
            return nullptr;
        }
        std::string path = StringUtils::format("UI/LevelSelect/number/%c.png", c);
        auto digit = Sprite::create(path);
        if (!digit) {
            return nullptr;
        }
        Size size = digit->getContentSize();
        totalWidth += size.width;
        if (i > 0) {
            totalWidth += spacing;
        }
        maxHeight = std::max(maxHeight, size.height);
        digits.push_back(digit);
    }

    if (totalWidth <= 0.0f || maxHeight <= 0.0f) {
        return nullptr;
    }

    float scale = 1.0f;
    if (targetWidth > 0.0f && targetHeight > 0.0f) {
        scale = std::min(targetWidth / totalWidth, targetHeight / maxHeight);
    }

    auto container = Node::create();
    float scaledWidth = totalWidth * scale;
    float scaledHeight = maxHeight * scale;
    container->setContentSize(Size(scaledWidth, scaledHeight));
    container->setAnchorPoint(Vec2(0.5f, 0.5f));
    container->setIgnoreAnchorPointForPosition(false);

    float x = 0.0f;
    for (auto* digit : digits) {
        digit->setAnchorPoint(Vec2(0.0f, 0.5f));
        digit->setScale(scale);
        digit->setPosition(Vec2(x, scaledHeight * 0.5f));
        container->addChild(digit);
        x += (digit->getContentSize().width + spacing) * scale;
    }

    return container;
}

Sprite* createUnitIdleSprite(int unitId, float targetSize, bool forceAnimate) {
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) {
        return nullptr;
    }

    std::string baseName = config->spriteFrameName;
    if (baseName.size() > 4) {
        std::string suffix = baseName.substr(baseName.size() - 4);
        if (suffix == ".png" || suffix == ".PNG") {
            baseName = baseName.substr(0, baseName.size() - 4);
        }
    }

    auto sprite = Sprite::create(baseName + ".png");
    if (!sprite) {
        sprite = Sprite::create(baseName + "_" + config->anim_idle + "_1.png");
    }
    if (!sprite) {
        sprite = Sprite::create();
        auto marker = DrawNode::create();
        marker->drawSolidRect(Vec2(-12.0f, -12.0f), Vec2(12.0f, 12.0f), Color4F(0.3f, 0.3f, 0.3f, 1.0f));
        sprite->addChild(marker);
        return sprite;
    }

    Size contentSize = sprite->getContentSize();
    if (contentSize.width > 0 && contentSize.height > 0) {
        float scale = targetSize / std::max(contentSize.width, contentSize.height);
        sprite->setScale(scale);
    }

    auto anim = AnimationUtils::buildAnimationFromFrames(
        baseName,
        config->anim_idle,
        config->anim_idle_frames,
        config->anim_idle_delay,
        false
    );
    if ((!anim || anim->getFrames().size() <= 1) && forceAnimate) {
        anim = AnimationUtils::buildAnimationFromFrames(
            baseName,
            config->anim_walk,
            config->anim_walk_frames,
            config->anim_walk_delay,
            false
        );
    }

    if (anim && anim->getFrames().size() > 1) {
        auto animate = Animate::create(anim);
        sprite->runAction(RepeatForever::create(animate));
    }

    return sprite;
}
} // namespace

// ===================================================
// 场景创建
// ===================================================

Scene* LevelSelectScene::createScene() {
    return LevelSelectScene::create();
}

Scene* LevelSelectScene::createScene(const std::map<int, int>& selectedUnits) {
    auto scene = LevelSelectScene::create();
    if (scene) {
        scene->setSelectedUnits(selectedUnits);
    }
    return scene;
}

// ===================================================
// 初始化
// ===================================================

bool LevelSelectScene::init() {
    if (!Scene::init()) {
        return false;
    }
    GameSettings::applyBattleSpeed(false);

    CCLOG("[关卡选择] 初始化关卡选择场景");

    if (UnitManager::getInstance()->getAllUnitIds().empty()) {
        UnitManager::getInstance()->loadConfig("res/units_config.json");
    }

    // 初始化关卡数据
    initLevelData(_currentTab);

    // 设置各个UI组件（黑白网格风格）
    setupBackground();
    setupTitle();
    setupModeButton();
    setupLevelButtons();
    setupUnitPreview();
    setupExitButton();
    AudioManager::playMainBgm();

    return true;
}

// ===================================================
// 设置已选择的兵种
// ===================================================

void LevelSelectScene::setSelectedUnits(const std::map<int, int>& units) {
    _selectedUnits = units;
    updateUnitPreview();
}

// ===================================================
// 初始化关卡数据
// ===================================================

void LevelSelectScene::initLevelData(LevelTab tab) {
    _levels.clear();

    int highestCompleted = 0;
    int maxLevels = (tab == LevelTab::Defense)
        ? LevelSelectConfig::MAX_DEFENSE_LEVELS
        : LevelSelectConfig::MAX_ATTACK_LEVELS;

    int levelOffset = (tab == LevelTab::Defense)
        ? LevelSelectConfig::DEFENSE_LEVEL_OFFSET
        : 0;

    for (int i = 1; i <= maxLevels; ++i) {
        int levelId = i + levelOffset;
        if (Core::getInstance()->isLevelCompleted(levelId) && i > highestCompleted) {
            highestCompleted = i;
        }
    }

    int unlockLimit = std::max(3, highestCompleted + 3);
    if (unlockLimit > maxLevels) {
        unlockLimit = maxLevels;
    }

    // 初始化关卡列表
    for (int i = 1; i <= maxLevels; ++i) {
        LevelInfo level;
        level.levelId = i + levelOffset;
        level.displayId = i;
        level.name = "Level " + std::to_string(i);
        level.starCount = Core::getInstance()->getLevelStars(level.levelId);
        level.isUnlocked = (i <= unlockLimit);
        level.description = "Challenge Level " + std::to_string(i);

        if (tab == LevelTab::Attack) {
            switch (i) {
            case 9:
                level.name = "究极地刺";
                level.description = "密集地刺阵，考验部署节奏";
                break;
            case 10:
                level.name = "双管炮的极意";
                level.description = "双管炮覆盖中路，注意兵种分批";
                break;
            case 11:
                level.name = "夺命连环箭";
                level.description = "多重箭塔连锁压制";
                break;
            case 12:
                level.name = "地狱的火焰";
                level.description = "火焰塔与炮塔构成炼狱列阵";
                break;
            default:
                break;
            }
        }

        _levels.push_back(level);
    }
}

// ===================================================
// 背景设置 - 黑白网格风格
// ===================================================

void LevelSelectScene::setupBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _background = Sprite::create("UI/LevelSelect/background/select_level_background.png");
    if (_background) {
        _background->setAnchorPoint(Vec2(0.5f, 0.5f));
        _background->setPosition(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2);
        Size bgSize = _background->getContentSize();
        if (bgSize.width > 0 && bgSize.height > 0) {
            float scaleX = visibleSize.width / bgSize.width;
            float scaleY = visibleSize.height / bgSize.height;
            float scale = std::max(scaleX, scaleY);
            _background->setScale(scale);
        }
        this->addChild(_background, 0);
        return;
    }

    auto bgLayer = LayerColor::create(Color4B(18, 18, 18, 255), visibleSize.width, visibleSize.height);
    bgLayer->setPosition(origin);
    this->addChild(bgLayer, 0);
}

// ===================================================
// 标题设置 - 白色标题（紧凑布局）
// ===================================================

void LevelSelectScene::setupTitle() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 使用TTF字体确保无乱码（缩小字体）
    _titleLabel = Label::createWithTTF(
        "SELECT LEVEL",
        "fonts/ScienceGothic.ttf",
        LevelSelectConfig::TITLE_FONT_SIZE
    );
    if (!_titleLabel) {
        _titleLabel = Label::createWithSystemFont("SELECT LEVEL", "Arial", LevelSelectConfig::TITLE_FONT_SIZE);
    }

    _titleLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    _titleLabel->setPosition(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - LevelSelectConfig::TITLE_TOP_OFFSET
    );
    _titleLabel->setColor(Color3B::WHITE);

    auto titleBg = Sprite::create("UI/LevelSelect/background/label_background.png");
    if (titleBg) {
        titleBg->setAnchorPoint(Vec2(0.5f, 1.0f));
        titleBg->setPosition(_titleLabel->getPosition());
        Size bgSize = titleBg->getContentSize();
        Size labelSize = _titleLabel->getContentSize();
        if (bgSize.width > 0.0f && bgSize.height > 0.0f) {
            float targetWidth = labelSize.width + 26.0f;
            float targetHeight = labelSize.height + 14.0f;
            float scaleX = targetWidth / bgSize.width;
            float scaleY = targetHeight / bgSize.height;
            titleBg->setScale(scaleX, scaleY);
        }
        this->addChild(titleBg, 9);
    }

    this->addChild(_titleLabel, 10);
}

// ===================================================
// 模式切换按钮
// ===================================================

void LevelSelectScene::setupModeButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    if (_modeButtonNode) {
        _modeButtonNode->removeFromParent();
        _modeButtonNode = nullptr;
    }

    const float btnWidth = 180.0f;
    const float btnHeight = 30.0f;
    float btnX = origin.x + visibleSize.width - LevelSelectConfig::EXIT_BUTTON_MARGIN - btnWidth / 2;
    float btnY = origin.y + visibleSize.height - LevelSelectConfig::EXIT_BUTTON_MARGIN - btnHeight / 2;

    _modeButtonNode = Node::create();
    _modeButtonNode->setPosition(Vec2(btnX, btnY));
    this->addChild(_modeButtonNode, 11);

    _modeButtonBg = LayerColor::create(Color4B(50, 50, 50, 255), btnWidth, btnHeight);
    _modeButtonBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    _modeButtonBg->setIgnoreAnchorPointForPosition(false);
    _modeButtonNode->addChild(_modeButtonBg, 0);

    _modeButtonBorder = DrawNode::create();
    _modeButtonBorder->drawRect(Vec2(-btnWidth / 2, -btnHeight / 2), Vec2(btnWidth / 2, btnHeight / 2), Color4F::WHITE);
    _modeButtonNode->addChild(_modeButtonBorder, 1);

    _modeButtonLabel = Label::createWithTTF("", "fonts/ScienceGothic.ttf", 16);
    if (!_modeButtonLabel) {
        _modeButtonLabel = Label::createWithSystemFont("", "Arial", 16);
    }
    _modeButtonLabel->setPosition(Vec2::ZERO);
    _modeButtonLabel->setColor(Color3B::WHITE);
    _modeButtonNode->addChild(_modeButtonLabel, 2);

    _modeButton = Button::create();
    _modeButton->setScale9Enabled(true);
    _modeButton->setContentSize(Size(btnWidth, btnHeight));
    _modeButton->setPosition(Vec2::ZERO);
    _modeButton->setSwallowTouches(true);
    _modeButton->addClickEventListener([this](Ref*) {
        AudioManager::playButtonClick();
        LevelTab nextTab = (_currentTab == LevelTab::Attack) ? LevelTab::Defense : LevelTab::Attack;
        this->switchTab(nextTab);
    });
    _modeButtonNode->addChild(_modeButton, 3);

    updateModeButton();
}

// ===================================================
// 关卡按钮区域设置 - 黑白网格风格
// ===================================================

void LevelSelectScene::setupLevelButtons() {
    if (!_levelButtonArea) {
        auto origin = Director::getInstance()->getVisibleOrigin();
        _levelButtonArea = Node::create();
        _levelButtonArea->setPosition(origin);
        this->addChild(_levelButtonArea, 10);
    }

    rebuildLevelButtons();
}

void LevelSelectScene::rebuildLevelButtons() {
    if (!_levelButtonArea) {
        return;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();

    _levelButtonArea->removeAllChildren();
    _levelButtonNodes.clear();

    // 计算按钮起始位置（网格居中）
    float buttonSize = LevelSelectConfig::LEVEL_BUTTON_SIZE;
    float spacing = LevelSelectConfig::LEVEL_BUTTON_SPACING;
    int cols = LevelSelectConfig::LEVEL_GRID_COLS;
    int rows = (_levels.size() + cols - 1) / cols;

    float gridWidth = cols * buttonSize + (cols - 1) * spacing;
    float gridHeight = rows * buttonSize + (rows - 1) * spacing;

    float centerY = visibleSize.height * LevelSelectConfig::LEVEL_AREA_Y_RATIO;
    float startX = (visibleSize.width - gridWidth) / 2 + buttonSize / 2;
    float startY = centerY + gridHeight / 2 - buttonSize / 2;

    for (size_t i = 0; i < _levels.size(); ++i) {
        int row = static_cast<int>(i) / cols;
        int col = static_cast<int>(i) % cols;
        float buttonX = startX + col * (buttonSize + spacing);
        float buttonY = startY - row * (buttonSize + spacing);

        auto levelNode = createLevelButton(_levels[i], static_cast<int>(i));
        if (levelNode) {
            levelNode->setPosition(Vec2(buttonX, buttonY));
            _levelButtonArea->addChild(levelNode);
            _levelButtonNodes.push_back(levelNode);
        }
    }

    animateLevelButtons();
}

// ===================================================
// 创建单个关卡按钮 - 黑白方块风格
// ===================================================

Node* LevelSelectScene::createLevelButton(const LevelInfo& level, int index) {
    auto node = Node::create();
    float size = LevelSelectConfig::LEVEL_BUTTON_SIZE;

    node->setContentSize(Size(size, size));
    node->setAnchorPoint(Vec2(0.5f, 0.5f));
    node->setIgnoreAnchorPointForPosition(false);

    auto bg = Sprite::create("UI/LevelSelect/background/level_background.png");
    if (bg) {
        bg->setAnchorPoint(Vec2(0.5f, 0.5f));
        bg->setPosition(Vec2(size / 2, size / 2));
        Size bgSize = bg->getContentSize();
        if (bgSize.width > 0 && bgSize.height > 0) {
            float scale = std::min(size / bgSize.width, size / bgSize.height);
            bg->setScale(scale);
        }
        if (!level.isUnlocked) {
            bg->setColor(Color3B(140, 140, 140));
            bg->setOpacity(200);
        }
        node->addChild(bg);
    }
    else {
        Color4B bgColor = level.isUnlocked ? Color4B(60, 60, 60, 255) : Color4B(30, 30, 30, 255);
        auto fallbackBg = LayerColor::create(bgColor, size, size);
        fallbackBg->setAnchorPoint(Vec2(0.5f, 0.5f));
        fallbackBg->setIgnoreAnchorPointForPosition(false);
        fallbackBg->setPosition(Vec2(size / 2, size / 2));
        node->addChild(fallbackBg);

        auto border = DrawNode::create();
        Color4F borderColor = level.isUnlocked ? Color4F::WHITE : Color4F(0.4f, 0.4f, 0.4f, 1.0f);
        border->drawRect(Vec2(0, 0), Vec2(size, size), borderColor);
        node->addChild(border, 1);
    }

    if (level.isUnlocked) {
        auto numberNode = createLevelNumberNode(level.displayId, size * 0.55f, size * 0.4f);
        if (numberNode) {
            numberNode->setPosition(Vec2(size / 2, size / 2 + 6.0f));
            node->addChild(numberNode, 2);
        }
        else {
            auto numLabel = Label::createWithTTF(
                std::to_string(level.displayId),
                "fonts/ScienceGothic.ttf",
                26
            );
            if (!numLabel) {
                numLabel = Label::createWithSystemFont(std::to_string(level.displayId), "Arial", 24);
            }
            numLabel->setPosition(Vec2(size / 2, size / 2 + 6.0f));
            numLabel->setColor(Color3B::WHITE);
            node->addChild(numLabel, 2);
        }
    }

    if (level.isUnlocked && level.starCount > 0) {
        std::string starStr;
        for (int s = 0; s < level.starCount; ++s) {
            starStr += "*";
        }
        for (int s = level.starCount; s < 3; ++s) {
            starStr += "-";
        }
        auto starLabel = Label::createWithTTF(starStr, "fonts/arial.ttf", 10);
        if (!starLabel) {
            starLabel = Label::createWithSystemFont(starStr, "Arial", 10);
        }
        starLabel->setPosition(Vec2(size / 2, size * 0.22f));
        starLabel->setColor(Color3B::YELLOW);
        node->addChild(starLabel, 2);
    }

    if (level.isUnlocked && level.starCount > 0) {
        auto passedSprite = Sprite::create("UI/LevelSelect/passed.png");
        if (passedSprite) {
            passedSprite->setAnchorPoint(Vec2(0.0f, 1.0f));
            float targetSize = size * 0.35f;
            Size tagSize = passedSprite->getContentSize();
            if (tagSize.width > 0 && tagSize.height > 0) {
                float scale = targetSize / std::max(tagSize.width, tagSize.height);
                passedSprite->setScale(scale);
            }
            passedSprite->setPosition(Vec2(4.0f, size - 4.0f));
            node->addChild(passedSprite, 3);
        }
    }

    if (!level.isUnlocked) {
        auto lockedSprite = Sprite::create("UI/LevelSelect/locked.png");
        if (lockedSprite) {
            Size lockSize = lockedSprite->getContentSize();
            float targetSize = size * 0.45f;
            if (lockSize.width > 0 && lockSize.height > 0) {
                float scale = targetSize / std::max(lockSize.width, lockSize.height);
                lockedSprite->setScale(scale);
            }
            lockedSprite->setPosition(Vec2(size / 2, size / 2));
            node->addChild(lockedSprite, 3);
        }
    }

    // 创建透明点击区域
    auto touchBtn = Button::create();
    touchBtn->setContentSize(Size(size, size));
    touchBtn->setScale9Enabled(true);
    touchBtn->setPosition(Vec2(size / 2, size / 2));

    // 绑定点击事件
    int levelId = level.levelId;
    bool unlocked = level.isUnlocked;
    touchBtn->setSwallowTouches(true);
    touchBtn->setEnabled(level.isUnlocked);
    touchBtn->setBright(level.isUnlocked);
    const float originScale = node->getScale();
    touchBtn->addTouchEventListener([this, levelId, unlocked, node, originScale](Ref*, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::BEGAN) {
            if (unlocked) {
                node->setScale(originScale * 0.96f);
            }
            return;
        }
        if (type == Widget::TouchEventType::CANCELED) {
            node->setScale(originScale);
            return;
        }
        if (type != Widget::TouchEventType::ENDED) {
            return;
        }

        node->setScale(originScale);

        if (unlocked) {
            AudioManager::playButtonClick();
            this->onLevelSelected(levelId);
        }
        else {
            CCLOG("[关卡选择] 关卡 %d 尚未解锁", levelId);
            AudioManager::playButtonCancel();
        }
    });

    node->addChild(touchBtn, 10);

    return node;
}

// ===================================================
// 兵种预览区域设置 - 黑白风格
// ===================================================

void LevelSelectScene::setupUnitPreview() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto previewBg = Sprite::create("UI/LevelSelect/background/unit_show_background.png");
    if (previewBg) {
        float centerX = origin.x + visibleSize.width / 2;
        float centerY = origin.y + LevelSelectConfig::UNIT_PREVIEW_BOTTOM + LevelSelectConfig::UNIT_PREVIEW_HEIGHT / 2;
        previewBg->setAnchorPoint(Vec2(0.5f, 0.5f));
        previewBg->setPosition(Vec2(centerX, centerY));
        Size bgSize = previewBg->getContentSize();
        if (bgSize.width > 0 && bgSize.height > 0) {
            float scaleX = visibleSize.width / bgSize.width;
            float scaleY = LevelSelectConfig::UNIT_PREVIEW_HEIGHT / bgSize.height;
            previewBg->setScale(scaleX, scaleY);
        }
        this->addChild(previewBg, 5);
    }
    else {
        auto fallback = LayerColor::create(
            Color4B(26, 26, 26, 255),
            visibleSize.width,
            LevelSelectConfig::UNIT_PREVIEW_HEIGHT
        );
        fallback->setPosition(origin.x, origin.y + LevelSelectConfig::UNIT_PREVIEW_BOTTOM);
        this->addChild(fallback, 5);
    }

    // 预览区域标题
    auto previewTitle = Label::createWithTTF(
        "UNITS:",
        "fonts/ScienceGothic.ttf",
        12
    );
    if (!previewTitle) {
        previewTitle = Label::createWithSystemFont("UNITS:", "Arial", 12);
    }
    previewTitle->setAnchorPoint(Vec2(0, 0.5f));
    previewTitle->setPosition(Vec2(
        origin.x + 10,
        origin.y + LevelSelectConfig::UNIT_PREVIEW_BOTTOM + LevelSelectConfig::UNIT_PREVIEW_HEIGHT - 15
    ));
    previewTitle->setColor(Color3B::WHITE);
    this->addChild(previewTitle, 10);
    _unitPreviewTitle = previewTitle;

    // 兵种图标区域
    _unitPreviewArea = Node::create();
    _unitPreviewArea->setPosition(origin);
    this->addChild(_unitPreviewArea, 10);

    if (_unitPreviewTitle) {
        _unitPreviewTitle->setString(_currentTab == LevelTab::Defense ? "DEFENSE:" : "UNITS:");
    }

    updateUnitPreview();
}

// ===================================================
// 更新兵种预览显示
// ===================================================

void LevelSelectScene::updateUnitPreview() {
    if (!_unitPreviewArea) return;

    _unitPreviewArea->removeAllChildren();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    float size = LevelSelectConfig::UNIT_ICON_SIZE;
    float spacing = LevelSelectConfig::UNIT_ICON_SPACING;
    float baseY = origin.y + LevelSelectConfig::UNIT_PREVIEW_BOTTOM +
        (LevelSelectConfig::UNIT_PREVIEW_HEIGHT - size) / 2;

    if (_currentTab == LevelTab::Defense) {
        auto defenseLabel = Label::createWithTTF(
            "Defense mode - no deployment",
            "fonts/ScienceGothic.ttf",
            10
        );
        if (!defenseLabel) {
            defenseLabel = Label::createWithSystemFont("Defense mode - no deployment", "Arial", 10);
        }
        defenseLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
        defenseLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, baseY + size / 2));
        defenseLabel->setColor(Color3B::GRAY);
        _unitPreviewArea->addChild(defenseLabel);
        return;
    }

    if (_selectedUnits.empty()) {
        // 没有选择兵种时显示提示（缩小字体）
        auto noUnitLabel = Label::createWithTTF(
            "No units trained.",
            "fonts/ScienceGothic.ttf",
            10
        );
        if (!noUnitLabel) {
            noUnitLabel = Label::createWithSystemFont("No units trained.", "Arial", 10);
        }
        noUnitLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
        noUnitLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, baseY + size / 2));
        noUnitLabel->setColor(Color3B::GRAY);
        _unitPreviewArea->addChild(noUnitLabel);
        return;
    }

    // 水平居中兵种预览行
    float totalWidth = _selectedUnits.size() * size +
        (_selectedUnits.size() - 1) * spacing;
    float xPos = origin.x + (visibleSize.width - totalWidth) / 2;
    for (const auto& pair : _selectedUnits) {
        auto iconNode = createUnitPreviewIcon(pair.first, pair.second);
        if (iconNode) {
            iconNode->setPosition(Vec2(xPos, baseY));
            _unitPreviewArea->addChild(iconNode);
            xPos += size + spacing;
        }
    }
}

// ===================================================
// 创建兵种预览图标 - 黑白风格（缩小尺寸）
// ===================================================

Node* LevelSelectScene::createUnitPreviewIcon(int unitId, int count) {
    auto node = Node::create();
    float size = LevelSelectConfig::UNIT_ICON_SIZE;

    node->setContentSize(Size(size, size));
    node->setAnchorPoint(Vec2(0.0f, 0.0f));
    node->setIgnoreAnchorPointForPosition(false);

    auto bg = Sprite::create("UI/LevelSelect/background/unit_background.png");
    if (bg) {
        bg->setAnchorPoint(Vec2(0.5f, 0.5f));
        bg->setPosition(Vec2(size / 2, size / 2));
        Size bgSize = bg->getContentSize();
        if (bgSize.width > 0 && bgSize.height > 0) {
            float scale = std::min(size / bgSize.width, size / bgSize.height);
            bg->setScale(scale);
        }
        node->addChild(bg, 0);
    }
    else {
        auto fallbackBg = LayerColor::create(Color4B(45, 45, 45, 255), size, size);
        fallbackBg->setAnchorPoint(Vec2(0.5f, 0.5f));
        fallbackBg->setIgnoreAnchorPointForPosition(false);
        fallbackBg->setPosition(Vec2(size / 2, size / 2));
        node->addChild(fallbackBg, 0);

        auto border = DrawNode::create();
        border->drawRect(Vec2(0, 0), Vec2(size, size), Color4F::WHITE);
        node->addChild(border, 1);
    }

    auto idleSprite = createUnitIdleSprite(unitId, size * 0.62f, true);
    if (idleSprite) {
        idleSprite->setPosition(Vec2(size / 2, size / 2 + 4));
        node->addChild(idleSprite, 2);
    }

    // 数量
    auto countLabel = Label::createWithTTF(
        "x" + std::to_string(count),
        "fonts/ScienceGothic.ttf",
        10
    );
    if (!countLabel) {
        countLabel = Label::createWithSystemFont("x" + std::to_string(count), "Arial", 10);
    }
    countLabel->setPosition(Vec2(size / 2, 10));
    countLabel->setColor(Color3B::YELLOW);
    node->addChild(countLabel, 2);

    return node;
}

// ===================================================
// 退出按钮设置 - 黑白风格（缩小尺寸）
// ===================================================
// 退出按钮设置 - 使用Button组件确保点击可靠
// ===================================================

void LevelSelectScene::setupExitButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _exitButton = Button::create("UI/exit.png", "UI/exit.png");
    if (!_exitButton) {
        _exitButton = Button::create("exit.png", "exit.png");
    }
    if (!_exitButton) {
        return;
    }

    Size btnSize = _exitButton->getContentSize();
    if (btnSize.width > 0.0f && btnSize.height > 0.0f) {
        float scale = LevelSelectConfig::EXIT_BUTTON_SIZE / std::max(btnSize.width, btnSize.height);
        _exitButton->setScale(scale);
    }

    float btnX = origin.x + LevelSelectConfig::EXIT_BUTTON_MARGIN + LevelSelectConfig::EXIT_BUTTON_SIZE / 2;
    float btnY = origin.y + visibleSize.height - LevelSelectConfig::EXIT_BUTTON_MARGIN - LevelSelectConfig::EXIT_BUTTON_SIZE / 2;
    _exitButton->setPosition(Vec2(btnX, btnY));
    _exitButton->setPressedActionEnabled(true);
    _exitButton->setZoomScale(0.06f);
    _exitButton->setSwallowTouches(true);
    _exitButton->addClickEventListener([this](Ref*) {
        AudioManager::playButtonCancel();
        CCLOG("[关卡选择] 点击返回按钮");
        this->onExitButton(nullptr);
    });
    this->addChild(_exitButton, 21);
}

// ===================================================
// 关卡按钮进入动画
// ===================================================
void LevelSelectScene::animateLevelButtons() {
    if (_levelButtonNodes.empty()) {
        return;
    }

    const float delayStep = 0.04f;
    const float moveOffset = 14.0f;

    for (size_t i = 0; i < _levelButtonNodes.size(); ++i) {
        auto node = _levelButtonNodes[i];
        if (!node) {
            continue;
        }
        node->stopAllActions();
        Vec2 originPos = node->getPosition();
        node->setPosition(originPos - Vec2(0.0f, moveOffset));
        node->setScale(0.92f);
        auto move = EaseBackOut::create(MoveTo::create(0.25f, originPos));
        auto scale = EaseBackOut::create(ScaleTo::create(0.25f, 1.0f));
        auto spawn = Spawn::create(move, scale, nullptr);
        node->runAction(Sequence::create(DelayTime::create(delayStep * i), spawn, nullptr));
    }
}

// ===================================================
// 关卡选择回调
// ===================================================

void LevelSelectScene::onLevelSelected(int levelId) {
    CCLOG("[关卡选择] 选择了关卡: %d", levelId);
    _selectedLevel = levelId;

    // 直接开始战斗
    startBattle(levelId);
}

// ===================================================
// 开始战斗 - 跳转到战斗场景
// ===================================================

void LevelSelectScene::startBattle(int levelId) {
    CCLOG("[关卡选择] 开始战斗: 关卡 %d", levelId);

    // 引入战斗场景头文件在cpp顶部
    // 跳转到战斗场景，传入已选择的兵种
    // 传入已训练兵种，不使用默认兵种
    bool defenseMode = (_currentTab == LevelTab::Defense);
    auto scene = BattleScene::createScene(levelId, _selectedUnits, false, defenseMode);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// ===================================================
// 选项卡切换
// ===================================================

void LevelSelectScene::switchTab(LevelTab tab) {
    if (_currentTab == tab) {
        return;
    }

    _currentTab = tab;
    initLevelData(tab);
    rebuildLevelButtons();
    updateModeButton();
    updateUnitPreview();
}

void LevelSelectScene::updateModeButton() {
    if (_modeButtonLabel) {
        _modeButtonLabel->setString(_currentTab == LevelTab::Defense ? "MODE: DEFENSE" : "MODE: ATTACK");
    }
    if (_modeButtonBg) {
        _modeButtonBg->setColor(_currentTab == LevelTab::Defense ? Color3B(80, 60, 60) : Color3B(50, 50, 50));
    }
    if (_modeButtonBorder && _modeButtonBg) {
        Size size = _modeButtonBg->getContentSize();
        _modeButtonBorder->clear();
        _modeButtonBorder->drawRect(
            Vec2(-size.width / 2, -size.height / 2),
            Vec2(size.width / 2, size.height / 2),
            Color4F::WHITE
        );
    }
    if (_unitPreviewTitle) {
        _unitPreviewTitle->setString(_currentTab == LevelTab::Defense ? "DEFENSE:" : "UNITS:");
    }
}

// ===================================================
// 退出按钮回调
// ===================================================

void LevelSelectScene::onExitButton(Ref* sender) {
    CCLOG("[关卡选择] 返回基地");

    auto scene = BaseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}
