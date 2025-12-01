// MainMenuScene.h
// 主菜单场景
// 功能: 游戏入口界面，提供开始游戏、设置等选项

#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"
#include "../Core/GameDefines.h"

USING_NS_CC;

/**
 * @class MainMenuScene
 * @brief 主菜单场景类
 * 
 * 主界面功能:
 * - 游戏标题显示
 * - 开始游戏按钮
 * - 设置按钮
 * - 退出按钮
 */
class MainMenuScene : public Scene {
public:
    static Scene* createScene();
    
    virtual bool init() override;
    
    // 开始游戏
    void onStartGame(Ref* sender);
    
    // 打开设置
    void onSettings(Ref* sender);
    
    // 退出游戏
    void onExit(Ref* sender);
    
    // 帧更新
    virtual void update(float dt) override;
    
    CREATE_FUNC(MainMenuScene);

protected:
    // 创建背景
    void createBackground();
    
    // 创建标题
    void createTitle();
    
    // 创建菜单按钮
    void createMenuButtons();
    
    // 播放背景动画
    void playBackgroundAnimation();
    
    // 监听窗口大小变化
    void onWindowResize(float width, float height);
    
private:
    Node* _background;
    Label* _titleLabel;
    Menu* _menu;
};

#endif // __MAIN_MENU_SCENE_H__
