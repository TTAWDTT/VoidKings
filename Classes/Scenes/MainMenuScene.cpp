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

    background = Sprite::create("background.png");
    if (!background) return;

    background->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2));

    Size bgSize = background->getContentSize();

    float scaleX = visibleSize.width / bgSize.width;
    float scaleY = visibleSize.height / bgSize.height;
 
    float finalScale = std::max(scaleX, scaleY);

    background->setScale(finalScale);

    this->addChild(background);   
}


void MainMenuScene::createHeadLogo()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    headLogo = Label::createWithTTF("Void Kings", "fonts/Marker Felt.ttf", 48);
    headLogo->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - headLogo->getContentSize().height));
    this->addChild(headLogo, 1);
}

void MainMenuScene::createOtherThings()
{
	// 预留扩展
}

void MainMenuScene::createMainMenuLayer() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    mainMenuLayer = Node::create();
    this->addChild(mainMenuLayer, 10);

    float startY = visibleSize.height * 0.6f;

    Vector<MenuItem*> menuItems;

    // 开始游戏按钮
    auto startLabel = Label::createWithSystemFont("Start Game", "Arial", 25);
    startLabel->setColor(Color3B::WHITE);
    startLabel->setAnchorPoint(Vec2(0.5f, 0.5f));

    auto startButton = MenuItemLabel::create(startLabel, CC_CALLBACK_1(MainMenuScene::onStart, this));

    // 创建按钮背景
    auto startBg = DrawNode::create();
    startBg->drawSolidRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
        Vec2(buttonWidth / 2, buttonHeight / 2),
        Color4F(0.3f, 0.3f, 0.5f, 0.9f));
    startBg->drawRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
        Vec2(buttonWidth / 2, buttonHeight / 2),
        Color4F(1.0f, 0.84f, 0.0f, 1.0f));

    startBg->setPosition(startButton->getContentSize().width / 2,
        startButton->getContentSize().height / 2 );
    
    startButton->addChild(startBg, -1);
    startButton->setPosition(Vec2(visibleSize.width / 2, startY));
    menuItems.pushBack(startButton);

    // 设置按钮
    auto settingsLabel = Label::createWithSystemFont("Settings", "Arial", 25);
    settingsLabel->setColor(Color3B::WHITE);
    settingsLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    auto settingsButton = MenuItemLabel::create(settingsLabel, CC_CALLBACK_1(MainMenuScene::onSettings, this));

    auto settingsBg = DrawNode::create();
    settingsBg->drawSolidRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
        Vec2(buttonWidth / 2, buttonHeight / 2),
        Color4F(0.3f, 0.3f, 0.5f, 0.9f));
    settingsBg->drawRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
        Vec2(buttonWidth / 2, buttonHeight / 2),
        Color4F(0.6f, 0.6f, 0.8f, 1.0f));
    settingsBg->setPosition(settingsButton->getContentSize().width / 2,
        settingsButton->getContentSize().height / 2);
    
    settingsButton->addChild(settingsBg, -1);
    settingsButton->setPosition(Vec2(visibleSize.width / 2, startY - buttonSpacing));
    menuItems.pushBack(settingsButton);
	
    // 规则按钮
	auto ruleLabel = Label::createWithSystemFont("Rule", "Arial", 25);
	ruleLabel->setColor(Color3B::WHITE);
	ruleLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
	auto ruleButton = MenuItemLabel::create(ruleLabel, CC_CALLBACK_1(MainMenuScene::onRule, this));

	auto ruleBg = DrawNode::create();
	ruleBg->drawSolidRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
		Vec2(buttonWidth / 2, buttonHeight / 2),
		Color4F(0.3f, 0.5f, 0.3f, 0.9f));
	ruleBg->drawRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
		Vec2(buttonWidth / 2, buttonHeight / 2),
		Color4F(0.6f, 0.8f, 0.6f, 1.0f));
	ruleBg->setPosition(ruleButton->getContentSize().width / 2,
		ruleButton->getContentSize().height / 2);

	ruleButton->addChild(ruleBg, -1);
	ruleButton->setPosition(Vec2(visibleSize.width / 2, startY - buttonSpacing * 2));
	menuItems.pushBack(ruleButton);

    // 退出按钮
    auto exitLabel = Label::createWithSystemFont("Exit", "Arial", 25);
    exitLabel->setColor(Color3B::WHITE);
    exitLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    auto exitButton = MenuItemLabel::create(exitLabel, CC_CALLBACK_1(MainMenuScene::onExit, this));

    auto exitBg = DrawNode::create();
    exitBg->drawSolidRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
        Vec2(buttonWidth / 2, buttonHeight / 2),
        Color4F(0.6f, 0.2f, 0.2f, 0.9f));
    exitBg->drawRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
        Vec2(buttonWidth / 2, buttonHeight / 2),
        Color4F(0.8f, 0.4f, 0.4f, 1.0f));
    exitBg->setPosition(exitButton->getContentSize().width / 2,
        exitButton->getContentSize().height / 2);

    exitButton->addChild(exitBg, -1);
    exitButton->setPosition(Vec2(visibleSize.width / 2, startY - buttonSpacing * 3));
    menuItems.pushBack(exitButton);

    auto menu = Menu::createWithArray(menuItems);
    menu->setPosition(Vec2::ZERO);
    mainMenuLayer->addChild(menu);

    auto versionLabel = Label::createWithSystemFont("Version 1.0.0", "Arial", 16);
    versionLabel->setPosition(Vec2(visibleSize.width - 80, 20));
    versionLabel->setColor(Color3B(150, 150, 150));
    mainMenuLayer->addChild(versionLabel);
}

void MainMenuScene::onStart(Ref* sender) {
    // TODO: 待实现 BaseScene 后取消注释
    // auto scene = BaseScene::createScene();
    // Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
    
    // 临时提示
    CCLOG("Start Game clicked - BaseScene not yet implemented");
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

    auto label = Label::createWithSystemFont("Settings Page", "Arial", 25);
    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.7f));
    settingsLayer->addChild(label);

    auto returnLabel = Label::createWithSystemFont("Return", "Arial", 25);
    returnLabel->setColor(Color3B::WHITE);
    returnLabel->setAnchorPoint(Vec2(0.5f, 0.5f));

    auto returnButton = MenuItemLabel::create(returnLabel, CC_CALLBACK_1(MainMenuScene::onReturn, this));

    auto returnBg = DrawNode::create();
    returnBg->drawSolidRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
        Vec2(buttonWidth / 2, buttonHeight / 2),
        Color4F(0.6f, 0.2f, 0.2f, 0.9f));
    returnBg->drawRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
        Vec2(buttonWidth / 2, buttonHeight / 2),
        Color4F(0.8f, 0.4f, 0.4f, 1.0f));
    returnBg->setPosition(returnButton->getContentSize().width / 2,
        returnButton->getContentSize().height / 2);

    returnButton->addChild(returnBg, -1);
    returnButton->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.2f));

    auto menu = Menu::create(returnButton, nullptr);
    menu->setPosition(Vec2::ZERO);
    settingsLayer->addChild(menu);
}

void MainMenuScene::onSettings(Ref* sender)
{
    switchToLayer(settingsLayer);
}

void MainMenuScene::createRuleLayer()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    ruleLayer = Node::create();
    this->addChild(ruleLayer, 20);

    auto label = Label::createWithSystemFont("Rules", "Arial", 25);

    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.7f));
    ruleLayer->addChild(label);

    auto returnLabel = Label::createWithSystemFont("Return", "Arial", 25);
    returnLabel->setColor(Color3B::WHITE);
    returnLabel->setAnchorPoint(Vec2(0.5f, 0.5f));

    auto returnButton = MenuItemLabel::create(returnLabel, CC_CALLBACK_1(MainMenuScene::onReturn, this));

    auto returnBg = DrawNode::create();
    returnBg->drawSolidRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
        Vec2(buttonWidth / 2, buttonHeight / 2),
        Color4F(0.6f, 0.2f, 0.2f, 0.9f));
    returnBg->drawRect(Vec2(-buttonWidth / 2, -buttonHeight / 2),
        Vec2(buttonWidth / 2, buttonHeight / 2),
        Color4F(0.8f, 0.4f, 0.4f, 1.0f));
    returnBg->setPosition(returnButton->getContentSize().width / 2,
        returnButton->getContentSize().height / 2);

    returnButton->addChild(returnBg, -1);
    returnButton->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.2f));

    auto menu = Menu::create(returnButton, nullptr);
    menu->setPosition(Vec2::ZERO);
    ruleLayer->addChild(menu);
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

