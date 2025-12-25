#include "ProductionBuilding.h"
#include "Core/Core.h"
#include "Utils/AnimationUtils.h"
#include "Utils/EffectUtils.h"
#include "Utils/AudioManager.h"

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
    _pendingCollectAmount = 0;
    _collectType = ResourceType::COIN;

    _bodySprite = Sprite::create(_config->spriteFrameName);
    if (_bodySprite) {
        _bodySprite->setName("bodySprite");
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
        _healthBar->setName("healthBar");
        _healthBar->setAnchorPoint(Vec2(0.0f, 0.5f));
        this->addChild(_healthBar);
        _healthBar->setScaleX(1.0f);
        _healthBar->setColor(Color3B::GREEN);
        refreshHealthBarPosition();
    }

    updateHealthBar(false);
    this->scheduleUpdate();

    return true;
}

void ProductionBuilding::setCollectCallback(const std::function<void(ProductionBuilding*, ResourceType, int, const cocos2d::Vec2&)>& callback) {
    _collectCallback = callback;
}

void ProductionBuilding::refreshHealthBarPosition() {
    if (!_healthBar || !_bodySprite) {
        return;
    }

    float offsetY = 30.0f;
    float spriteHeight = _bodySprite->getContentSize().height * _bodySprite->getScaleY();
    if (spriteHeight <= 0.0f) {
        spriteHeight = _bodySprite->getBoundingBox().size.height;
    }
    float y = spriteHeight * 0.5f + offsetY;
    float x = -_healthBar->getContentSize().width * 0.5f;
    _healthBar->setPosition(Vec2(x, y));
}

void ProductionBuilding::refreshCollectIconPosition() {
    if (!_collectSprite || !_bodySprite) {
        return;
    }
    float spriteHeight = _bodySprite->getContentSize().height * _bodySprite->getScaleY();
    if (spriteHeight <= 0.0f) {
        spriteHeight = _bodySprite->getBoundingBox().size.height;
    }
    float y = spriteHeight * 0.5f + 22.0f;
    _collectSprite->setPosition(Vec2(0.0f, y));
}

void ProductionBuilding::setLevel(int level) {
    if (level < 0) level = 0;
    if (level > _config->MAXLEVEL) level = _config->MAXLEVEL;
    _level = level;
}

float ProductionBuilding::getCurrentMaxHP() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->HP.size()) {
        return _config->HP[_level];
    }
    return _config->HP.empty() ? 0.0f : _config->HP[0];
}

float ProductionBuilding::getCurrentDP() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->DP.size()) {
        return _config->DP[_level];
    }
    return _config->DP.empty() ? 0.0f : _config->DP[0];
}

float ProductionBuilding::getCurrentPRODUCE_ELIXIR() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->PRODUCE_ELIXIR.size()) {
        return _config->PRODUCE_ELIXIR[_level];
    }
    return _config->PRODUCE_ELIXIR.empty() ? 0.0f : _config->PRODUCE_ELIXIR[0];
}

float ProductionBuilding::getCurrentSTORAGE_ELIXIR_CAPACITY() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->STORAGE_ELIXIR_CAPACITY.size()) {
        return _config->STORAGE_ELIXIR_CAPACITY[_level];
    }
    return _config->STORAGE_ELIXIR_CAPACITY.empty() ? 0.0f : _config->STORAGE_ELIXIR_CAPACITY[0];
}

float ProductionBuilding::getCurrentPRODUCE_GOLD() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->PRODUCE_GOLD.size()) {
        return _config->PRODUCE_GOLD[_level];
    }
    return _config->PRODUCE_GOLD.empty() ? 0.0f : _config->PRODUCE_GOLD[0];
}

float ProductionBuilding::getCurrentSTORAGE_GOLD_CAPACITY() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->STORAGE_GOLD_CAPACITY.size()) {
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
    if (!_config) {
        return;
    }

    float gold = getCurrentPRODUCE_GOLD();
    float diamond = getCurrentPRODUCE_ELIXIR();
    if (gold <= 0.0f && diamond <= 0.0f) {
        return;
    }

    float currentTime = Director::getInstance()->getTotalFrames() / 60.0f;
    if (_lastProduceTime <= 0.0f) {
        _lastProduceTime = currentTime;
        return;
    }

    float interval = getProduceInterval();
    if (interval < 0.1f) {
        interval = 0.1f;
    }

    if (currentTime - _lastProduceTime >= interval) {
        _lastProduceTime = currentTime;

        bool useCollect = isCollectorBuilding() && _collectCallback;
        if (useCollect) {
            if (gold > 0.0f) {
                addPendingCollect(ResourceType::COIN, static_cast<int>(gold));
            }
            if (diamond > 0.0f) {
                addPendingCollect(ResourceType::DIAMOND, static_cast<int>(diamond));
            }
            playProducePulse();
            return;
        }

        // 自动收集资源并播放产出动画
        if (gold > 0.0f) {
            Core::getInstance()->addResource(ResourceType::COIN, static_cast<int>(gold));
            spawnProduceEffect(ResourceType::COIN);
        }
        if (diamond > 0.0f) {
            Core::getInstance()->addResource(ResourceType::DIAMOND, static_cast<int>(diamond));
            spawnProduceEffect(ResourceType::DIAMOND);
        }
        playProducePulse();
    }
}

void ProductionBuilding::takeDamage(float damage) {
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;
    
    EffectUtils::playHitFlash(_bodySprite);
    updateHealthBar(true);
    
    if (_currentHP <= 0) {
        AudioManager::playBuildingCollapse();
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

float ProductionBuilding::getProduceInterval() const {
    // 默认产出间隔（GoldMaker/DiamondMaker 受基地等级影响）
    constexpr float kBaseInterval = 4.0f;
    if (!_config) {
        return kBaseInterval;
    }

    bool isGoldMaker = (_config->id == 3003) || (_config->name == "GoldMaker");
    bool isDiamondMaker = (_config->id == 3004) || (_config->name == "DiamondMaker");
    if (!isGoldMaker && !isDiamondMaker) {
        return kBaseInterval;
    }

    float speedMul = Core::getInstance()->getBaseProduceSpeedMultiplier();
    if (speedMul < 0.2f) {
        speedMul = 0.2f;
    }
    return kBaseInterval / speedMul;
}

void ProductionBuilding::spawnProduceEffect(ResourceType type) {
    if (!_bodySprite) {
        return;
    }

    auto sprite = Core::getInstance()->createResourceSprite(type);
    if (!sprite) {
        return;
    }

    float offsetY = _bodySprite->getContentSize().height * 0.5f + 12.0f;
    sprite->setPosition(Vec2(0.0f, offsetY));
    sprite->setScale(0.75f);
    this->addChild(sprite, 20);

    auto move = MoveBy::create(0.6f, Vec2(0.0f, 26.0f));
    auto fade = FadeTo::create(0.6f, 0);
    sprite->runAction(Sequence::create(Spawn::create(move, fade, nullptr), RemoveSelf::create(), nullptr));
}

void ProductionBuilding::playProducePulse() {
    if (!_bodySprite) {
        return;
    }

    constexpr int kProducePulseTag = 31001;
    _bodySprite->stopActionByTag(kProducePulseTag);

    float baseScaleX = _bodySprite->getScaleX();
    float baseScaleY = _bodySprite->getScaleY();
    auto scaleUp = ScaleTo::create(0.08f, baseScaleX * 1.04f, baseScaleY * 1.04f);
    auto scaleDown = ScaleTo::create(0.12f, baseScaleX, baseScaleY);
    auto seq = Sequence::create(scaleUp, scaleDown, nullptr);
    seq->setTag(kProducePulseTag);
    _bodySprite->runAction(seq);
}

bool ProductionBuilding::isCollectorBuilding() const {
    if (!_config) {
        return false;
    }
    return _config->id == 3003 || _config->id == 3004
        || _config->name == "GoldMaker" || _config->name == "DiamondMaker";
}

ResourceType ProductionBuilding::getCollectType() const {
    if (_config && (_config->id == 3004 || _config->name == "DiamondMaker")) {
        return ResourceType::DIAMOND;
    }
    return ResourceType::COIN;
}

void ProductionBuilding::addPendingCollect(ResourceType type, int amount) {
    if (amount <= 0) {
        return;
    }
    _collectType = type;
    _pendingCollectAmount += amount;
    updateCollectIcon();
}

void ProductionBuilding::updateCollectIcon() {
    if (_pendingCollectAmount <= 0) {
        clearCollectIcon();
        return;
    }
    if (!_collectSprite) {
        _collectSprite = Core::getInstance()->createResourceSprite(_collectType);
        if (!_collectSprite) {
            return;
        }
        _collectSprite->setName("collectIcon");
        float targetHeight = 18.0f;
        float scale = targetHeight / _collectSprite->getContentSize().height;
        _collectSprite->setScale(scale);
        this->addChild(_collectSprite, 5);
        refreshCollectIconPosition();

        _collectListener = EventListenerTouchOneByOne::create();
        _collectListener->setSwallowTouches(true);
        float baseScale = _collectSprite->getScale();
        _collectListener->onTouchBegan = [this, baseScale](Touch* touch, Event*) {
            if (!_collectSprite || _pendingCollectAmount <= 0) {
                return false;
            }
            Vec2 local = this->convertToNodeSpace(touch->getLocation());
            if (_collectSprite->getBoundingBox().containsPoint(local)) {
                _collectSprite->setScale(baseScale * 0.92f);
                return true;
            }
            return false;
        };
        _collectListener->onTouchCancelled = [this, baseScale](Touch*, Event*) {
            if (_collectSprite) {
                _collectSprite->setScale(baseScale);
            }
        };
        _collectListener->onTouchEnded = [this, baseScale](Touch*, Event*) {
            if (_collectSprite) {
                _collectSprite->setScale(baseScale);
            }
            collectPending();
        };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(_collectListener, _collectSprite);
    }
}

void ProductionBuilding::clearCollectIcon() {
    if (_collectListener && _collectSprite) {
        _eventDispatcher->removeEventListener(_collectListener);
        _collectListener = nullptr;
    }
    if (_collectSprite) {
        _collectSprite->removeFromParent();
        _collectSprite = nullptr;
    }
}

void ProductionBuilding::collectPending() {
    if (_pendingCollectAmount <= 0) {
        return;
    }
    int amount = _pendingCollectAmount;
    _pendingCollectAmount = 0;

    Vec2 worldPos = this->convertToWorldSpace(Vec2::ZERO);
    if (_collectSprite) {
        worldPos = this->convertToWorldSpace(_collectSprite->getPosition());
    }

    clearCollectIcon();

    if (_collectCallback) {
        _collectCallback(this, _collectType, amount, worldPos);
    }
    else {
        Core::getInstance()->addResource(_collectType, amount);
    }
}
