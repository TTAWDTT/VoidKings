/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
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

// AppDelegate.cpp
// 应用程序代理类
// 功能: 管理应用程序生命周期，初始化游戏窗口和场景
// 修改说明: 
// - 调整设计分辨率为1280x720以适配游戏界面
// - 启用窗口大小调整功能
// - 默认启动主菜单场景

#include "AppDelegate.h"
#include "Scenes/MainMenuScene.h"

// #define USE_AUDIO_ENGINE 1
// #define USE_SIMPLE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE && USE_SIMPLE_AUDIO_ENGINE
#error "Don't use AudioEngine and SimpleAudioEngine at the same time. Please just select one in your game!"
#endif

#if USE_AUDIO_ENGINE
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#elif USE_SIMPLE_AUDIO_ENGINE
#include "audio/include/SimpleAudioEngine.h"
using namespace CocosDenshion;
#endif

USING_NS_CC;

// 设计分辨率 - 游戏画面的基准尺寸
static cocos2d::Size designResolutionSize = cocos2d::Size(1280, 720);
// 小屏幕分辨率
static cocos2d::Size smallResolutionSize = cocos2d::Size(960, 540);
// 中等屏幕分辨率
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1280, 720);
// 大屏幕分辨率
static cocos2d::Size largeResolutionSize = cocos2d::Size(1920, 1080);

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate() 
{
#if USE_AUDIO_ENGINE
    AudioEngine::end();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::end();
#endif
}

// 初始化OpenGL上下文属性
// 设置颜色深度、深度缓冲、模板缓冲等参数
void AppDelegate::initGLContextAttrs()
{
    // 设置OpenGL上下文属性: 红,绿,蓝,透明度,深度,模板,多重采样数
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};

    GLView::setGLContextAttrs(glContextAttrs);
}

// 包管理器注册函数
// 如果需要使用包管理器安装更多包，不要修改或删除此函数
static int register_all_packages()
{
    return 0; // 包管理器标志
}

bool AppDelegate::applicationDidFinishLaunching() {
    // 初始化导演类
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    
    if(!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        // Win32/Mac/Linux平台: 创建可调整大小的窗口
        // 参数: 窗口标题, 窗口矩形区域, 缩放因子, 是否可调整大小
        glview = GLViewImpl::createWithRect("Void Kings - Clash of Clans Style Game", 
                                            cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height),
                                            1.0f,
                                            true);  // 启用窗口大小调整
#else
        glview = GLViewImpl::create("Void Kings");
#endif
        director->setOpenGLView(glview);
    }

    // 开启FPS显示(调试用)
    director->setDisplayStats(true);

    // 设置帧率为60FPS
    director->setAnimationInterval(1.0f / 60);

    // 设置设计分辨率
    // 使用SHOW_ALL策略确保所有内容可见，同时保持宽高比
    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::SHOW_ALL);
    
    auto frameSize = glview->getFrameSize();
    
    // 根据实际窗口大小设置内容缩放因子
    if (frameSize.height > mediumResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(largeResolutionSize.height/designResolutionSize.height, 
                                           largeResolutionSize.width/designResolutionSize.width));
    }
    else if (frameSize.height > smallResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(mediumResolutionSize.height/designResolutionSize.height, 
                                           mediumResolutionSize.width/designResolutionSize.width));
    }
    else
    {        
        director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, 
                                           smallResolutionSize.width/designResolutionSize.width));
    }

    // 注册所有包
    register_all_packages();

    // 创建并运行主菜单场景
    auto scene = MainMenuScene::createScene();
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
    SimpleAudioEngine::getInstance()->pauseAllEffects();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    SimpleAudioEngine::getInstance()->resumeAllEffects();
#endif
}
