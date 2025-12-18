/**
 * @file BaseUIPanel.cpp
 * @brief 基地场景UI面板实现
 */

#include "BaseUIPanel.h"
#include "UI/IDCardPanel.h"

// ===================================================
// 创建与初始化
// ===================================================

BaseUIPanel* BaseUIPanel::create(const BaseUICallbacks& callbacks) {
    BaseUIPanel* ret = new (std::nothrow) BaseUIPanel();
    if (ret && ret->init(callbacks)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool BaseUIPanel::init(const BaseUICallbacks& callbacks) {
    if (!Node::init()) {
        return false;
    }
    
    _callbacks = callbacks;
    
    // 初始化UI组件
    setupButtons();
    setupResourcePanel();
    
    CCLOG("[基地UI面板] 初始化完成");
    
    return true;
}

// ===================================================
// 创建按钮面板
// ===================================================
void BaseUIPanel::setupButtons() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    float panelX = origin.x + BaseUIConfig::BUTTON_PANEL_X;
    float startY = visibleSize.height / 2 + 100;
    
    // 进攻按钮
    _attackButton = Button::create("UI/attack.png");
    if (_attackButton) {
        _attackButton->setScale(BaseUIConfig::BUTTON_SCALE);
        _attackButton->setPosition(Vec2(panelX, startY - BaseUIConfig::BUTTON_SPACING));
        _attackButton->addClickEventListener([this](Ref* sender) {
            if (_callbacks.onAttack) _callbacks.onAttack();
        });
        this->addChild(_attackButton);
        
        // 添加提示框
        auto attackTip = createTooltip("Attack", Size(80, 30));
        bindTooltip(_attackButton, attackTip);
    }
    
    // 建造按钮
    _buildButton = Button::create("UI/build.png");
    if (_buildButton) {
        _buildButton->setScale(BaseUIConfig::BUTTON_SCALE);
        _buildButton->setPosition(Vec2(panelX, startY - BaseUIConfig::BUTTON_SPACING * 2));
        _buildButton->addClickEventListener([this](Ref* sender) {
            if (_callbacks.onBuild) _callbacks.onBuild();
        });
        this->addChild(_buildButton);
        
        // 添加提示框
        auto buildTip = createTooltip("Build", Size(60, 30));
        bindTooltip(_buildButton, buildTip);
    }
    
    // 退出按钮
    _exitButton = Button::create("UI/exit.png");
    if (_exitButton) {
        _exitButton->setScale(BaseUIConfig::BUTTON_SCALE);
        _exitButton->setPosition(Vec2(panelX, startY - BaseUIConfig::BUTTON_SPACING * 3));
        _exitButton->addClickEventListener([this](Ref* sender) {
            if (_callbacks.onExit) _callbacks.onExit();
        });
        this->addChild(_exitButton);
        
        // 添加提示框
        auto exitTip = createTooltip("Exit", Size(60, 30));
        bindTooltip(_exitButton, exitTip);
    }
}

// ===================================================
// 创建资源显示面板
// ===================================================
void BaseUIPanel::setupResourcePanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    float panelX = origin.x + BaseUIConfig::BUTTON_PANEL_X;
    float startY = visibleSize.height / 2 + 100;
    
    // 使用IDCardPanel显示资源
    _idCardPanel = IDCardPanel::createPanel(this);
    if (_idCardPanel) {
        _idCardPanel->setPosition(Vec2(panelX + 50, startY + 20));
    }
}

// ===================================================
// 创建提示框
// ===================================================
Node* BaseUIPanel::createTooltip(const std::string& text, const Size& size) {
    auto panel = Node::create();
    panel->setContentSize(size);
    panel->setAnchorPoint(Vec2(0, 0.5f));
    
    // 背景
    auto bg = LayerColor::create(Color4B(0, 0, 0, 180), size.width, size.height);
    bg->setPosition(Vec2::ZERO);
    panel->addChild(bg);
    
    // 文字
    auto label = Label::createWithTTF(text, "fonts/ScienceGothic.ttf", BaseUIConfig::TOOLTIP_FONT_SIZE);
    label->setWidth(size.width - 10);
    label->setAlignment(TextHAlignment::LEFT);
    label->setPosition(Vec2(size.width / 2, size.height / 2));
    panel->addChild(label);
    
    return panel;
}

// ===================================================
// 绑定提示框到按钮
// ===================================================
void BaseUIPanel::bindTooltip(Node* targetBtn, Node* tooltip, float offsetX) {
    if (!targetBtn || !tooltip) return;
    
    tooltip->setVisible(false);
    this->addChild(tooltip, 999);
    
    // 创建鼠标事件监听器
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = [=](EventMouse* event) {
        Vec2 mouseWorldPos(event->getCursorX(), event->getCursorY());
        
        // 检查鼠标是否在按钮范围内
        if (targetBtn->getBoundingBox().containsPoint(mouseWorldPos)) {
            float btnRight = targetBtn->getPositionX() +
                targetBtn->getContentSize().width * targetBtn->getScale() / 2;
            tooltip->setPosition(Vec2(btnRight + offsetX, targetBtn->getPositionY()));
            tooltip->setVisible(true);
        } else {
            tooltip->setVisible(false);
        }
    };
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

// ===================================================
// 更新资源显示
// ===================================================
void BaseUIPanel::updateResourceDisplay(int gold, int diamond) {
    // 资源显示已经由IDCardPanel管理
    // 如果需要更新，可以在这里添加逻辑
    if (_goldLabel) {
        char buffer[64];
        sprintf(buffer, "Gold: %d", gold);
        _goldLabel->setString(buffer);
    }
    if (_diamondLabel) {
        char buffer[64];
        sprintf(buffer, "Diamond: %d", diamond);
        _diamondLabel->setString(buffer);
    }
}
