/*
1.label
Background
Head Logo
Other Things
2.button
Start -> 切换到base
Settings -> 挂出label，内有button
Rule -> 挂出label，内有button（only Exit）
Exit -> 退出
*/

#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class MainMenuScene : public Scene
{
public:
	static Scene* createScene();
	virtual bool init() override;
	
	// 实现 create() 方法
	CREATE_FUNC(MainMenuScene);
	
	void onStart(Ref* sender);
	void onStartTouch(Ref* sender, ui::Widget::TouchEventType type);
	void onSettings(Ref* sender);
	void onSettingsTouch(Ref* sender, ui::Widget::TouchEventType type);
	void onRule(Ref* sender);
	void onRuleTouch(Ref* sender, ui::Widget::TouchEventType type);
	void onReturn(Ref* sender);
	void onReturnTouch(Ref* sender, ui::Widget::TouchEventType type);
	void onExit(Ref* sender);
	void onExitTouch(Ref* sender, ui::Widget::TouchEventType type);
protected:
	void createBackground();
	void switchBackground(float dt);
	void createHeadLogo();
	void createOtherThings();
	void createMainMenuLayer();
	void createSettingsLayer();
	void createRuleLayer();
	void switchToLayer(Node* targetLayer);
	Button* createIconButton(
		const std::string& title,
		const std::string& iconPath,
		const Widget::ccWidgetTouchCallback& callback);
private:
	Node* mainMenuLayer = nullptr;
	Node* settingsLayer = nullptr;
	Node* ruleLayer = nullptr;
	Sprite* background = nullptr;
	Sprite* otherThings = nullptr;
	Label* headLogo = nullptr;
	Menu* _menu = nullptr;
	const float buttonWidth = 150;
	const float buttonHeight = 30;
	const float buttonSpacing = 50;
	const float paddingX = 40;
	const float paddingY = 10;
	Vector<Sprite*> backgroundList;
	int currentBgIndex = 0;
};

#endif // __MAIN_MENU_SCENE_H__