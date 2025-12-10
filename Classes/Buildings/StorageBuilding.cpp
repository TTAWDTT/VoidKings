// StorageBuilding.cpp
#include "StorageBuilding.h"

// 从config中创建
StorageBuilding* StorageBuilding::create(const StorageBuildingConfig* config, int level) {
    StorageBuilding* pRet = new(std::nothrow) StorageBuilding(); // 在堆内存（Heap）中申请一块内存来存放 StorageBuilding 对象 - nothrow会安全一点,内存不够时返回nullptr,而不是直接崩溃
    if (pRet && pRet->init(config, level)) { // 调用init进行初始化
        pRet->autorelease(); // 意思为：这一帧结束时,如果没有被addChild,就自动滚
        return pRet;
    }
    delete pRet;
    return nullptr; // 没内存就滚
}

bool StorageBuilding::init(const StorageBuildingConfig* config, int level) {
    // 底层数据初始化
    if (!Node::init()) return false;

    // 1. 绑定配置
    _config = config; // 这个写指针绑定,而不是拷贝,节约内存

    // 2. 设置等级并验证
    _level = level;
    if (_level < 0) _level = 0;
    if (_level > _config->MAXLEVEL) _level = _config->MAXLEVEL;
    
    // 3. 初始化运行时状态 - 使用level索引访问属性
    _currentHP = getCurrentMaxHP();
    _currentDP = getCurrentDP();
    _currentADD_STORAGE_ELIXIR_CAPACITY = getCurrentADD_STORAGE_ELIXIR_CAPACITY();
    _currentADD_STORAGE_GOLD_CAPACITY = getCurrentADD_STORAGE_GOLD_CAPACITY();
    // 4. 获得占地尺寸
    _length = getLength();
    _width = getWidth();

    // 5. 创建外观 (根据配置里的图片路径)
    _bodySprite = cocos2d::Sprite::createWithSpriteFrameName(_config->spriteFrameName);
    this->addChild(_bodySprite); // 这个大概就体现了逻辑实体和显示实体的组合
    // 6. 开启 Update 循环
    this->scheduleUpdate();

    return true;
}

void StorageBuilding::setLevel(int level) {
    if (level < 0) level = 0;
    if (level > _config->MAXLEVEL) level = _config->MAXLEVEL;
    // 保存血量百分比
    float hpPercent = _currentHP / getCurrentMaxHP();
    _level = level;

}

float StorageBuilding::getCurrentMaxHP() const {
    if (_level >= 0 && _level < _config->HP.size()) {
        return _config->HP[_level];
    }
    return _config->HP.empty() ? 0.0f : _config->HP[0];
}
    

float StorageBuilding::getCurrentDP() const {
    if (_level >= 0 && _level < _config->DP.size()) {
        return _config->DP[_level];
    }
    return _config->DP.empty() ? 0.0f : _config->DP[0];
}

float StorageBuilding::getCurrentADD_STORAGE_ELIXIR_CAPACITY() const {
    if (_level >= 0 && _level < _config->ADD_STORAGE_ELIXIR_CAPACITY.size()) {
        return _config->ADD_STORAGE_ELIXIR_CAPACITY[_level];
    }
    return _config->ADD_STORAGE_ELIXIR_CAPACITY.empty() ? 0.0f : _config->ADD_STORAGE_ELIXIR_CAPACITY[0];
}

float StorageBuilding::getCurrentADD_STORAGE_GOLD_CAPACITY() const {
    if (_level >= 0 && _level < _config->ADD_STORAGE_GOLD_CAPACITY.size()) {
        return _config->ADD_STORAGE_GOLD_CAPACITY[_level];
    }
    return _config->ADD_STORAGE_GOLD_CAPACITY.empty() ? 0.0f : _config->ADD_STORAGE_GOLD_CAPACITY[0];
}
float StorageBuilding::getCurrentHP() const {
    return _currentHP;
}
int StorageBuilding::getLength() const {
    return _config->length;
}
int StorageBuilding::getWidth() const {
    return _config->width;
}

void StorageBuilding::update(float dt) {
    levelup();//升级，未实现
}


void StorageBuilding::takeDamage(float damage) {
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;
    
    // 死亡处理
    if (_currentHP <= 0) {
        // 处理死亡逻辑
    }
}

void StorageBuilding::updateHealthBar(bool animate) {
    if (!_healthBar || !_config) return;

    float maxHP = getCurrentMaxHP();
    float pct = 0.0f;
    if (maxHP > 0.00001f) {
        pct = _currentHP / maxHP;
    }
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;

    float targetScaleX = pct;

    if (animate) {
        _healthBar->stopAllActions();
        auto action = cocos2d::ScaleTo::create(0.12f, targetScaleX, 1.0f);
        _healthBar->runAction(action);
    }
    else {
        _healthBar->setScaleX(targetScaleX);
    }

    if (pct > 0.5f) {
        _healthBar->setColor(cocos2d::Color3B::GREEN);
    }
    else if (pct > 0.2f) {
        _healthBar->setColor(cocos2d::Color3B::YELLOW);
    }
    else {
        _healthBar->setColor(cocos2d::Color3B::RED);
    }

    if (pct <= 0.0f) {
        _healthBar->setVisible(false);
    }
    else {
        _healthBar->setVisible(true);
    }
}

void StorageBuilding::stopCurrentAnimation() {
    if (!_bodySprite) return;
    _bodySprite->stopAllActions();
    _currentActionKey.clear();
}