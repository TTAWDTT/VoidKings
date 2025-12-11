#include "ProductionBuilding.h"
#include "Utils/AnimationUtils.h"

USING_NS_CC;

ProductionBuilding* ProductionBuilding::create(const ProductionBuildingConfig* config, int level) {
    ProductionBuilding* pRet = new(std::nothrow) ProductionBuilding();
    if (pRet && pRet->init(config, level)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool ProductionBuilding::init(const ProductionBuildingConfig* config, int level) {
    if (!Node::init()) return false;

    _config = config;
    _level = level;
    if (_level < 0) _level = 0;
    if (_level > _config->MAXLEVEL) _level = _config->MAXLEVEL;
    
    _currentHP = getCurrentMaxHP();
    _lastProduceTime = 0.0f;
    _currentActionKey.clear();

    _bodySprite = Sprite::create(_config->spriteFrameName);
    if (_bodySprite) {
        this->addChild(_bodySprite);
    }

    _healthBar = Sprite::create("res/health_bar.png");
    if (_healthBar) {
        _healthBar->setAnchorPoint(Vec2(0.0f, 0.5f));
        float offsetY = 30.0f;
        if (_bodySprite) {
            float y = _bodySprite->getContentSize().height * 0.5f + offsetY;
            float x = -_healthBar->getContentSize().width * 0.5f;
            _healthBar->setPosition(Vec2(x, y));
        }
        this->addChild(_healthBar);
        _healthBar->setScaleX(1.0f);
        _healthBar->setColor(Color3B::GREEN);
    }

    updateHealthBar(false);
    this->scheduleUpdate();

    return true;
}

void ProductionBuilding::setLevel(int level) {
    if (level < 0) level = 0;
    if (level > _config->MAXLEVEL) level = _config->MAXLEVEL;
    _level = level;
}

float ProductionBuilding::getCurrentMaxHP() const {
    if (_level >= 0 && _level < _config->HP.size()) {
        return _config->HP[_level];
    }
    return _config->HP.empty() ? 0.0f : _config->HP[0];
}

float ProductionBuilding::getCurrentDP() const {
    if (_level >= 0 && _level < _config->DP.size()) {
        return _config->DP[_level];
    }
    return _config->DP.empty() ? 0.0f : _config->DP[0];
}

float ProductionBuilding::getCurrentPRODUCE_ELIXIR() const {
    if (_level >= 0 && _level < _config->PRODUCE_ELIXIR.size()) {
        return _config->PRODUCE_ELIXIR[_level];
    }
    return _config->PRODUCE_ELIXIR.empty() ? 0.0f : _config->PRODUCE_ELIXIR[0];
}

float ProductionBuilding::getCurrentSTORAGE_ELIXIR_CAPACITY() const {
    if (_level >= 0 && _level < _config->STORAGE_ELIXIR_CAPACITY.size()) {
        return _config->STORAGE_ELIXIR_CAPACITY[_level];
    }
    return _config->STORAGE_ELIXIR_CAPACITY.empty() ? 0.0f : _config->STORAGE_ELIXIR_CAPACITY[0];
}

float ProductionBuilding::getCurrentPRODUCE_GOLD() const {
    if (_level >= 0 && _level < _config->PRODUCE_GOLD.size()) {
        return _config->PRODUCE_GOLD[_level];
    }
    return _config->PRODUCE_GOLD.empty() ? 0.0f : _config->PRODUCE_GOLD[0];
}

float ProductionBuilding::getCurrentSTORAGE_GOLD_CAPACITY() const {
    if (_level >= 0 && _level < _config->STORAGE_GOLD_CAPACITY.size()) {
        return _config->STORAGE_GOLD_CAPACITY[_level];
    }
    return _config->STORAGE_GOLD_CAPACITY.empty() ? 0.0f : _config->STORAGE_GOLD_CAPACITY[0];
}

int ProductionBuilding::getLength() const {
    return _config->length;
}

int ProductionBuilding::getWidth() const {
    return _config->width;
}

void ProductionBuilding::update(float dt) {
    produce(dt);
}

void ProductionBuilding::produce(float dt) {
    // TODO: Implement resource production
}

void ProductionBuilding::takeDamage(float damage) {
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;
    
    updateHealthBar(true);
    
    if (_currentHP <= 0) {
        this->removeFromParent();
    }
}

void ProductionBuilding::updateHealthBar(bool animate) {
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
        auto action = ScaleTo::create(0.12f, targetScaleX, 1.0f);
        _healthBar->runAction(action);
    } else {
        _healthBar->setScaleX(targetScaleX);
    }

    if (pct > 0.5f) {
        _healthBar->setColor(Color3B::GREEN);
    } else if (pct > 0.2f) {
        _healthBar->setColor(Color3B::YELLOW);
    } else {
        _healthBar->setColor(Color3B::RED);
    }

    if (pct <= 0.0f) {
        _healthBar->setVisible(false);
    } else {
        _healthBar->setVisible(true);
    }
}

void ProductionBuilding::playAnimation(const std::string& animType, int frameCount, float delay, bool loop) {
    if (!_bodySprite || !_config) return;
    
    std::string key = animType;
    if (_currentActionKey == key) return;
    
    auto anim = AnimationUtils::buildAnimationFromFrames(_config->spriteFrameName, animType, frameCount, delay);
    if (!anim) return;
    
    _bodySprite->stopAllActions();
    
    if (loop) {
        auto act = RepeatForever::create(Animate::create(anim));
        _bodySprite->runAction(act);
    } else {
        auto sequence = Sequence::create(
            Animate::create(anim),
            CallFunc::create([this]() {
                _currentActionKey.clear();
            }),
            nullptr
        );
        _bodySprite->runAction(sequence);
    }
    
    _currentActionKey = key;
}

void ProductionBuilding::stopCurrentAnimation() {
    if (!_bodySprite) return;
    _bodySprite->stopAllActions();
    _currentActionKey.clear();
}
