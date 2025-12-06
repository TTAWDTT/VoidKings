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

USING_NS_CC;

class MainMenuScene : public Scene
{
public:
	static Scene* createScene();
	virtual bool init() override;
	
	// 实现 create() 方法
	CREATE_FUNC(MainMenuScene);
	
	void onStart(Ref* sender);
	void onSettings(Ref* sender);
	void onRule(Ref* sender);
	void onReturn(Ref* sender);
	void onExit(Ref* sender);
protected:
	void createBackground();
	void createHeadLogo();
	void createOtherThings();
	void createMainMenuLayer();
	void createSettingsLayer();
	void createRuleLayer();
	void switchToLayer(Node* targetLayer);
private:
	Node* mainMenuLayer = nullptr;
	Node* settingsLayer = nullptr;
	Node* ruleLayer = nullptr;
	Sprite* background = nullptr;
	Sprite* otherThings = nullptr;
	Label* headLogo = nullptr;
	Menu* _menu = nullptr;
};

#endif // __MAIN_MENU_SCENE_H__