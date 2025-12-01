// MainMenuScene.cpp
// 主菜单场景实现文件
// 功能: 实现主菜单界面的显示和交互

#include "MainMenuScene.h"
#include "BaseScene.h"

// ==================== 场景创建 ====================

Scene* MainMenuScene::createScene() {
    return MainMenuScene::create();
}

bool MainMenuScene::init() {
    if (!Scene::init()) {
        return false;
    }
    
    // 创建各个UI组件
    createBackground();
    createTitle();
    createMenuButtons();
    
    // 播放背景动画
    playBackgroundAnimation();
    
    // 启用帧更新
    scheduleUpdate();
    
    return true;
}

// ==================== UI创建 ====================

void MainMenuScene::createBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建渐变背景
    _background = Node::create();
    
    // 绘制渐变背景
    auto bgDraw = DrawNode::create();
    
    // 天空渐变(从深蓝到浅蓝)
    Vec2 vertices[4] = {
        Vec2(0, 0),
        Vec2(visibleSize.width, 0),
        Vec2(visibleSize.width, visibleSize.height),
        Vec2(0, visibleSize.height)
    };
    
    Color4F colors[4] = {
        Color4F(0.1f, 0.3f, 0.2f, 1.0f),   // 左下 - 深绿
        Color4F(0.1f, 0.3f, 0.2f, 1.0f),   // 右下 - 深绿
        Color4F(0.2f, 0.5f, 0.7f, 1.0f),   // 右上 - 天蓝
        Color4F(0.2f, 0.5f, 0.7f, 1.0f)    // 左上 - 天蓝
    };
    
    // 绘制背景矩形
    bgDraw->drawSolidRect(Vec2(0, 0), Vec2(visibleSize.width, visibleSize.height), 
                          Color4F(0.15f, 0.35f, 0.25f, 1.0f));
    
    // 添加装饰性元素
    // 绘制一些"山丘"
    for (int i = 0; i < 5; i++) {
        float x = i * visibleSize.width / 4;
        float height = 50 + rand() % 100;
        float width = 200 + rand() % 150;
        
        Vec2 hillVerts[3] = {
            Vec2(x - width/2, 0),
            Vec2(x, height),
            Vec2(x + width/2, 0)
        };
        
        bgDraw->drawPolygon(hillVerts, 3, Color4F(0.2f, 0.45f, 0.3f, 1.0f), 0, Color4F::BLACK);
    }
    
    // 绘制一些装饰性建筑轮廓
    for (int i = 0; i < 8; i++) {
        float x = 50 + i * (visibleSize.width - 100) / 7;
        float y = 20;
        float width = 30 + rand() % 40;
        float height = 60 + rand() % 80;
        
        Vec2 buildingVerts[4] = {
            Vec2(x - width/2, y),
            Vec2(x + width/2, y),
            Vec2(x + width/2, y + height),
            Vec2(x - width/2, y + height)
        };
        
        bgDraw->drawPolygon(buildingVerts, 4, Color4F(0.1f, 0.2f, 0.15f, 0.5f), 0, Color4F::BLACK);
        
        // 添加屋顶
        Vec2 roofVerts[3] = {
            Vec2(x - width/2 - 5, y + height),
            Vec2(x, y + height + 30),
            Vec2(x + width/2 + 5, y + height)
        };
        bgDraw->drawPolygon(roofVerts, 3, Color4F(0.3f, 0.2f, 0.1f, 0.5f), 0, Color4F::BLACK);
    }
    
    _background->addChild(bgDraw);
    _background->setPosition(origin);
    this->addChild(_background, Z_BACKGROUND);
    
    // 添加一些云朵
    for (int i = 0; i < 5; i++) {
        auto cloud = DrawNode::create();
        float cx = rand() % (int)visibleSize.width;
        float cy = visibleSize.height * 0.6f + rand() % (int)(visibleSize.height * 0.3f);
        
        // 绘制云朵(多个圆组成)
        float size = 20 + rand() % 30;
        cloud->drawSolidCircle(Vec2(0, 0), size, 0, 20, Color4F(1, 1, 1, 0.4f));
        cloud->drawSolidCircle(Vec2(size * 0.8f, 0), size * 0.7f, 0, 20, Color4F(1, 1, 1, 0.4f));
        cloud->drawSolidCircle(Vec2(-size * 0.8f, 0), size * 0.7f, 0, 20, Color4F(1, 1, 1, 0.4f));
        cloud->drawSolidCircle(Vec2(0, size * 0.5f), size * 0.6f, 0, 20, Color4F(1, 1, 1, 0.4f));
        
        cloud->setPosition(Vec2(cx, cy));
        _background->addChild(cloud);
        
        // 云朵飘动动画
        float moveDistance = 100 + rand() % 100;
        float duration = 10 + rand() % 10;
        auto move = MoveBy::create(duration, Vec2(moveDistance, 0));
        auto moveBack = MoveBy::create(duration, Vec2(-moveDistance, 0));
        auto seq = Sequence::create(move, moveBack, nullptr);
        cloud->runAction(RepeatForever::create(seq));
    }
}

void MainMenuScene::createTitle() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建标题
    _titleLabel = Label::createWithSystemFont("Void Kings", "Arial", 72);
    if (_titleLabel) {
        _titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                     origin.y + visibleSize.height * 0.75f));
        _titleLabel->setColor(Color3B(255, 215, 0));  // 金色
        
        // 添加标题阴影效果
        _titleLabel->enableShadow(Color4B(0, 0, 0, 200), Size(3, -3), 5);
        _titleLabel->enableOutline(Color4B(100, 50, 0, 255), 3);
        
        this->addChild(_titleLabel, Z_UI);
        
        // 标题呼吸动画
        auto scaleUp = ScaleTo::create(1.5f, 1.05f);
        auto scaleDown = ScaleTo::create(1.5f, 1.0f);
        auto seq = Sequence::create(scaleUp, scaleDown, nullptr);
        _titleLabel->runAction(RepeatForever::create(seq));
    }
    
    // 创建副标题
    auto subtitleLabel = Label::createWithSystemFont("Clash of Clans Style Game", "Arial", 24);
    if (subtitleLabel) {
        subtitleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                       origin.y + visibleSize.height * 0.65f));
        subtitleLabel->setColor(Color3B(200, 200, 200));
        this->addChild(subtitleLabel, Z_UI);
    }
}

void MainMenuScene::createMenuButtons() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    Vector<MenuItem*> menuItems;
    
    // 按钮样式设置 - 统一尺寸确保对齐
    const float buttonWidth = 220;
    const float buttonHeight = 55;
    const float buttonSpacing = 75;
    const float buttonCenterX = origin.x + visibleSize.width / 2;
    const float startY = origin.y + visibleSize.height * 0.42f;
    const float borderWidth = 3.0f;
    
    // 辅助函数：创建带背景的按钮
    auto createButton = [=](const std::string& text, const Color4F& bgColor, 
                           const Color4F& borderColor, const ccMenuCallback& callback) -> MenuItem* {
        auto label = Label::createWithSystemFont(text, "Arial", 26);
        label->setColor(Color3B::WHITE);
        auto button = MenuItemLabel::create(label, callback);
        
        auto bg = DrawNode::create();
        // 绘制背景矩形
        bg->drawSolidRect(Vec2(-buttonWidth/2, -buttonHeight/2), 
                          Vec2(buttonWidth/2, buttonHeight/2), bgColor);
        // 绘制边框 - 使用更粗的线条
        bg->drawRect(Vec2(-buttonWidth/2, -buttonHeight/2), 
                     Vec2(buttonWidth/2, buttonHeight/2), borderColor);
        // 添加内部高亮线
        bg->drawLine(Vec2(-buttonWidth/2 + 2, buttonHeight/2 - 2),
                     Vec2(buttonWidth/2 - 2, buttonHeight/2 - 2),
                     Color4F(1.0f, 1.0f, 1.0f, 0.3f));
        button->addChild(bg, -1);
        
        return button;
    };
    
    // 开始游戏按钮
    auto startButton = createButton("Start Game", 
                                    Color4F(0.2f, 0.6f, 0.3f, 0.95f),
                                    Color4F(0.3f, 0.8f, 0.4f, 1.0f),
                                    CC_CALLBACK_1(MainMenuScene::onStartGame, this));
    startButton->setPosition(Vec2(buttonCenterX, startY));
    menuItems.pushBack(startButton);
    
    // 帮助按钮（新增：游戏规则说明）
    auto helpButton = createButton("Help / Rules",
                                   Color4F(0.3f, 0.5f, 0.6f, 0.95f),
                                   Color4F(0.4f, 0.7f, 0.8f, 1.0f),
                                   CC_CALLBACK_1(MainMenuScene::onSettings, this));  // 暂时复用settings
    helpButton->setPosition(Vec2(buttonCenterX, startY - buttonSpacing));
    menuItems.pushBack(helpButton);
    
    // 设置按钮
    auto settingsButton = createButton("Settings",
                                       Color4F(0.3f, 0.3f, 0.5f, 0.95f),
                                       Color4F(0.5f, 0.5f, 0.7f, 1.0f),
                                       CC_CALLBACK_1(MainMenuScene::onSettings, this));
    settingsButton->setPosition(Vec2(buttonCenterX, startY - buttonSpacing * 2));
    menuItems.pushBack(settingsButton);
    
    // 退出按钮
    auto exitButton = createButton("Exit",
                                   Color4F(0.6f, 0.2f, 0.2f, 0.95f),
                                   Color4F(0.8f, 0.3f, 0.3f, 1.0f),
                                   CC_CALLBACK_1(MainMenuScene::onExit, this));
    exitButton->setPosition(Vec2(buttonCenterX, startY - buttonSpacing * 3));
    menuItems.pushBack(exitButton);
    
    // 创建菜单
    _menu = Menu::createWithArray(menuItems);
    _menu->setPosition(Vec2::ZERO);
    this->addChild(_menu, Z_UI);
    
    // 版本信息 - 右下角对齐
    auto versionLabel = Label::createWithSystemFont("Version 1.0.0", "Arial", 14);
    versionLabel->setAnchorPoint(Vec2(1.0f, 0.0f));  // 右下角锚点
    versionLabel->setPosition(Vec2(origin.x + visibleSize.width - 15, origin.y + 15));
    versionLabel->setColor(Color3B(150, 150, 150));
    this->addChild(versionLabel, Z_UI);
    
    // 版权信息 - 左下角对齐
    auto copyrightLabel = Label::createWithSystemFont("© 2025 Tongji University - Software Engineering", "Arial", 12);
    copyrightLabel->setAnchorPoint(Vec2(0.0f, 0.0f));  // 左下角锚点
    copyrightLabel->setPosition(Vec2(origin.x + 15, origin.y + 15));
    copyrightLabel->setColor(Color3B(120, 120, 120));
    this->addChild(copyrightLabel, Z_UI);
}

void MainMenuScene::playBackgroundAnimation() {
    // 背景动画效果已在createBackground中实现
}

// ==================== 按钮回调 ====================

void MainMenuScene::onStartGame(Ref* sender) {
    // 播放点击音效(如果有)
    
    // 切换到基地场景
    auto scene = BaseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
}

void MainMenuScene::onSettings(Ref* sender) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建帮助/规则弹窗
    auto helpPanel = Node::create();
    helpPanel->setName("helpPanel");
    
    // 背景遮罩
    auto mask = DrawNode::create();
    mask->drawSolidRect(Vec2(0, 0), Vec2(visibleSize.width, visibleSize.height),
                        Color4F(0, 0, 0, 0.7f));
    helpPanel->addChild(mask);
    
    // 面板尺寸
    const float panelWidth = 700;
    const float panelHeight = 500;
    const float panelX = (visibleSize.width - panelWidth) / 2;
    const float panelY = (visibleSize.height - panelHeight) / 2;
    
    // 面板背景
    auto panel = DrawNode::create();
    panel->drawSolidRect(Vec2(panelX, panelY), Vec2(panelX + panelWidth, panelY + panelHeight),
                         Color4F(0.12f, 0.12f, 0.18f, 0.98f));
    panel->drawRect(Vec2(panelX, panelY), Vec2(panelX + panelWidth, panelY + panelHeight),
                    Color4F(0.4f, 0.6f, 0.8f, 1.0f));
    helpPanel->addChild(panel);
    
    // 标题
    auto titleLabel = Label::createWithSystemFont("Game Rules / Help", "Arial", 32);
    titleLabel->setPosition(Vec2(visibleSize.width / 2, panelY + panelHeight - 40));
    titleLabel->setColor(Color3B(255, 215, 0));
    helpPanel->addChild(titleLabel);
    
    // 规则内容
    std::string rulesText = 
        "=== Game Objectives ===\n"
        "Build your base, train troops, and attack enemy bases to earn resources!\n\n"
        
        "=== Building Types ===\n"
        "- Town Hall: Core building, protects your base\n"
        "- Cannon/Archer Tower: Defense buildings, attack enemies\n"
        "- Gold Mine/Elixir Collector: Produce resources over time\n"
        "- Gold/Elixir Storage: Store your resources\n"
        "- Barracks: Train troops for battle\n"
        "- Wall: Block enemy troops\n\n"
        
        "=== Battle System ===\n"
        "- Deploy troops at the edge of enemy base\n"
        "- Destroy 50% for 1 star, Town Hall for 2 stars, 100% for 3 stars\n"
        "- Earn Gold and Elixir by destroying enemy buildings\n"
        "- Battle time limit: 3 minutes\n\n"
        
        "=== Controls ===\n"
        "- Drag to move the camera\n"
        "- Scroll to zoom in/out\n"
        "- Click buildings to select and see info\n"
        "- Click troop icon then click to deploy";
    
    auto rulesLabel = Label::createWithSystemFont(rulesText, "Arial", 16);
    rulesLabel->setPosition(Vec2(visibleSize.width / 2, panelY + panelHeight / 2 - 20));
    rulesLabel->setColor(Color3B(220, 220, 220));
    rulesLabel->setAlignment(TextHAlignment::LEFT, TextVAlignment::TOP);
    rulesLabel->setMaxLineWidth(panelWidth - 60);
    helpPanel->addChild(rulesLabel);
    
    // 关闭按钮
    auto closeLabel = Label::createWithSystemFont("Close", "Arial", 22);
    closeLabel->setColor(Color3B::WHITE);
    auto closeButton = MenuItemLabel::create(closeLabel, [helpPanel](Ref* sender) {
        helpPanel->removeFromParent();
    });
    auto closeBg = DrawNode::create();
    closeBg->drawSolidRect(Vec2(-60, -20), Vec2(60, 20), Color4F(0.5f, 0.3f, 0.3f, 1.0f));
    closeBg->drawRect(Vec2(-60, -20), Vec2(60, 20), Color4F(0.7f, 0.4f, 0.4f, 1.0f));
    closeButton->addChild(closeBg, -1);
    closeButton->setPosition(Vec2(visibleSize.width / 2, panelY + 45));
    
    auto menu = Menu::create(closeButton, nullptr);
    menu->setPosition(Vec2::ZERO);
    helpPanel->addChild(menu);
    
    helpPanel->setPosition(origin);
    this->addChild(helpPanel, Z_POPUP);
}

void MainMenuScene::onExit(Ref* sender) {
    // 退出游戏
    Director::getInstance()->end();
}

// ==================== 窗口大小变化 ====================

void MainMenuScene::onWindowResize(float width, float height) {
    // 重新布局UI元素
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    if (_titleLabel) {
        _titleLabel->setPosition(Vec2(origin.x + width / 2, origin.y + height * 0.75f));
    }
    
    // 重新创建背景
    if (_background) {
        _background->removeFromParent();
    }
    createBackground();
}

// ==================== 帧更新 ====================

void MainMenuScene::update(float dt) {
    // 可以添加一些动态效果
}
