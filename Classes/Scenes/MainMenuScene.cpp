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
#include "Save/SaveManager.h"
#include "Utils/AudioManager.h"
#include <algorithm>

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
	createModalOverlay();
    SaveManager::getInstance();
	
	// 创建各个界面层
	createMainMenuLayer();
	createSettingsLayer();
	createRuleLayer();
    createSaveLayer();

	// 初始显示主菜单
	mainMenuLayer->setVisible(true);
	settingsLayer->setVisible(false);
	ruleLayer->setVisible(false);
    if (saveLayer) {
        saveLayer->setVisible(false);
    }
	animateMenuButtons();
	AudioManager::playMainBgm();

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

    // 标题轻微浮动，增强视觉动感
    auto floatUp = EaseSineInOut::create(MoveBy::create(1.8f, Vec2(0.0f, 6.0f)));
    auto floatDown = EaseSineInOut::create(MoveBy::create(1.8f, Vec2(0.0f, -6.0f)));
    headLogo->runAction(RepeatForever::create(Sequence::create(floatUp, floatDown, nullptr)));
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

    // 轻微上下浮动，增加呼吸感
    auto bobUp = EaseSineInOut::create(MoveBy::create(1.4f, Vec2(0.0f, 5.0f)));
    auto bobDown = EaseSineInOut::create(MoveBy::create(1.4f, Vec2(0.0f, -5.0f)));
    dinosaur->runAction(RepeatForever::create(Sequence::create(bobUp, bobDown, nullptr)));

}

void MainMenuScene::createModalOverlay()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    modalOverlay = LayerColor::create(Color4B(0, 0, 0, 120),
        visibleSize.width,
        visibleSize.height);
    modalOverlay->setPosition(origin);
    modalOverlay->setVisible(false);
    this->addChild(modalOverlay, 15);
}

Button* MainMenuScene::createIconButton(
    const std::string& title,
    const std::string& iconPath,
    const Widget::ccWidgetTouchCallback& callback)
{
    float padding = 10;
    float leftPadding = 5;
    float spacing = 5;

    // 按钮背景
    auto button = Button::create("btn_normal.png", "btn_pressed.png");
    button->setScale9Enabled(true);
    button->setPressedActionEnabled(true);
    button->setZoomScale(0.05f);
    button->setSwallowTouches(true);

    // 文字
    button->setTitleFontName("fonts/ScienceGothic.ttf");
    button->setTitleFontSize(28); // 增大字体
    button->setTitleText(title);
 
    auto titleRenderer = button->getTitleRenderer();
    auto titleSize = titleRenderer->getContentSize();

    // 图标
    auto icon = Sprite::create(iconPath);
    if (!icon) {
        float buttonWidth = 200;
        float buttonHeight = titleSize.height + padding + 10;
        button->setContentSize(Size(buttonWidth, buttonHeight));
        titleRenderer->setAnchorPoint(Vec2(0.5f, 0.5f));
        titleRenderer->setPosition(Vec2(buttonWidth / 2, buttonHeight / 2));
        button->addTouchEventListener(callback);
        return button;
    }

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

Button* MainMenuScene::createTextButton(
    const std::string& title,
    float width,
    float height,
    const Widget::ccWidgetTouchCallback& callback)
{
    auto button = Button::create("btn_normal.png", "btn_pressed.png");
    if (!button) {
        return nullptr;
    }
    button->setScale9Enabled(true);
    button->setContentSize(Size(width, height));
    button->setTitleFontName("fonts/ScienceGothic.ttf");
    button->setTitleFontSize(20);
    button->setTitleText(title);
    button->setPressedActionEnabled(true);
    button->setZoomScale(0.05f);
    button->setSwallowTouches(true);
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
    menuButtons.push_back(startBtn);

    // Settings
    auto settingsBtn = createIconButton(
        "Settings",
        "settings.png",
        CC_CALLBACK_2(MainMenuScene::onSettingsTouch, this)
    );
    settingsBtn->setPosition(Vec2(centerX, startY - (btnHeight + spacingY)));
    mainMenuLayer->addChild(settingsBtn);
    menuButtons.push_back(settingsBtn);

    // Rules
    auto rulesBtn = createIconButton(
        "Rules",
        "rules.png",
        CC_CALLBACK_2(MainMenuScene::onRuleTouch, this)
    );
    rulesBtn->setPosition(Vec2(centerX, startY - 2 * (btnHeight + spacingY)));
    mainMenuLayer->addChild(rulesBtn);
    menuButtons.push_back(rulesBtn);

    // Exit
    auto exitBtn = createIconButton(
        "Exit",
        "exit.png",
        CC_CALLBACK_2(MainMenuScene::onExitTouch, this)
    );
    exitBtn->setPosition(Vec2(centerX, startY - 3 * (btnHeight + spacingY)));
    mainMenuLayer->addChild(exitBtn);
    menuButtons.push_back(exitBtn);
}

void MainMenuScene::onStart(Ref* sender) {
    showSaveLayer();
}

void MainMenuScene::switchToLayer(Node* targetLayer)
{
    mainMenuLayer->setVisible(targetLayer == mainMenuLayer);
    settingsLayer->setVisible(targetLayer == settingsLayer);
    ruleLayer->setVisible(targetLayer == ruleLayer);
    if (saveLayer) {
        saveLayer->setVisible(targetLayer == saveLayer);
    }

    const bool showModal = targetLayer != mainMenuLayer;
    if (modalOverlay) {
        modalOverlay->stopAllActions();
        modalOverlay->setVisible(showModal);
        if (showModal) {
            modalOverlay->setOpacity(0);
            modalOverlay->runAction(FadeTo::create(0.18f, 160));
        }
        else {
            modalOverlay->runAction(Sequence::create(
                FadeTo::create(0.12f, 0),
                CallFunc::create([this]() {
                    if (modalOverlay) {
                        modalOverlay->setVisible(false);
                    }
                }),
                nullptr));
        }
    }

    if (targetLayer == settingsLayer) {
        animatePanel(settingsPanel);
    }
    else if (targetLayer == ruleLayer) {
        animatePanel(rulePanel);
    }
    else if (targetLayer == saveLayer) {
        animatePanel(savePanel);
    }
}

void MainMenuScene::createSettingsLayer()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    settingsLayer = Node::create();
    this->addChild(settingsLayer, 20);

    // 半透明遮罩
    auto dimBg = LayerColor::create(Color4B(0, 0, 0, 80),
        visibleSize.width,
        visibleSize.height);
    dimBg->setPosition(origin);
    settingsLayer->addChild(dimBg, 0);

	// 面板背景
    float panelWidth = visibleSize.width * 0.6f;   // 面板宽度
    float panelHeight = visibleSize.height * 0.7f; // 面板高度

    auto panel = DrawNode::create();
    Color4F panelColor(0.0f, 0.0f, 0.0f, 0.45f);  // 半透明黑

    panel->drawSolidRect(Vec2(-panelWidth / 2, -panelHeight / 2),
        Vec2(panelWidth / 2, panelHeight / 2),
        panelColor);

    panel->setPosition(Vec2(visibleSize.width / 2,
        visibleSize.height / 2-20));
    settingsLayer->addChild(panel, 1);
    settingsPanel = panel;

    //标题
    auto label = Label::createWithTTF("Settings", "fonts/ScienceGothic.ttf", 32);
    label->setPosition(Vec2(0, panelHeight / 2 - 20));  // 相对 panel 的位置
    panel->addChild(label, 2);

    // 设置说明内容
    std::string settingsText =
        "Audio: On\n"
        "Music Volume: 55%\n"
        "SFX Volume: 90%\n"
        "Language: English\n"
        "Difficulty: Normal\n"
        "Tips: Enabled";
    auto settingsLabel = Label::createWithTTF(settingsText, "fonts/ScienceGothic.ttf", 20);
    if (!settingsLabel) {
        settingsLabel = Label::createWithSystemFont(settingsText, "Arial", 20);
    }
    settingsLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    settingsLabel->setAlignment(TextHAlignment::LEFT);
    settingsLabel->setPosition(Vec2(0, panelHeight / 2 - 80));
    settingsLabel->setLineSpacing(8.0f);
    panel->addChild(settingsLabel, 2);

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
    auto origin = Director::getInstance()->getVisibleOrigin();

    ruleLayer = Node::create();
    this->addChild(ruleLayer, 20);

    // 半透明遮罩
    auto dimBg = LayerColor::create(Color4B(0, 0, 0, 80),
        visibleSize.width,
        visibleSize.height);
    dimBg->setPosition(origin);
    ruleLayer->addChild(dimBg, 0);

    // 面板背景
    float panelWidth = visibleSize.width * 0.6f;   // 面板宽度
    float panelHeight = visibleSize.height * 0.7f; // 面板高度

    auto panel = DrawNode::create();
    Color4F panelColor(0.0f, 0.0f, 0.0f, 0.45f);  // 半透明黑

    panel->drawSolidRect(Vec2(-panelWidth / 2, -panelHeight / 2),
        Vec2(panelWidth / 2, panelHeight / 2),
        panelColor);

    panel->setPosition(Vec2(visibleSize.width / 2,
        visibleSize.height / 2 - 20));
    ruleLayer->addChild(panel, 1);
    rulePanel = panel;

    //标题
    auto label = Label::createWithTTF("Rules", "fonts/ScienceGothic.ttf", 32);
    label->setPosition(Vec2(0, panelHeight / 2 - 20));  // 相对 panel 的位置
    panel->addChild(label, 2);

    // 规则说明内容
    std::string rulesText =
        "1. Deploy units within the green zone.\n"
        "2. Destroy enemy buildings to gain progress.\n"
        "3. Fewer casualties earn more stars.\n"
        "4. Traps trigger on contact.\n"
        "5. Defeat the enemy base to win early.";
    auto rulesLabel = Label::createWithTTF(rulesText, "fonts/ScienceGothic.ttf", 20);
    if (!rulesLabel) {
        rulesLabel = Label::createWithSystemFont(rulesText, "Arial", 20);
    }
    rulesLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    rulesLabel->setAlignment(TextHAlignment::LEFT);
    rulesLabel->setPosition(Vec2(0, panelHeight / 2 - 80));
    rulesLabel->setLineSpacing(8.0f);
    panel->addChild(rulesLabel, 2);

    //Return 按钮
    auto returnBtn = createIconButton(
        "Return",
        "exit.png",   // 图标
        CC_CALLBACK_2(MainMenuScene::onReturnTouch, this)
    );

    returnBtn->setPosition(Vec2(0, -panelHeight / 2 + 60)); // 放在底部区域
    panel->addChild(returnBtn, 2);
}

void MainMenuScene::createSaveLayer()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    saveLayer = Node::create();
    this->addChild(saveLayer, 20);

    auto dimBg = LayerColor::create(Color4B(0, 0, 0, 80),
        visibleSize.width,
        visibleSize.height);
    dimBg->setPosition(origin);
    saveLayer->addChild(dimBg, 0);

    float panelWidth = visibleSize.width * 0.7f;
    float panelHeight = visibleSize.height * 0.75f;

    auto panel = DrawNode::create();
    Color4F panelColor(0.0f, 0.0f, 0.0f, 0.45f);
    panel->drawSolidRect(Vec2(-panelWidth / 2, -panelHeight / 2),
        Vec2(panelWidth / 2, panelHeight / 2),
        panelColor);
    panel->setPosition(Vec2(visibleSize.width / 2,
        visibleSize.height / 2 - 20));
    saveLayer->addChild(panel, 1);
    savePanel = panel;

    auto title = Label::createWithTTF("Save Slots", "fonts/ScienceGothic.ttf", 32);
    if (!title) {
        title = Label::createWithSystemFont("Save Slots", "Arial", 32);
    }
    title->setPosition(Vec2(0, panelHeight / 2 - 20));
    panel->addChild(title, 2);

    saveSlotLabels.clear();
    saveSlotLoadButtons.clear();
    saveSlotDeleteButtons.clear();

    float rowStartY = panelHeight / 2 - 80.0f;
    float rowSpacing = 60.0f;
    float labelX = -panelWidth / 2 + 30.0f;
    float loadX = panelWidth / 2 - 220.0f;
    float deleteX = panelWidth / 2 - 90.0f;
    float btnWidth = 100.0f;
    float btnHeight = 36.0f;

    for (int i = 0; i < 6; ++i) {
        int slot = i + 1;
        float y = rowStartY - i * rowSpacing;

        std::string labelText = StringUtils::format("Slot %d: Empty", slot);
        auto label = Label::createWithTTF(labelText, "fonts/ScienceGothic.ttf", 18);
        if (!label) {
            label = Label::createWithSystemFont(labelText, "Arial", 18);
        }
        label->setAnchorPoint(Vec2(0, 0.5f));
        label->setPosition(Vec2(labelX, y));
        panel->addChild(label, 2);
        saveSlotLabels.push_back(label);

        auto loadBtn = createTextButton("Load", btnWidth, btnHeight,
            [this, slot](Ref* sender, Widget::TouchEventType type) {
                if (type == Widget::TouchEventType::ENDED) {
                    AudioManager::playButtonClick();
                    onSaveSlotSelected(slot);
                }
            });
        if (loadBtn) {
            loadBtn->setPosition(Vec2(loadX, y));
            panel->addChild(loadBtn, 2);
        }
        saveSlotLoadButtons.push_back(loadBtn);

        auto deleteBtn = createTextButton("Delete", btnWidth, btnHeight,
            [this, slot](Ref* sender, Widget::TouchEventType type) {
                if (type == Widget::TouchEventType::ENDED) {
                    AudioManager::playButtonCancel();
                    onSaveSlotDelete(slot);
                }
            });
        if (deleteBtn) {
            deleteBtn->setPosition(Vec2(deleteX, y));
            panel->addChild(deleteBtn, 2);
        }
        saveSlotDeleteButtons.push_back(deleteBtn);
    }

    auto returnBtn = createIconButton(
        "Return",
        "exit.png",
        CC_CALLBACK_2(MainMenuScene::onReturnTouch, this)
    );
    if (returnBtn) {
        returnBtn->setPosition(Vec2(0, -panelHeight / 2 + 60));
        panel->addChild(returnBtn, 2);
    }

    refreshSaveSlots();
}

void MainMenuScene::showSaveLayer()
{
    if (!saveLayer) {
        return;
    }
    refreshSaveSlots();
    switchToLayer(saveLayer);
}

void MainMenuScene::refreshSaveSlots()
{
    auto slots = SaveManager::getInstance()->listSlots();
    size_t count = std::min(slots.size(), saveSlotLabels.size());
    for (size_t i = 0; i < count; ++i) {
        const auto& info = slots[i];
        auto* label = saveSlotLabels[i];
        if (label) {
            std::string text = StringUtils::format("Slot %d: %s", info.slot, info.summary.c_str());
            label->setString(text);
        }

        auto* loadBtn = (i < saveSlotLoadButtons.size()) ? saveSlotLoadButtons[i] : nullptr;
        if (loadBtn) {
            loadBtn->setTitleText(info.exists ? "Load" : "New");
            loadBtn->setEnabled(true);
            loadBtn->setBright(true);
            loadBtn->setOpacity(255);
        }

        auto* deleteBtn = (i < saveSlotDeleteButtons.size()) ? saveSlotDeleteButtons[i] : nullptr;
        if (deleteBtn) {
            deleteBtn->setEnabled(info.exists);
            deleteBtn->setBright(info.exists);
            deleteBtn->setOpacity(info.exists ? 255 : 150);
        }
    }
}

void MainMenuScene::onSaveSlotSelected(int slot)
{
    auto* saveManager = SaveManager::getInstance();
    saveManager->setActiveSlot(slot);
    saveManager->loadSlot(slot);

    auto scene = BaseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
}

void MainMenuScene::onSaveSlotDelete(int slot)
{
    SaveManager::getInstance()->deleteSlot(slot);
    refreshSaveSlots();
}

void MainMenuScene::onReturnTouch(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        AudioManager::playButtonCancel();
        onReturn(sender);
    }
}

void MainMenuScene::onStartTouch(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        AudioManager::playButtonClick();
        onStart(sender);
    }
}

void MainMenuScene::onSettingsTouch(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        AudioManager::playButtonClick();
        onSettings(sender);
    }
}

void MainMenuScene::onRuleTouch(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        AudioManager::playButtonClick();
        onRule(sender);
    }
}

void MainMenuScene::onExitTouch(Ref* sender, ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        AudioManager::playButtonCancel();
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

void MainMenuScene::animateMenuButtons()
{
    if (menuButtons.empty()) {
        return;
    }

    float delayStep = 0.06f;
    float moveOffset = 18.0f;

    for (size_t i = 0; i < menuButtons.size(); ++i) {
        auto* button = menuButtons[i];
        if (!button) {
            continue;
        }
        button->stopAllActions();
        Vec2 originPos = button->getPosition();
        button->setOpacity(0);
        button->setPosition(originPos - Vec2(0.0f, moveOffset));
        auto move = EaseBackOut::create(MoveTo::create(0.25f, originPos));
        auto fade = FadeTo::create(0.2f, 255);
        auto spawn = Spawn::create(move, fade, nullptr);
        button->runAction(Sequence::create(DelayTime::create(delayStep * i), spawn, nullptr));
    }
}

void MainMenuScene::animatePanel(Node* panel)
{
    if (!panel) {
        return;
    }

    panel->stopAllActions();
    panel->setScale(0.94f);
    panel->runAction(EaseBackOut::create(ScaleTo::create(0.22f, 1.0f)));
}
