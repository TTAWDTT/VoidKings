// TrainPanel.cpp
// 兵种训练面板实现

#include "TrainPanel.h"
#include "Core/Core.h"

// ===================================================
// 创建与初始化
// ===================================================

TrainPanel* TrainPanel::create(
    const std::function<void(int)>& onTrainComplete,
    const std::function<void()>& onClose
) {
    TrainPanel* ret = new (std::nothrow) TrainPanel();
    if (ret && ret->init(onTrainComplete, onClose)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool TrainPanel::init(
    const std::function<void(int)>& onTrainComplete,
    const std::function<void()>& onClose
) {
    if (!Node::init()) {
        return false;
    }
    
    _onTrainComplete = onTrainComplete;
    _onClose = onClose;
    _isShowing = false;
    
    // 初始化可训练兵种列表（从UnitManager获取）
    // 这里硬编码了三种兵种ID，实际可以从配置读取
    _availableUnits = {101, 102, 103};  // Goblin, Barbarian, Archer
    
    // 初始化各个UI组件
    setupBackground();
    setupPanel();
    setupTitle();
    setupUnitButtons();
    setupQueueArea();
    setupCloseButton();
    setupResourceDisplay();
    
    // 初始隐藏
    this->setVisible(false);
    
    // 启用update
    this->scheduleUpdate();
    
    return true;
}

// ===================================================
// 背景设置 - 半透明遮罩覆盖全屏
// ===================================================
void TrainPanel::setupBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _background = LayerColor::create(
        Color4B(0, 0, 0, 180),
        visibleSize.width,
        visibleSize.height
    );
    _background->setPosition(origin);
    this->addChild(_background, 0);
}

// ===================================================
// 面板主体设置 - 居中显示的主面板
// ===================================================
void TrainPanel::setupPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _panel = LayerColor::create(
        Color4B(60, 60, 80, 255),
        TrainPanelConfig::PANEL_SIZE.width,
        TrainPanelConfig::PANEL_SIZE.height
    );
    
    // 居中定位
    float panelX = origin.x + (visibleSize.width - TrainPanelConfig::PANEL_SIZE.width) / 2;
    float panelY = origin.y + (visibleSize.height - TrainPanelConfig::PANEL_SIZE.height) / 2;
    _panel->setPosition(panelX, panelY);
    
    this->addChild(_panel, 1);
}

// ===================================================
// 标题设置
// ===================================================
void TrainPanel::setupTitle() {
    _titleLabel = Label::createWithSystemFont(
        "Unit Training",
        "Arial",
        TrainPanelConfig::TITLE_FONT_SIZE
    );
    _titleLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    _titleLabel->setPosition(
        TrainPanelConfig::PANEL_SIZE.width / 2,
        TrainPanelConfig::PANEL_SIZE.height - TrainPanelConfig::TITLE_TOP_OFFSET
    );
    _titleLabel->setColor(Color3B::WHITE);
    _panel->addChild(_titleLabel);
}

// ===================================================
// 兵种按钮区域设置
// ===================================================
void TrainPanel::setupUnitButtons() {
    _unitButtonArea = Node::create();
    _unitButtonArea->setPosition(Vec2(0, 0));
    _panel->addChild(_unitButtonArea);
    
    // 计算起始位置（水平居中排列）
    float totalWidth = _availableUnits.size() * TrainPanelConfig::UNIT_BUTTON_SIZE + 
                       (_availableUnits.size() - 1) * TrainPanelConfig::UNIT_BUTTON_SPACING;
    float startX = (TrainPanelConfig::PANEL_SIZE.width - totalWidth) / 2 + 
                   TrainPanelConfig::UNIT_BUTTON_SIZE / 2;
    float buttonY = TrainPanelConfig::PANEL_SIZE.height - TrainPanelConfig::UNIT_AREA_TOP_OFFSET - 
                    TrainPanelConfig::UNIT_BUTTON_SIZE / 2 - 30;
    
    for (size_t i = 0; i < _availableUnits.size(); ++i) {
        int unitId = _availableUnits[i];
        float buttonX = startX + i * (TrainPanelConfig::UNIT_BUTTON_SIZE + TrainPanelConfig::UNIT_BUTTON_SPACING);
        
        auto btn = createUnitButton(unitId, Vec2(buttonX, buttonY));
        if (btn) {
            _unitButtonArea->addChild(btn);
        }
    }
}

// ===================================================
// 创建单个兵种按钮
// ===================================================
Button* TrainPanel::createUnitButton(int unitId, const Vec2& position) {
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) {
        CCLOG("[训练面板] 未找到兵种配置: %d", unitId);
        return nullptr;
    }
    
    // 创建按钮容器
    auto btnNode = Node::create();
    btnNode->setPosition(position);
    
    // 创建按钮背景
    auto btn = Button::create("btn_normal.png", "btn_pressed.png");
    if (!btn) {
        // 如果按钮图片不存在，创建一个简单的色块按钮
        btn = Button::create();
        btn->setScale9Enabled(true);
        btn->setContentSize(Size(TrainPanelConfig::UNIT_BUTTON_SIZE, TrainPanelConfig::UNIT_BUTTON_SIZE));
    }
    btn->setScale(1.0f);
    btn->setPosition(position);
    
    // 设置按钮文字（兵种名称）
    btn->setTitleText(config->name);
    btn->setTitleFontSize(12);
    btn->setTitleColor(Color3B::WHITE);
    
    // 绑定点击事件
    btn->addClickEventListener([this, unitId](Ref* sender) {
        this->addToTrainQueue(unitId);
    });
    
    // 添加费用标签
    auto costLabel = Label::createWithSystemFont(
        std::to_string(config->COST_ELIXIR) + "E",
        "Arial",
        10
    );
    costLabel->setPosition(Vec2(
        btn->getContentSize().width / 2,
        -5
    ));
    costLabel->setColor(Color3B::YELLOW);
    btn->addChild(costLabel);
    
    return btn;
}

// ===================================================
// 训练队列区域设置
// ===================================================
void TrainPanel::setupQueueArea() {
    _queueArea = Node::create();
    
    // 队列区域位于面板中下部
    float queueY = TrainPanelConfig::QUEUE_AREA_HEIGHT + 50;
    _queueArea->setPosition(Vec2(20, queueY));
    
    _panel->addChild(_queueArea);
    
    // 队列标题
    auto queueTitle = Label::createWithSystemFont(
        "Training Queue:",
        "Arial",
        16
    );
    queueTitle->setAnchorPoint(Vec2(0, 0.5f));
    queueTitle->setPosition(Vec2(0, TrainPanelConfig::QUEUE_ITEM_SIZE / 2 + 30));
    queueTitle->setColor(Color3B::WHITE);
    _queueArea->addChild(queueTitle);
}

// ===================================================
// 关闭按钮设置
// ===================================================
void TrainPanel::setupCloseButton() {
    auto closeBtn = Button::create("btn_normal.png", "btn_pressed.png");
    if (!closeBtn) {
        closeBtn = Button::create();
    }
    closeBtn->setTitleText("Close");
    closeBtn->setTitleFontSize(20);
    closeBtn->setScale(1.2f);
    closeBtn->setPosition(Vec2(
        TrainPanelConfig::PANEL_SIZE.width / 2,
        TrainPanelConfig::CLOSE_BUTTON_BOTTOM
    ));
    closeBtn->addClickEventListener([this](Ref* sender) {
        this->hide();
        if (_onClose) {
            _onClose();
        }
    });
    _panel->addChild(closeBtn);
}

// ===================================================
// 资源显示设置
// ===================================================
void TrainPanel::setupResourceDisplay() {
    _resourceLabel = Label::createWithSystemFont(
        "",
        "Arial",
        14
    );
    _resourceLabel->setAnchorPoint(Vec2(1.0f, 1.0f));
    _resourceLabel->setPosition(Vec2(
        TrainPanelConfig::PANEL_SIZE.width - 20,
        TrainPanelConfig::PANEL_SIZE.height - 10
    ));
    _resourceLabel->setColor(Color3B::YELLOW);
    _panel->addChild(_resourceLabel);
    
    updateResourceDisplay();
}

// ===================================================
// 显示面板
// ===================================================
void TrainPanel::show() {
    this->setVisible(true);
    _isShowing = true;
    updateResourceDisplay();
    updateQueueDisplay();
}

// ===================================================
// 隐藏面板
// ===================================================
void TrainPanel::hide() {
    this->setVisible(false);
    _isShowing = false;
}

// ===================================================
// 每帧更新 - 处理训练队列
// ===================================================
void TrainPanel::update(float dt) {
    if (!_isShowing || _trainQueue.empty()) {
        return;
    }
    
    // 更新队列第一个兵种的训练时间
    TrainQueueItem& currentItem = _trainQueue.front();
    currentItem.remainingTime -= dt;
    
    // 检查是否训练完成
    if (currentItem.remainingTime <= 0) {
        // 训练完成，添加到已完成列表
        _trainedUnits[currentItem.unitId]++;
        
        CCLOG("[训练面板] 兵种训练完成: %s (ID: %d)", 
              currentItem.unitName.c_str(), currentItem.unitId);
        
        // 触发回调
        if (_onTrainComplete) {
            _onTrainComplete(currentItem.unitId);
        }
        
        // 从队列移除
        _trainQueue.erase(_trainQueue.begin());
        
        // 更新显示
        updateQueueDisplay();
    }
}

// ===================================================
// 添加兵种到训练队列
// ===================================================
void TrainPanel::addToTrainQueue(int unitId) {
    // 检查资源是否足够
    if (!canAffordUnit(unitId)) {
        CCLOG("[训练面板] 资源不足，无法训练兵种: %d", unitId);
        return;
    }
    
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) {
        return;
    }
    
    // 扣除资源
    Core::getInstance()->consumeResource(ResourceType::DIAMOND, config->COST_ELIXIR);
    
    // 创建队列项
    TrainQueueItem item;
    item.unitId = unitId;
    item.unitName = config->name;
    item.totalTime = static_cast<float>(config->TRAIN_TIME);
    item.remainingTime = item.totalTime;
    
    // 添加到队列
    _trainQueue.push_back(item);
    
    CCLOG("[训练面板] 添加兵种到训练队列: %s, 训练时间: %.1f秒", 
          config->name.c_str(), item.totalTime);
    
    // 更新显示
    updateQueueDisplay();
    updateResourceDisplay();
}

// ===================================================
// 从训练队列移除
// ===================================================
void TrainPanel::removeFromTrainQueue(int index) {
    if (index < 0 || index >= static_cast<int>(_trainQueue.size())) {
        return;
    }
    
    // 返还部分资源（可选功能）
    const TrainQueueItem& item = _trainQueue[index];
    const UnitConfig* config = UnitManager::getInstance()->getConfig(item.unitId);
    if (config) {
        // 返还50%资源
        int refund = config->COST_ELIXIR / 2;
        Core::getInstance()->addResource(ResourceType::DIAMOND, refund);
        CCLOG("[训练面板] 取消训练，返还资源: %d", refund);
    }
    
    _trainQueue.erase(_trainQueue.begin() + index);
    updateQueueDisplay();
    updateResourceDisplay();
}

// ===================================================
// 更新队列显示
// ===================================================
void TrainPanel::updateQueueDisplay() {
    // 清除旧的队列显示
    _queueArea->removeAllChildren();
    
    // 队列标题
    auto queueTitle = Label::createWithSystemFont(
        "Training Queue:",
        "Arial",
        16
    );
    queueTitle->setAnchorPoint(Vec2(0, 0.5f));
    queueTitle->setPosition(Vec2(0, TrainPanelConfig::QUEUE_ITEM_SIZE / 2 + 30));
    queueTitle->setColor(Color3B::WHITE);
    _queueArea->addChild(queueTitle);
    
    // 显示队列中的每个兵种
    for (size_t i = 0; i < _trainQueue.size() && i < 8; ++i) {
        auto itemNode = createQueueItemNode(_trainQueue[i], static_cast<int>(i));
        if (itemNode) {
            float itemX = i * (TrainPanelConfig::QUEUE_ITEM_SIZE + TrainPanelConfig::QUEUE_ITEM_SPACING);
            itemNode->setPosition(Vec2(itemX, 0));
            _queueArea->addChild(itemNode);
        }
    }
    
    // 显示已完成的兵种数量
    if (!_trainedUnits.empty()) {
        std::string trainedText = "Trained: ";
        for (const auto& pair : _trainedUnits) {
            const UnitConfig* config = UnitManager::getInstance()->getConfig(pair.first);
            if (config) {
                trainedText += config->name + "x" + std::to_string(pair.second) + " ";
            }
        }
        auto trainedLabel = Label::createWithSystemFont(trainedText, "Arial", 12);
        trainedLabel->setAnchorPoint(Vec2(0, 0.5f));
        trainedLabel->setPosition(Vec2(0, -30));
        trainedLabel->setColor(Color3B::GREEN);
        _queueArea->addChild(trainedLabel);
    }
}

// ===================================================
// 创建队列项显示节点
// ===================================================
Node* TrainPanel::createQueueItemNode(const TrainQueueItem& item, int index) {
    auto node = Node::create();
    
    // 背景
    auto bg = LayerColor::create(
        Color4B(80, 80, 100, 255),
        TrainPanelConfig::QUEUE_ITEM_SIZE,
        TrainPanelConfig::QUEUE_ITEM_SIZE
    );
    node->addChild(bg);
    
    // 兵种名称
    auto nameLabel = Label::createWithSystemFont(
        item.unitName.substr(0, 3),  // 只显示前3个字符
        "Arial",
        10
    );
    nameLabel->setPosition(Vec2(
        TrainPanelConfig::QUEUE_ITEM_SIZE / 2,
        TrainPanelConfig::QUEUE_ITEM_SIZE / 2 + 10
    ));
    node->addChild(nameLabel);
    
    // 剩余时间
    char timeStr[32];
    sprintf(timeStr, "%.0fs", item.remainingTime);
    auto timeLabel = Label::createWithSystemFont(timeStr, "Arial", 10);
    timeLabel->setPosition(Vec2(
        TrainPanelConfig::QUEUE_ITEM_SIZE / 2,
        TrainPanelConfig::QUEUE_ITEM_SIZE / 2 - 10
    ));
    timeLabel->setColor(Color3B::YELLOW);
    node->addChild(timeLabel);
    
    // 进度条背景
    auto progressBg = LayerColor::create(
        Color4B(40, 40, 40, 255),
        TrainPanelConfig::QUEUE_ITEM_SIZE - 10,
        6
    );
    progressBg->setPosition(Vec2(5, 5));
    node->addChild(progressBg);
    
    // 进度条
    float progress = 1.0f - (item.remainingTime / item.totalTime);
    float progressWidth = (TrainPanelConfig::QUEUE_ITEM_SIZE - 10) * progress;
    auto progressBar = LayerColor::create(
        Color4B(100, 200, 100, 255),
        progressWidth,
        6
    );
    progressBar->setPosition(Vec2(5, 5));
    node->addChild(progressBar);
    
    return node;
}

// ===================================================
// 更新资源显示
// ===================================================
void TrainPanel::updateResourceDisplay() {
    int coin = Core::getInstance()->getResource(ResourceType::COIN);
    int diamond = Core::getInstance()->getResource(ResourceType::DIAMOND);
    
    char buffer[64];
    sprintf(buffer, "金币: %d  钻石: %d", coin, diamond);
    _resourceLabel->setString(buffer);
}

// ===================================================
// 检查资源是否足够
// ===================================================
bool TrainPanel::canAffordUnit(int unitId) {
    const UnitConfig* config = UnitManager::getInstance()->getConfig(unitId);
    if (!config) {
        return false;
    }
    
    // 使用钻石作为训练资源（与COST_ELIXIR对应）
    int diamond = Core::getInstance()->getResource(ResourceType::DIAMOND);
    return diamond >= config->COST_ELIXIR;
}
