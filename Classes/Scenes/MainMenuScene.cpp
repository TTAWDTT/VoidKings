/****************************************************************************
 Copyright (c) 2013-2014 cocos2d-x.org
 Copyright (c) 2015-2017 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "MainMenuScene.h"
#include "BaseScene.h"

Scene* MainMenuScene::createScene()
{
	return MainMenuScene::create();
}

bool MainMenuScene::init()
{
	if (!Scene::init())
	{
		return false;
	}
	// 创建各个UI组件
	createBackground();
	createHeadLogo();
	createOtherThings();
	
	// 创建各个界面层
	createMainMenuLayer();
	createSettingsLayer();
	createRuleLayer();

	// 初始显示主菜单
	mainMenuLayer->setVisible(true);
	settingsLayer->setVisible(false);
	ruleLayer->setVisible(false);

	return true;
}

void MainMenuScene::createBackground()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 背景图列表
    std::vector<std::string> bgList = {
        "background/background1.png",
        "background/background2.png",
        "background/background3.png",
		"background/background4.png"
    };

    int bgCount = bgList.size();
    if (bgCount == 0) return;

    for (int i = 0; i < bgCount; i++)
    {
        auto bg = Sprite::create(bgList[i]);
        if (!bg) continue;

        bg->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));

        // 缩放适配窗口
        Size bgSize = bg->getContentSize();
        float scaleX = visibleSize.width / bgSize.width;
        float scaleY = visibleSize.height / bgSize.height;
        float finalScale = std::max(scaleX, scaleY);
        bg->setScale(finalScale);

        // 所有背景先透明
        bg->setOpacity(0);

        this->addChild(bg, -10); // 放在最底层

        backgroundList.pushBack(bg);  // 存入成员变量
    }

    // 第一张淡入
    if (!backgroundList.empty())
    {
        backgroundList.at(0)->runAction(FadeTo::create(1.2f, 255));
        currentBgIndex = 0;
    }

    // 每 6 秒切一次图
    this->schedule(CC_SCHEDULE_SELECTOR(MainMenuScene::switchBackground), 6.0f);
}

void MainMenuScene::switchBackground(float dt)
{
    if (backgroundList.empty()) return;

    int nextIndex = (currentBgIndex + 1) % backgroundList.size();

    auto currentBg = backgroundList.at(currentBgIndex);
    auto nextBg = backgroundList.at(nextIndex);

    // 当前淡出
    currentBg->runAction(FadeTo::create(1.5f, 0));

    // 下一个淡入
    nextBg->runAction(FadeTo::create(1.5f, 255));

    currentBgIndex = nextIndex;
}

void MainMenuScene::createHeadLogo()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    headLogo = Label::createWithTTF("Void Kings", "fonts/Marker Felt.ttf", 48);
    headLogo->setTextColor(Color4B(255, 200, 50, 255));
    headLogo->setAnchorPoint(Vec2(0.5f, 1.0f));
    headLogo->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - 24.0f));
    this->addChild(headLogo, 1);
}

void MainMenuScene::createOtherThings()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 创建初始精灵（从第0帧开始）
    auto dinosaur = Sprite::create("dinosaur/sprite_0000.png");
    if (!dinosaur) {
        CCLOG("Failed to load dinosaur sprite");
        return;
    }

    // 创建动画
    Animation* anim = Animation::create();
    for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 6; k++) {
            for (int i = 2; i <= 11; ++i) {
                std::string framePath = StringUtils::format("dinosaur/sprite_%04d.png", 10 * j + i);
                auto frame = SpriteFrame::create(framePath, Rect(0, 0, 64, 64));
                if (frame) {
                    anim->addSpriteFrame(frame);
                }
            }
        }
    }
    anim->setDelayPerUnit(0.1f);

    // 配置精灵并播放动画
    dinosaur->setScale(4.0f);
    dinosaur->setAnchorPoint(Vec2(1.0f, 0.0f)); // 右下角锚点
    
    // 位置：右下角，留边距
    float margin = 24.0f;
    dinosaur->setPosition(Vec2(origin.x + visibleSize.width - margin,
                               origin.y + margin));

    // 添加到场景（不是mainMenuLayer，而是直接添加到Scene）
    this->addChild(dinosaur, 5);

    // 播放循环动画
    dinosaur->runAction(RepeatForever::create(Animate::create(anim)));

    // 存储精灵指针便于后续控制（如需要）
    otherThings = dinosaur;
}

Button* MainMenuScene::createIconButton(
    const std::string& title,
    const std::string& iconPath,
    const Widget::ccWidgetTouchCallback& callback)
{
    float padding = 10;
    float leftPadding = 5;
    float spacing = 5;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 按钮背景
    auto button = Button::create("btn_normal.png", "btn_pressed.png");
    button->setScale9Enabled(true);

    // 文字
    button->setTitleFontName("fonts/ScienceGothic.ttf");
    button->setTitleFontSize(28); // 增大字体
    button->setTitleText(title);
 
    auto titleRenderer = button->getTitleRenderer();
    auto titleSize = titleRenderer->getContentSize();

    // 图标
    auto icon = Sprite::create(iconPath);

    // 先用文字高度估一个按钮高度
    float tmpHeight = std::max(icon->getContentSize().height, titleSize.height) + padding;

    // 图标自动缩放
    float targetHeight = tmpHeight * 0.85f; // 稍微增大图标比例
    float scale = targetHeight / icon->getContentSize().height;
    icon->setScale(scale);

    float iconWidth = icon->getContentSize().width * scale;
    float iconHeight = icon->getContentSize().height * scale;

    // 计算按钮最终大小
    float buttonWidth = 200; // 增大按钮宽度

    float buttonHeight =
        std::max(iconHeight, titleSize.height) + padding + 10; // 增加高度padding

    button->setContentSize(Size(buttonWidth, buttonHeight));

    auto innerBg = DrawNode::create();

    float margin = 4.0f;     // 与外层按钮的边距
    Color4F boxColor = Color4F(0.2f, 0.4f, 0.9f, 0.5f); // 半透明蓝色

    innerBg->drawSolidRect(
        Vec2(margin, margin),
        Vec2(buttonWidth - margin, buttonHeight - margin),
        boxColor);
    button->addChild(innerBg, -5);

    // 图标位置
    icon->setAnchorPoint(Vec2(0, 0.5));
    icon->setPosition(Vec2(leftPadding, buttonHeight / 2));
    button->addChild(icon,1);

    // 文字位置
    titleRenderer->setAnchorPoint(Vec2(0, 0.5));
    titleRenderer->setPosition(Vec2(
        leftPadding + iconWidth + spacing,
        buttonHeight / 2));

    // 点击事件
    button->addTouchEventListener(callback);

    return button;
}

void MainMenuScene::createMainMenuLayer() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    mainMenuLayer = Node::create();
    mainMenuLayer->setPosition(origin);
    this->addChild(mainMenuLayer, 10);

    // 按钮组居中布局
    float spacingY = 60.0f;
    float centerX = origin.x + visibleSize.width / 2;

    // Start
    auto startBtn = createIconButton(
        "Start",
        "start.png",
        CC_CALLBACK_2(MainMenuScene::onStartTouch, this)
    );
    float btnHeight = startBtn->getContentSize().height;
    float totalHeight = btnHeight * 4 + spacingY * 3;
    float startY = origin.y + visibleSize.height / 2 + totalHeight / 2 - btnHeight / 2;
    startBtn->setPosition(Vec2(centerX, startY));
    mainMenuLayer->addChild(startBtn);

    // Settings
    auto settingsBtn = createIconButton(
        "Settings",
        "settings.png",
        CC_CALLBACK_2(MainMenuScene::onSettingsTouch, this)
    );
    settingsBtn->setPosition(Vec2(centerX, startY - (btnHeight + spacingY)));
    mainMenuLayer->addChild(settingsBtn);

    // Rules
    auto rulesBtn = createIconButton(
        "Rules",
        "rules.png",
        CC_CALLBACK_2(MainMenuScene::onRuleTouch, this)
    );
    rulesBtn->setPosition(Vec2(centerX, startY - 2 * (btnHeight + spacingY)));
    mainMenuLayer->addChild(rulesBtn);

    // Exit
    auto exitBtn = createIconButton(
        "Exit",
        "exit.png",
        CC_CALLBACK_2(MainMenuScene::onExitTouch, this)
    );
    exitBtn->setPosition(Vec2(centerX, startY - 3 * (btnHeight + spacingY)));
    mainMenuLayer->addChild(exitBtn);
}

void MainMenuScene::onStart(Ref* sender) {
    auto scene = BaseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
}

void MainMenuScene::switchToLayer(Node* targetLayer)
{
    mainMenuLayer->setVisible(targetLayer == mainMenuLayer);
    settingsLayer->setVisible(targetLayer == settingsLayer);
    ruleLayer->setVisible(targetLayer == ruleLayer);
}

void MainMenuScene::createSettingsLayer()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    settingsLayer = Node::create();
    this->addChild(settingsLayer, 20);

	// 面板背景
    float panelWidth = visibleSize.width * 0.6f;   // 面板宽度
    float panelHeight = visibleSize.height * 0.7f; // 面板高度

    auto panel = DrawNode::create();
    Color4F panelColor(0.0f, 0.0f, 0.0f, 0.45f);  // 半透明黑
    float radius = 20.0f; // 圆角

    panel->drawSolidRect(Vec2(-panelWidth / 2, -panelHeight / 2),
        Vec2(panelWidth / 2, panelHeight / 2),
        panelColor);

    panel->setPosition(Vec2(visibleSize.width / 2,
        visibleSize.height / 2-20));
    settingsLayer->addChild(panel, 1);

    //标题
    auto label = Label::createWithTTF("Settings", "fonts/ScienceGothic.ttf", 32);
    label->setPosition(Vec2(0, panelHeight / 2 - 20));  // 相对 panel 的位置
    panel->addChild(label, 2);

    //Return 按钮
    auto returnBtn = createIconButton(
        "Return",
        "exit.png",   // 图标
        CC_CALLBACK_2(MainMenuScene::onReturnTouch, this)
    );

    returnBtn->setPosition(Vec2(0, -panelHeight / 2 + 60)); // 放在底部区域
    panel->addChild(returnBtn, 2);
}


void MainMenuScene::createRuleLayer()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    ruleLayer = Node::create();
    this->addChild(ruleLayer, 20);

    // 面板背景
    float panelWidth = visibleSize.width * 0.6f;   // 面板宽度
    float panelHeight = visibleSize.height * 0.7f; // 面板高度

    auto panel = DrawNode::create();
    Color4F panelColor(0.0f, 0.0f, 0.0f, 0.45f);  // 半透明黑
    float radius = 20.0f; // 圆角

    panel->drawSolidRect(Vec2(-panelWidth / 2, -panelHeight / 2),
        Vec2(panelWidth / 2, panelHeight / 2),
        panelColor);

    panel->setPosition(Vec2(visibleSize.width / 2,
        visibleSize.height / 2 - 20));
    ruleLayer->addChild(panel, 1);

    //标题
    auto label = Label::createWithTTF("Rule", "fonts/ScienceGothic.ttf", 32);
    label->setPosition(Vec2(0, panelHeight / 2 - 20));  // 相对 panel 的位置
    panel->addChild(label, 2);

    //Return 按钮
    auto returnBtn = createIconButton(
        "Return",
        "exit.png",   // 图标
        CC_CALLBACK_2(MainMenuScene::onReturnTouch, this)
    );

    returnBtn->setPosition(Vec2(0, -panelHeight / 2 + 60)); // 放在底部区域
    panel->addChild(returnBtn, 2);
}

void MainMenuScene::onReturnTouch(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        onReturn(sender);
    }
}

void MainMenuScene::onStartTouch(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        onStart(sender);
    }
}

void MainMenuScene::onSettingsTouch(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        onSettings(sender);
    }
}

void MainMenuScene::onRuleTouch(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        onRule(sender);
    }
}

void MainMenuScene::onExitTouch(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        onExit(sender);
    }
}

void MainMenuScene::onSettings(Ref* sender)
{
    switchToLayer(settingsLayer);
}

void MainMenuScene::onRule(Ref* sender)
{
	switchToLayer(ruleLayer);
}

void MainMenuScene::onReturn(Ref* sender)
{
	switchToLayer(mainMenuLayer);
}

void MainMenuScene::onExit(Ref* sender) 
{
    Director::getInstance()->end();
}
