// LevelSelectScene.cpp
// 关卡选择场景实现

#include "LevelSelectScene.h"
#include "BaseScene.h"
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
    
    // 设置各个UI组件
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
    // 初始化5个关卡
    for (int i = 1; i <= LevelSelectConfig::MAX_LEVELS; ++i) {
        LevelInfo level;
        level.levelId = i;
        level.name = "Level " + std::to_string(i);
        level.isUnlocked = (i <= 3);  // 前3关解锁
        level.starCount = (i == 1) ? 3 : (i == 2) ? 2 : 0;  // 模拟已完成状态
        level.description = "Challenge Level " + std::to_string(i);
        
        _levels.push_back(level);
    }
}

// ===================================================
// 背景设置
// ===================================================

void LevelSelectScene::setupBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 尝试加载背景图片
    _background = Sprite::create("UI/level_select_bg.png");
    if (!_background) {
        // 如果背景图不存在，使用渐变色背景
        auto bgLayer = LayerColor::create(
            Color4B(30, 40, 60, 255),
            visibleSize.width,
            visibleSize.height
        );
        bgLayer->setPosition(origin);
        this->addChild(bgLayer, 0);
        
        // 添加装饰性渐变层
        auto gradientLayer = LayerGradient::create(
            Color4B(20, 30, 50, 200),
            Color4B(50, 70, 100, 200),
            Vec2(0, 1)
        );
        gradientLayer->setContentSize(visibleSize);
        gradientLayer->setPosition(origin);
        this->addChild(gradientLayer, 1);
    } else {
        // 缩放背景以覆盖屏幕
        float scaleX = visibleSize.width / _background->getContentSize().width;
        float scaleY = visibleSize.height / _background->getContentSize().height;
        _background->setScale(MAX(scaleX, scaleY));
        _background->setPosition(
            origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2
        );
        this->addChild(_background, 0);
    }
}

// ===================================================
// 标题设置
// ===================================================

void LevelSelectScene::setupTitle() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _titleLabel = Label::createWithSystemFont(
        "Select Level",
        "Arial",
        LevelSelectConfig::TITLE_FONT_SIZE
    );
    _titleLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    _titleLabel->setPosition(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - LevelSelectConfig::TITLE_TOP_OFFSET
    );
    _titleLabel->setColor(Color3B::WHITE);
    
    // 添加阴影效果
    _titleLabel->enableShadow(Color4B::BLACK, Size(2, -2), 3);
    
    this->addChild(_titleLabel, 10);
}

// ===================================================
// 关卡按钮区域设置
// ===================================================

void LevelSelectScene::setupLevelButtons() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _levelButtonArea = Node::create();
    _levelButtonArea->setPosition(origin);
    this->addChild(_levelButtonArea, 10);
    
    // 计算按钮起始位置（水平居中）
    float totalWidth = _levels.size() * LevelSelectConfig::LEVEL_BUTTON_SIZE + 
                       (_levels.size() - 1) * LevelSelectConfig::LEVEL_BUTTON_SPACING;
    float startX = (visibleSize.width - totalWidth) / 2 + LevelSelectConfig::LEVEL_BUTTON_SIZE / 2;
    float buttonY = visibleSize.height * LevelSelectConfig::LEVEL_AREA_Y_RATIO;
    
    for (size_t i = 0; i < _levels.size(); ++i) {
        float buttonX = startX + i * (LevelSelectConfig::LEVEL_BUTTON_SIZE + LevelSelectConfig::LEVEL_BUTTON_SPACING);
        
        auto levelNode = createLevelButton(_levels[i], static_cast<int>(i));
        if (levelNode) {
            levelNode->setPosition(Vec2(buttonX, buttonY));
            _levelButtonArea->addChild(levelNode);
        }
    }
}

// ===================================================
// 创建单个关卡按钮
// ===================================================

Node* LevelSelectScene::createLevelButton(const LevelInfo& level, int index) {
    auto node = Node::create();
    
    // 按钮背景
    Button* btn = nullptr;
    if (level.isUnlocked) {
        btn = Button::create("btn_normal.png", "btn_pressed.png");
    } else {
        btn = Button::create("btn_normal.png");  // 锁定状态
    }
    
    if (!btn) {
        // 如果按钮图片不存在，创建简单背景
        auto bg = LayerColor::create(
            level.isUnlocked ? Color4B(80, 120, 180, 255) : Color4B(80, 80, 80, 255),
            LevelSelectConfig::LEVEL_BUTTON_SIZE,
            LevelSelectConfig::LEVEL_BUTTON_SIZE
        );
        bg->setAnchorPoint(Vec2(0.5f, 0.5f));
        bg->setIgnoreAnchorPointForPosition(false);
        node->addChild(bg);
        
        // 创建一个透明按钮覆盖在上面接收点击
        btn = Button::create();
        btn->setContentSize(Size(
            LevelSelectConfig::LEVEL_BUTTON_SIZE,
            LevelSelectConfig::LEVEL_BUTTON_SIZE
        ));
    }
    
    btn->setScale(1.5f);
    node->addChild(btn);
    
    // 关卡编号
    auto numLabel = Label::createWithSystemFont(
        std::to_string(level.levelId),
        "ScienceGothic",
        24
    );
    numLabel->setPosition(Vec2(0, 5));
    numLabel->setColor(level.isUnlocked ? Color3B::WHITE : Color3B::GRAY);
    node->addChild(numLabel);
    
    // 星星显示（已完成的关卡）
    if (level.starCount > 0) {
        std::string starStr = "";
        for (int s = 0; s < level.starCount; ++s) {
            starStr += "★";
        }
        auto starLabel = Label::createWithSystemFont(starStr, "Arial", 12);
        starLabel->setPosition(Vec2(0, -25));
        starLabel->setColor(Color3B::YELLOW);
        node->addChild(starLabel);
    }
    
    // 锁定图标
    if (!level.isUnlocked) {
        auto lockLabel = Label::createWithSystemFont("??", "Arial", 20);
        lockLabel->setPosition(Vec2(0, -25));
        node->addChild(lockLabel);
    }
    
    // 绑定点击事件
    int levelId = level.levelId;
    bool unlocked = level.isUnlocked;
    btn->addClickEventListener([this, levelId, unlocked](Ref* sender) {
        if (unlocked) {
            this->onLevelSelected(levelId);
        } else {
            CCLOG("[关卡选择] 关卡 %d 尚未解锁", levelId);
        }
    });
    
    return node;
}

// ===================================================
// 兵种预览区域设置
// ===================================================

void LevelSelectScene::setupUnitPreview() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建预览区域背景
    auto previewBg = LayerColor::create(
        Color4B(0, 0, 0, 150),
        visibleSize.width,
        LevelSelectConfig::UNIT_PREVIEW_HEIGHT
    );
    previewBg->setPosition(origin.x, origin.y + LevelSelectConfig::UNIT_PREVIEW_BOTTOM);
    this->addChild(previewBg, 5);
    
    // 预览区域标题
    auto previewTitle = Label::createWithSystemFont(
        "Deploy Units:",
        "Arial",
        18
    );
    previewTitle->setAnchorPoint(Vec2(0, 0.5f));
    previewTitle->setPosition(Vec2(
        origin.x + 20,
        origin.y + LevelSelectConfig::UNIT_PREVIEW_BOTTOM + LevelSelectConfig::UNIT_PREVIEW_HEIGHT - 25
    ));
    previewTitle->setColor(Color3B::WHITE);
    this->addChild(previewTitle, 10);
    
    // 兵种图标区域
    _unitPreviewArea = Node::create();
    _unitPreviewArea->setPosition(Vec2(
        origin.x + 30,
        origin.y + LevelSelectConfig::UNIT_PREVIEW_BOTTOM + 30
    ));
    this->addChild(_unitPreviewArea, 10);
    
    updateUnitPreview();
}

// ===================================================
// 更新兵种预览显示
// ===================================================

void LevelSelectScene::updateUnitPreview() {
    if (!_unitPreviewArea) return;
    
    _unitPreviewArea->removeAllChildren();
    
    if (_selectedUnits.empty()) {
        // 没有选择兵种时显示提示
        auto noUnitLabel = Label::createWithSystemFont(
            "No units. Train in barracks first.",
            "Arial",
            14
        );
        noUnitLabel->setAnchorPoint(Vec2(0, 0.5f));
        noUnitLabel->setPosition(Vec2(0, LevelSelectConfig::UNIT_ICON_SIZE / 2));
        noUnitLabel->setColor(Color3B::GRAY);
        _unitPreviewArea->addChild(noUnitLabel);
        return;
    }
    
    float xPos = 0;
    for (const auto& pair : _selectedUnits) {
        auto iconNode = createUnitPreviewIcon(pair.first, pair.second);
        if (iconNode) {
            iconNode->setPosition(Vec2(xPos, 0));
            _unitPreviewArea->addChild(iconNode);
            xPos += LevelSelectConfig::UNIT_ICON_SIZE + LevelSelectConfig::UNIT_ICON_SPACING;
        }
    }
}

// ===================================================
// 创建兵种预览图标
// ===================================================

Node* LevelSelectScene::createUnitPreviewIcon(int unitId, int count) {
    auto node = Node::create();
    
    // 获取兵种配置
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    std::string unitName = config ? config->name : "???";
    
    // 图标背景
    auto bg = LayerColor::create(
        Color4B(60, 80, 100, 255),
        LevelSelectConfig::UNIT_ICON_SIZE,
        LevelSelectConfig::UNIT_ICON_SIZE
    );
    node->addChild(bg);
    
    // 兵种名称（简写）
    auto nameLabel = Label::createWithSystemFont(
        unitName.substr(0, 3),
        "Arial",
        12
    );
    nameLabel->setPosition(Vec2(
        LevelSelectConfig::UNIT_ICON_SIZE / 2,
        LevelSelectConfig::UNIT_ICON_SIZE / 2 + 8
    ));
    nameLabel->setColor(Color3B::WHITE);
    node->addChild(nameLabel);
    
    // 数量
    auto countLabel = Label::createWithSystemFont(
        "x" + std::to_string(count),
        "Arial",
        14
    );
    countLabel->setPosition(Vec2(
        LevelSelectConfig::UNIT_ICON_SIZE / 2,
        LevelSelectConfig::UNIT_ICON_SIZE / 2 - 12
    ));
    countLabel->setColor(Color3B::YELLOW);
    node->addChild(countLabel);
    
    return node;
}

// ===================================================
// 退出按钮设置
// ===================================================

void LevelSelectScene::setupExitButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _exitButton = Button::create("UI/exit.png");
    if (!_exitButton) {
        _exitButton = Button::create("btn_normal.png", "btn_pressed.png");
        if (_exitButton) {
            _exitButton->setTitleText("Back");
            _exitButton->setTitleFontSize(16);
        }
    }
    
    if (_exitButton) {
        _exitButton->setScale(3.0f);
        _exitButton->setPosition(Vec2(
            origin.x + LevelSelectConfig::EXIT_BUTTON_MARGIN + LevelSelectConfig::EXIT_BUTTON_SIZE / 2,
            origin.y + visibleSize.height - LevelSelectConfig::EXIT_BUTTON_MARGIN - LevelSelectConfig::EXIT_BUTTON_SIZE / 2
        ));
        _exitButton->addClickEventListener(CC_CALLBACK_1(LevelSelectScene::onExitButton, this));
        this->addChild(_exitButton, 20);
    }
}

// ===================================================
// 关卡选择回调
// ===================================================

void LevelSelectScene::onLevelSelected(int levelId) {
    CCLOG("[关卡选择] 选择了关卡: %d", levelId);
    _selectedLevel = levelId;
    
    // 可以在这里显示确认对话框或直接开始战斗
    startBattle(levelId);
}

// ===================================================
// 开始战斗
// ===================================================

void LevelSelectScene::startBattle(int levelId) {
    CCLOG("[关卡选择] 开始战斗: 关卡 %d", levelId);
    
    // TODO: 这里应该跳转到战斗场景
    // 目前暂时返回BaseScene
    // auto battleScene = BattleScene::createScene(levelId, _selectedUnits);
    // Director::getInstance()->replaceScene(TransitionFade::create(0.5f, battleScene));
    
    // 临时：显示提示
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    auto tipLabel = Label::createWithSystemFont(
        "Battle Scene in development...\nLevel: " + std::to_string(levelId),
        "Arial",
        24
    );
    tipLabel->setPosition(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2 - 100
    );
    tipLabel->setColor(Color3B::YELLOW);
    this->addChild(tipLabel, 100);
    
    // 2秒后返回
    this->scheduleOnce([](float dt) {
        auto scene = BaseScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
    }, 2.0f, "return_to_base");
}

// ===================================================
// 退出按钮回调
// ===================================================

void LevelSelectScene::onExitButton(Ref* sender) {
    CCLOG("[关卡选择] 返回基地");
    
    auto scene = BaseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}
