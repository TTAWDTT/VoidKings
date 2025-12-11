#include "DefenceBuilding.h"
#include "Utils/AnimationUtils.h"

USING_NS_CC;

DefenceBuilding* DefenceBuilding::create(const DefenceBuildingConfig* config, int level) {
    DefenceBuilding* pRet = new(std::nothrow) DefenceBuilding();
    if (pRet && pRet->init(config, level)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool DefenceBuilding::init(const DefenceBuildingConfig* config, int level) {
    if (!Node::init()) return false;

    _config = config;
    _level = level;
    if (_level < 0) _level = 0;
    if (_level > _config->MAXLEVEL) _level = _config->MAXLEVEL;
    
    _currentHP = getCurrentMaxHP();
    _target = nullptr;
    _lastAttackTime = 0.0f;
    _currentActionKey.clear();

    _bodySprite = Sprite::create(_config->spriteFrameName);
    if (_bodySprite) {
        this->addChild(_bodySprite);
    }

    // Create health bar using DrawNode if image not available
    _healthBar = Sprite::create("res/health_bar.png");
    if (!_healthBar) {
        // Create a simple colored sprite as health bar
        auto healthBarBg = Sprite::create();
        if (healthBarBg) {
            // Create a 1x1 white texture and scale it
            unsigned char data[] = {255, 255, 255, 255};
            auto texture = new Texture2D();
            texture->initWithData(data, 1, Texture2D::PixelFormat::RGBA8888, 1, 1, Size(1, 1));
            healthBarBg->initWithTexture(texture);
            healthBarBg->setTextureRect(Rect(0, 0, 50, 5));
            texture->release();
            _healthBar = healthBarBg;
        }
    }
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

void DefenceBuilding::setLevel(int level) {
    if (level < 0) level = 0;
    if (level > _config->MAXLEVEL) level = _config->MAXLEVEL;
    _level = level;
}

float DefenceBuilding::getCurrentMaxHP() const {
    if (_level >= 0 && _level < _config->HP.size()) {
        return _config->HP[_level];
    }
    return _config->HP.empty() ? 0.0f : _config->HP[0];
}

float DefenceBuilding::getCurrentDP() const {
    if (_level >= 0 && _level < _config->DP.size()) {
        return _config->DP[_level];
    }
    return _config->DP.empty() ? 0.0f : _config->DP[0];
}

float DefenceBuilding::getCurrentATK_SPEED() const {
    if (_level >= 0 && _level < _config->ATK_SPEED.size()) {
        return _config->ATK_SPEED[_level];
    }
    return _config->ATK_SPEED.empty() ? 0.0f : _config->ATK_SPEED[0];
}

float DefenceBuilding::getCurrentATK() const {
    if (_level >= 0 && _level < _config->ATK.size()) {
        return _config->ATK[_level];
    }
    return _config->ATK.empty() ? 0.0f : _config->ATK[0];
}

float DefenceBuilding::getCurrentATK_RANGE() const {
    if (_level >= 0 && _level < _config->ATK_RANGE.size()) {
        return _config->ATK_RANGE[_level];
    }
    return _config->ATK_RANGE.empty() ? 0.0f : _config->ATK_RANGE[0];
}

int DefenceBuilding::getLength() const {
    return _config->length;
}

int DefenceBuilding::getWidth() const {
    return _config->width;
}

void DefenceBuilding::update(float dt) {
    if (!_target) {
        findTarget();
    } else {
        float dist = this->getPosition().distance(_target->getPosition());
        if (dist <= getCurrentATK_RANGE()) {
            attackTarget();
        } else {
            _target = nullptr;
        }
    }
}

void DefenceBuilding::findTarget() {
    // TODO: Implement with GameWorld
}

void DefenceBuilding::takeDamage(float damage) {
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;
    
    updateHealthBar(true);
    
    if (_currentHP <= 0) {
        this->removeFromParent();
    }
}

void DefenceBuilding::attackTarget() {
    if (!_target) return;
    
    float currentTime = Director::getInstance()->getTotalFrames() / 60.0f;
    float attackSpeed = getCurrentATK_SPEED();
    
    if (currentTime - _lastAttackTime >= attackSpeed) {
        playAnimation(_config->anim_attack, _config->anim_attack_frames, _config->anim_attack_delay, false);
        _lastAttackTime = currentTime;
        
        // TODO: Create and fire bullet
    }
}

void DefenceBuilding::updateHealthBar(bool animate) {
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

void DefenceBuilding::playAnimation(const std::string& animType, int frameCount, float delay, bool loop) {
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

void DefenceBuilding::stopCurrentAnimation() {
    if (!_bodySprite) return;
    _bodySprite->stopAllActions();
    _currentActionKey.clear();
}
