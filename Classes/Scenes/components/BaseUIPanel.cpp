/**
 * @file BaseUIPanel.cpp
 * @brief 基地场景UI面板实现
 */

#include "BaseUIPanel.h"
#include "UI/IDCardPanel.h"
#include "Utils/AudioManager.h"
#include <memory>

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

    float scale = BaseUIConfig::BUTTON_SCALE;
    float spacing = BaseUIConfig::BUTTON_SPACING;

    auto applyPressStyle = [](Button* button) {
        if (!button) return;
        button->setPressedActionEnabled(true);
        button->setZoomScale(0.05f);
        button->setSwallowTouches(true);
    };

    // 按钮组整体居中，左侧留边距
    float buttonX = origin.x + BaseUIConfig::EDGE_MARGIN;
    float startY = origin.y + visibleSize.height / 2;
    float btnHeight = 0.0f;

    _attackButton = Button::create("UI/attack.png");
    if (_attackButton) {
        Size btnSize = _attackButton->getContentSize();
        float btnWidth = btnSize.width * scale;
        btnHeight = btnSize.height * scale;

        // 计算三按钮整体高度，保持等距
        float totalHeight = btnHeight * 3 + spacing * 2;
        startY = origin.y + visibleSize.height / 2 + totalHeight / 2 - btnHeight / 2;
        buttonX = origin.x + BaseUIConfig::EDGE_MARGIN + btnWidth / 2;

        _attackButton->setScale(scale);
        _attackButton->setPosition(Vec2(buttonX, startY));
        applyPressStyle(_attackButton);
        _attackButton->addClickEventListener([this](Ref* sender) {
            AudioManager::playButtonClick();
            if (_callbacks.onAttack) _callbacks.onAttack();
            });
        this->addChild(_attackButton);

        auto attackTip = createTooltip("Attack", Size(80, 30));
        bindTooltip(_attackButton, attackTip);
        bindHoverEffect(_attackButton);
    }

    _buildButton = Button::create("UI/build.png");
    if (_buildButton) {
        _buildButton->setScale(scale);
        _buildButton->setPosition(Vec2(buttonX, startY - (btnHeight + spacing)));
        applyPressStyle(_buildButton);
        _buildButton->addClickEventListener([this](Ref* sender) {
            AudioManager::playButtonClick();
            if (_callbacks.onBuild) _callbacks.onBuild();
            });
        this->addChild(_buildButton);

        auto buildTip = createTooltip("Build", Size(60, 30));
        bindTooltip(_buildButton, buildTip);
        bindHoverEffect(_buildButton);
    }

    _exitButton = Button::create("UI/exit.png");
    if (_exitButton) {
        _exitButton->setScale(scale);
        _exitButton->setPosition(Vec2(buttonX, startY - 2 * (btnHeight + spacing)));
        applyPressStyle(_exitButton);
        _exitButton->addClickEventListener([this](Ref* sender) {
            AudioManager::playButtonCancel();
            if (_callbacks.onExit) _callbacks.onExit();
            });
        this->addChild(_exitButton);

        auto exitTip = createTooltip("Exit", Size(60, 30));
        bindTooltip(_exitButton, exitTip);
        bindHoverEffect(_exitButton);
    }
}



// ===================================================
// 创建资源显示面板
// ===================================================
// 统一控制主按钮可用状态
void BaseUIPanel::setButtonsEnabled(bool enabled) {
    auto applyState = [enabled](Button* button) {
        if (!button) return;
        button->setEnabled(enabled);
        button->setBright(enabled);
        button->setOpacity(enabled ? 255 : 160);
    };

    applyState(_attackButton);
    applyState(_buildButton);
    applyState(_exitButton);
}

void BaseUIPanel::setupResourcePanel() {
    _idCardPanel = IDCardPanel::createPanel(this);
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

        // 按钮禁用时不显示提示
        auto widget = dynamic_cast<Widget*>(targetBtn);
        if (widget && !widget->isEnabled()) {
            tooltip->setVisible(false);
            return;
        }


        // 检查鼠标是否在按钮范围内
        if (targetBtn->getBoundingBox().containsPoint(mouseWorldPos)) {
            float btnRight = targetBtn->getPositionX() +
                targetBtn->getContentSize().width * targetBtn->getScale() / 2;
            tooltip->setPosition(Vec2(btnRight + offsetX, targetBtn->getPositionY()));
            tooltip->setVisible(true);
        }
        else {
            tooltip->setVisible(false);
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

// ===================================================
// 按钮悬停效果
// ===================================================
void BaseUIPanel::bindHoverEffect(Button* button) {
    if (!button) {
        return;
    }

    const float baseScale = button->getScale();
    const Color3B baseColor = button->getColor();
    auto hoverState = std::make_shared<bool>(false);

    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = [this, button, baseScale, baseColor, hoverState](EventMouse* event) {
        if (!button->isVisible()) {
            return;
        }
        auto widget = dynamic_cast<Widget*>(button);
        if (widget && !widget->isEnabled()) {
            if (*hoverState) {
                button->setScale(baseScale);
                button->setColor(baseColor);
                *hoverState = false;
            }
            return;
        }

        Vec2 mouseWorldPos(event->getCursorX(), event->getCursorY());
        Node* parent = button->getParent();
        Vec2 localPos = parent ? parent->convertToNodeSpace(mouseWorldPos) : mouseWorldPos;
        bool inside = button->getBoundingBox().containsPoint(localPos);

        if (inside && !*hoverState) {
            button->setScale(baseScale * 1.03f);
            button->setColor(Color3B::WHITE);
            *hoverState = true;
        }
        else if (!inside && *hoverState) {
            button->setScale(baseScale);
            button->setColor(baseColor);
            *hoverState = false;
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, button);
}

// ===================================================
// 更新资源显示
// ===================================================
void BaseUIPanel::updateResourceDisplay(int gold, int diamond) {
    if (!_idCardPanel) {
        return;
    }

    auto goldPanel = _idCardPanel->getChildByName("goldPanel");
    auto diamondPanel = _idCardPanel->getChildByName("diamondPanel");
    auto goldLabel = goldPanel ? dynamic_cast<Label*>(goldPanel->getChildByName("valueLabel")) : nullptr;
    auto diamondLabel = diamondPanel ? dynamic_cast<Label*>(diamondPanel->getChildByName("valueLabel")) : nullptr;

    if (goldLabel) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Gold: %d", gold);
        goldLabel->setString(buffer);
    }
    if (diamondLabel) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Diamonds: %d", diamond);
        diamondLabel->setString(buffer);
    }
}