// LevelSelectScene.cpp
// 关卡选择场景实现
// 采用黑白网格风格设计

#include "LevelSelectScene.h"
#include "BaseScene.h"
#include "BattleScene.h"
#include "Soldier/UnitManager.h"
#include "Soldier/UnitData.h"

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

    CCLOG("[关卡选择] 初始化关卡选择场景");

    // 初始化关卡数据
    initLevelData();

    // 设置各个UI组件（黑白网格风格）
    setupBackground();
    setupTitle();
    setupLevelButtons();
    setupUnitPreview();
    setupExitButton();

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

void LevelSelectScene::initLevelData() {
    // 初始化12个关卡
    for (int i = 1; i <= LevelSelectConfig::MAX_LEVELS; ++i) {
        LevelInfo level;
        level.levelId = i;
        level.name = "Level " + std::to_string(i);
        level.isUnlocked = true;  // 全部关卡开放
        level.starCount = 0;
        level.description = "Challenge Level " + std::to_string(i);

        _levels.push_back(level);
    }
}

// ===================================================
// 背景设置 - 黑白网格风格
// ===================================================

void LevelSelectScene::setupBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 深色渐变背景
    auto bgLayer = LayerGradient::create(
        Color4B(18, 18, 18, 255),
        Color4B(32, 32, 32, 255)
    );
    bgLayer->setContentSize(visibleSize);
    bgLayer->setPosition(origin);
    this->addChild(bgLayer, 0);

    // 绘制网格线
    auto gridNode = DrawNode::create();
    float gridSpacing = 50.0f;

    // 绘制垂直线
    for (float x = 0; x <= visibleSize.width; x += gridSpacing) {
        gridNode->drawLine(
            Vec2(x, 0),
            Vec2(x, visibleSize.height),
            Color4F(0.12f, 0.12f, 0.12f, 1.0f)
        );
    }

    // 绘制水平线
    for (float y = 0; y <= visibleSize.height; y += gridSpacing) {
        gridNode->drawLine(
            Vec2(0, y),
            Vec2(visibleSize.width, y),
            Color4F(0.12f, 0.12f, 0.12f, 1.0f)
        );
    }

    gridNode->setPosition(origin);
    this->addChild(gridNode, 1);
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

    this->addChild(_titleLabel, 10);

    // 标题下方的装饰线
    auto decorLine = DrawNode::create();
    float lineY = origin.y + visibleSize.height - LevelSelectConfig::TITLE_TOP_OFFSET - 25;
    float lineHalfWidth = 110.0f;
    decorLine->drawLine(
        Vec2(origin.x + visibleSize.width / 2 - lineHalfWidth, lineY),
        Vec2(origin.x + visibleSize.width / 2 + lineHalfWidth, lineY),
        Color4F::WHITE
    );
    this->addChild(decorLine, 10);
}

// ===================================================
// 关卡按钮区域设置 - 黑白网格风格
// ===================================================

void LevelSelectScene::setupLevelButtons() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _levelButtonArea = Node::create();
    _levelButtonArea->setPosition(origin);
    this->addChild(_levelButtonArea, 10);

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

    // 背景方块 - 黑白风格
    Color4B bgColor = level.isUnlocked ? Color4B(60, 60, 60, 255) : Color4B(30, 30, 30, 255);
    auto bg = LayerColor::create(bgColor, size, size);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    node->addChild(bg);

    // 边框
    auto border = DrawNode::create();
    Color4F borderColor = level.isUnlocked ? Color4F::WHITE : Color4F(0.4f, 0.4f, 0.4f, 1.0f);
    border->drawRect(
        Vec2(-size / 2, -size / 2),
        Vec2(size / 2, size / 2),
        borderColor
    );
    node->addChild(border, 1);

    // 关卡编号
    auto numLabel = Label::createWithTTF(
        std::to_string(level.levelId),
        "fonts/ScienceGothic.ttf",
        26
    );
    if (!numLabel) {
        numLabel = Label::createWithSystemFont(std::to_string(level.levelId), "Arial", 24);
    }
    numLabel->setPosition(Vec2(0, 8));
    numLabel->setColor(level.isUnlocked ? Color3B::WHITE : Color3B::GRAY);
    node->addChild(numLabel, 2);

    // 关卡名称
    auto nameLabel = Label::createWithTTF(level.name, "fonts/arial.ttf", 10);
    if (!nameLabel) {
        nameLabel = Label::createWithSystemFont(level.name, "Arial", 10);
    }
    nameLabel->setPosition(Vec2(0, -8));
    nameLabel->setColor(level.isUnlocked ? Color3B(210, 210, 210) : Color3B::GRAY);
    node->addChild(nameLabel, 2);

    // 星星显示（已完成的关卡）
    if (level.starCount > 0) {
        std::string starStr = "";
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
        starLabel->setPosition(Vec2(0, -22));
        starLabel->setColor(Color3B::YELLOW);
        node->addChild(starLabel, 2);
    }

    // 锁定标识（缩小字体）
    if (!level.isUnlocked) {
        auto lockLabel = Label::createWithTTF("LOCKED", "fonts/arial.ttf", 7);
        if (!lockLabel) {
            lockLabel = Label::createWithSystemFont("LOCKED", "Arial", 7);
        }
        lockLabel->setPosition(Vec2(0, -15));
        lockLabel->setColor(Color3B::GRAY);
        node->addChild(lockLabel, 2);
    }

    // 创建透明点击区域
    auto touchBtn = Button::create();
    touchBtn->setContentSize(Size(size, size));
    touchBtn->setScale9Enabled(true);

    // 绑定点击事件
    int levelId = level.levelId;
    bool unlocked = level.isUnlocked;
    touchBtn->setSwallowTouches(true);
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
            this->onLevelSelected(levelId);
        }
        else {
            CCLOG("[关卡选择] 关卡 %d 尚未解锁", levelId);
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

    // 创建预览区域背景 - 深灰色
    auto previewBg = LayerColor::create(
        Color4B(26, 26, 26, 255),
        visibleSize.width,
        LevelSelectConfig::UNIT_PREVIEW_HEIGHT
    );
    previewBg->setPosition(origin.x, origin.y + LevelSelectConfig::UNIT_PREVIEW_BOTTOM);
    this->addChild(previewBg, 5);

    // 边框线
    auto borderLine = DrawNode::create();
    borderLine->drawLine(
        Vec2(origin.x, origin.y + LevelSelectConfig::UNIT_PREVIEW_BOTTOM + LevelSelectConfig::UNIT_PREVIEW_HEIGHT),
        Vec2(origin.x + visibleSize.width, origin.y + LevelSelectConfig::UNIT_PREVIEW_BOTTOM + LevelSelectConfig::UNIT_PREVIEW_HEIGHT),
        Color4F::WHITE
    );
    this->addChild(borderLine, 6);

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

    // 兵种图标区域
    _unitPreviewArea = Node::create();
    _unitPreviewArea->setPosition(origin);
    this->addChild(_unitPreviewArea, 10);

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

    // 获取兵种配置
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    std::string unitName = config ? config->name : "???";

    // 图标背景
    auto bg = LayerColor::create(
        Color4B(45, 45, 45, 255),
        size,
        size
    );
    node->addChild(bg);

    // 边框
    auto border = DrawNode::create();
    border->drawRect(Vec2(0, 0), Vec2(size, size), Color4F::WHITE);
    node->addChild(border, 1);

    // 兵种名称（简写）
    auto nameLabel = Label::createWithTTF(
        unitName.substr(0, 3),
        "fonts/ScienceGothic.ttf",
        10
    );
    if (!nameLabel) {
        nameLabel = Label::createWithSystemFont(unitName.substr(0, 3), "Arial", 9);
    }
    nameLabel->setPosition(Vec2(size / 2, size / 2 + 6));
    nameLabel->setColor(Color3B(230, 230, 230));
    node->addChild(nameLabel, 2);

    // 数量
    auto countLabel = Label::createWithTTF(
        "x" + std::to_string(count),
        "fonts/ScienceGothic.ttf",
        10
    );
    if (!countLabel) {
        countLabel = Label::createWithSystemFont("x" + std::to_string(count), "Arial", 10);
    }
    countLabel->setPosition(Vec2(size / 2, size / 2 - 8));
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

    // 按钮尺寸
    float btnWidth = 60.0f;
    float btnHeight = 25.0f;

    // 创建按钮容器节点
    auto exitNode = Node::create();

    // 按钮背景
    auto bg = LayerColor::create(Color4B(40, 40, 40, 255), btnWidth, btnHeight);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));
    bg->setIgnoreAnchorPointForPosition(false);
    exitNode->addChild(bg);

    // 边框
    auto border = DrawNode::create();
    border->drawRect(Vec2(-btnWidth / 2, -btnHeight / 2), Vec2(btnWidth / 2, btnHeight / 2), Color4F::WHITE);
    exitNode->addChild(border, 1);

    // 文字
    auto label = Label::createWithTTF("BACK", "fonts/arial.ttf", 11);
    if (!label) {
        label = Label::createWithSystemFont("BACK", "Arial", 11);
    }
    label->setColor(Color3B::WHITE);
    exitNode->addChild(label, 2);

    // 设置位置
    float btnX = origin.x + LevelSelectConfig::EXIT_BUTTON_MARGIN + btnWidth / 2;
    float btnY = origin.y + visibleSize.height - LevelSelectConfig::EXIT_BUTTON_MARGIN - btnHeight / 2;
    exitNode->setPosition(Vec2(btnX, btnY));
    this->addChild(exitNode, 20);

    // 创建透明Button覆盖在上面，确保点击可靠
    auto touchBtn = Button::create();
    touchBtn->setContentSize(Size(btnWidth, btnHeight));
    touchBtn->setScale9Enabled(true);
    touchBtn->setPosition(Vec2(btnX, btnY));
    touchBtn->setSwallowTouches(true);
    const float originScale = exitNode->getScale();
    touchBtn->addTouchEventListener([this, exitNode, originScale](Ref*, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::BEGAN) {
            exitNode->setScale(originScale * 0.96f);
            return;
        }
        if (type == Widget::TouchEventType::CANCELED) {
            exitNode->setScale(originScale);
            return;
        }
        if (type != Widget::TouchEventType::ENDED) {
            return;
        }

        exitNode->setScale(originScale);
        CCLOG("[关卡选择] 点击返回按钮");
        this->onExitButton(nullptr);
    });
    this->addChild(touchBtn, 21);
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
    auto scene = BattleScene::createScene(levelId, _selectedUnits, false);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// ===================================================
// 退出按钮回调
// ===================================================

void LevelSelectScene::onExitButton(Ref* sender) {
    CCLOG("[关卡选择] 返回基地");

    auto scene = BaseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}
