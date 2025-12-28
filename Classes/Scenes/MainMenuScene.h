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
#include <vector>

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
	void createModalOverlay();
	void createMainMenuLayer();
	void createSettingsLayer();
	void createRuleLayer();
	void createSaveLayer();
	void switchToLayer(Node* targetLayer);
	void animateMenuButtons();
	void animatePanel(Node* panel);
	void startDinoIdle();
	void onDinoTapped();
	void triggerDinoEasterEgg();
	void showDinoToast(const std::string& text);
	Button* createIconButton(
		const std::string& title,
		const std::string& iconPath,
		const Widget::ccWidgetTouchCallback& callback);
	Button* createTextButton(
		const std::string& title,
		float width,
		float height,
		const Widget::ccWidgetTouchCallback& callback);
	void showSaveLayer();
	void refreshSaveSlots();
	void onSaveSlotSelected(int slot);
	void onSaveSlotDelete(int slot);
private:
	Node* mainMenuLayer = nullptr;
	Node* settingsLayer = nullptr;
	Node* ruleLayer = nullptr;
	Node* saveLayer = nullptr;
	LayerColor* modalOverlay = nullptr;
	Node* settingsPanel = nullptr;
	Node* rulePanel = nullptr;
	Node* savePanel = nullptr;
	Label* headLogo = nullptr;
	Sprite* _dinosaurSprite = nullptr;
	Vec2 _dinoBasePos = Vec2::ZERO;
	float _dinoBaseScale = 1.0f;
	int _dinoTapCount = 0;
	bool _dinoEasterActive = false;
	std::vector<Button*> menuButtons;
	Vector<Sprite*> backgroundList;
	int currentBgIndex = 0;
	std::vector<Label*> saveSlotLabels;
	std::vector<Button*> saveSlotLoadButtons;
	std::vector<Button*> saveSlotDeleteButtons;
};

#endif // __MAIN_MENU_SCENE_H__
