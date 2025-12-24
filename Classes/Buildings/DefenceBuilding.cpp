#include "DefenceBuilding.h"
#include "Soldier/Soldier.h"
#include "Bullet/Bullet.h"
#include "Utils/AnimationUtils.h"
#include "Utils/EffectUtils.h"

USING_NS_CC;

const std::vector<Soldier*>* DefenceBuilding::s_enemySoldiers = nullptr;

namespace {
class DefenceBullet : public Bullet {
public:
    static DefenceBullet* create(const std::string& spriteFrame,
                                 float damage,
                                 float speed,
                                 bool isAOE,
                                 float aoeRange,
                                 bool allowSky,
                                 bool allowGround,
                                 const std::vector<Soldier*>* enemySoldiers) {
        auto* bullet = new(std::nothrow) DefenceBullet();
        if (bullet && bullet->init(spriteFrame, damage, speed)) {
            bullet->_isAOE = isAOE;
            bullet->_aoeRange = aoeRange;
            bullet->_allowSky = allowSky;
            bullet->_allowGround = allowGround;
            bullet->_enemySoldiers = enemySoldiers;
            bullet->autorelease();
            return bullet;
        }
        CC_SAFE_DELETE(bullet);
        return nullptr;
    }

protected:
    void onReachTarget() override {
        float damage = getDamage();
        if (_isAOE && _enemySoldiers && _aoeRange > 0.0f) {
            Vec2 impactPos = this->getPosition();
            for (auto* soldier : *_enemySoldiers) {
                if (!canHitSoldier(soldier)) {
                    continue;
                }
                if (impactPos.distance(soldier->getPosition()) <= _aoeRange) {
                    soldier->takeDamage(damage);
                }
            }
            Bullet::onReachTarget();
            return;
        }

        if (auto* soldier = dynamic_cast<Soldier*>(_target)) {
            if (canHitSoldier(soldier)) {
                soldier->takeDamage(damage);
            }
        }
        Bullet::onReachTarget();
    }

private:
    // 目标过滤：死亡/空地可攻击判断
    bool canHitSoldier(const Soldier* soldier) const {
        if (!soldier || !soldier->getParent()) {
            return false;
        }
        if (soldier->getCurrentHP() <= 0) {
            return false;
        }
        if (soldier->isFlying()) {
            return _allowSky;
        }
        return _allowGround;
    }

    bool _isAOE = false;
    float _aoeRange = 0.0f;
    bool _allowSky = false;
    bool _allowGround = false;
    const std::vector<Soldier*>* _enemySoldiers = nullptr;
};
} // namespace


void DefenceBuilding::setEnemySoldiers(const std::vector<Soldier*>* soldiers) {
    s_enemySoldiers = soldiers;
}


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

        // 检查是否是有动画的建筑（如Tree），如果是则播放帧动画
        tryPlayIdleAnimation();
    }

    // 如果没有血条图片，则创建简单的彩色血条
    _healthBar = Sprite::create("res/health_bar.png");
    if (!_healthBar) {
        auto healthBarBg = Sprite::create();
        if (healthBarBg) {
            unsigned char data[] = { 255, 255, 255, 255 };
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
    if (_level >= 0 && static_cast<size_t>(_level) < _config->HP.size()) {
        return _config->HP[_level];
    }
    return _config->HP.empty() ? 0.0f : _config->HP[0];
}

float DefenceBuilding::getCurrentDP() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->DP.size()) {
        return _config->DP[_level];
    }
    return _config->DP.empty() ? 0.0f : _config->DP[0];
}

float DefenceBuilding::getCurrentATK_SPEED() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->ATK_SPEED.size()) {
        return _config->ATK_SPEED[_level];
    }
    return _config->ATK_SPEED.empty() ? 0.0f : _config->ATK_SPEED[0];
}

float DefenceBuilding::getCurrentATK() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->ATK.size()) {
        return _config->ATK[_level];
    }
    return _config->ATK.empty() ? 0.0f : _config->ATK[0];
}

float DefenceBuilding::getCurrentATK_RANGE() const {
    if (_level >= 0 && static_cast<size_t>(_level) < _config->ATK_RANGE.size()) {
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
    if (_currentHP <= 0) {
        return;
    }

    if (_target) {
        auto* soldier = dynamic_cast<Soldier*>(_target);
        if (!canTargetSoldier(soldier)) {
            setTarget(nullptr);
        }
    }

    if (!_target) {
        findTarget();
    }

    if (_target) {
        auto* soldier = dynamic_cast<Soldier*>(_target);
        if (!canTargetSoldier(soldier)) {
            setTarget(nullptr);
            return;
        }
        float dist = this->getPosition().distance(_target->getPosition());
        if (dist <= getCurrentATK_RANGE()) {
            attackTarget();
        }
        else {
            setTarget(nullptr);
        }
    }
}

void DefenceBuilding::onExit() {
    setTarget(nullptr);
    Node::onExit();
}

void DefenceBuilding::setTarget(cocos2d::Node* target) {
    if (_target == target) {
        return;
    }
    if (_target) {
        _target->release();
    }
    _target = target;
    if (_target) {
        _target->retain();
    }
}

bool DefenceBuilding::canTargetSoldier(const Soldier* soldier) const {
    // 过滤不可攻击目标（空/地、死亡、失去父节点）
    if (!soldier || !soldier->getParent()) {
        return false;
    }
    if (soldier->getCurrentHP() <= 0) {
        return false;
    }
    if (!_config) {
        return false;
    }
    if (soldier->isFlying()) {
        return _config->SKY_ABLE;
    }
    return _config->GROUND_ABLE;
}

const std::vector<Soldier*>* DefenceBuilding::getEnemySoldiers(std::vector<Soldier*>& fallback) const {
    if (s_enemySoldiers && !s_enemySoldiers->empty()) {
        return s_enemySoldiers;
    }

    fallback.clear();
    const Node* parent = this->getParent();
    const Node* gridRoot = parent ? parent->getParent() : nullptr;
    if (!gridRoot) {
        return nullptr;
    }

    for (auto* layer : gridRoot->getChildren()) {
        if (!layer) {
            continue;
        }
        for (auto* node : layer->getChildren()) {
            auto* soldier = dynamic_cast<Soldier*>(node);
            if (soldier) {
                fallback.push_back(soldier);
            }
        }
    }

    return fallback.empty() ? nullptr : &fallback;
}

void DefenceBuilding::applyAoeDamage(const Vec2& center, float range, float damage) const {
    if (range <= 0.0f) {
        return;
    }

    std::vector<Soldier*> fallback;
    const auto* candidates = getEnemySoldiers(fallback);
    if (!candidates) {
        return;
    }

    for (auto* soldier : *candidates) {
        if (!canTargetSoldier(soldier)) {
            continue;
        }
        if (center.distance(soldier->getPosition()) <= range) {
            soldier->takeDamage(damage);
        }
    }
}

bool DefenceBuilding::isTreeSprite() const {
    if (!_config) {
        return false;
    }
    return _config->spriteFrameName.find("buildings/Tree/") != std::string::npos;
}

bool DefenceBuilding::playTreeAnimation(int frameCount, float delay, bool loop) {
    if (!_bodySprite) {
        return false;
    }
    if (frameCount <= 0) {
        return false;
    }
    if (delay <= 0.0f) {
        delay = 0.1f;
    }

    Animation* anim = Animation::create();
    int loadedFrames = 0;
    for (int i = 0; i < frameCount; ++i) {
        std::string framePath = StringUtils::format("buildings/Tree/sprite_%04d.png", i);
        auto tempSprite = Sprite::create(framePath);
        if (tempSprite) {
            Size frameSize = tempSprite->getContentSize();
            auto frame = SpriteFrame::create(framePath, Rect(0, 0, frameSize.width, frameSize.height));
            if (frame) {
                anim->addSpriteFrame(frame);
                loadedFrames++;
            }
        }
    }

    if (loadedFrames == 0) {
        return false;
    }

    anim->setDelayPerUnit(delay);
    anim->setRestoreOriginalFrame(false);

    _bodySprite->stopAllActions();
    if (loop) {
        auto animate = Animate::create(anim);
        _bodySprite->runAction(RepeatForever::create(animate));
    }
    else {
        auto sequence = Sequence::create(
            Animate::create(anim),
            CallFunc::create([this]() {
                _currentActionKey.clear();
                // 攻击动画结束后恢复待机
                tryPlayIdleAnimation();
            }),
            nullptr
        );
        _bodySprite->runAction(sequence);
    }

    if (loop) {
        CCLOG("[防御建筑] Tree动画加载成功，共%d帧", loadedFrames);
    }
    return true;
}

void DefenceBuilding::playFallbackAttackEffect() {
    if (!_bodySprite) {
        return;
    }

    // 没有序列帧资源时，使用轻微缩放作为攻击反馈
    constexpr int kAttackEffectTag = 10001;
    _bodySprite->stopActionByTag(kAttackEffectTag);

    float scaleX = _bodySprite->getScaleX();
    float scaleY = _bodySprite->getScaleY();
    float boostX = scaleX * 1.06f;
    float boostY = scaleY * 1.06f;

    auto scaleUp = ScaleTo::create(0.05f, boostX, boostY);
    auto scaleDown = ScaleTo::create(0.08f, scaleX, scaleY);
    auto seq = Sequence::create(scaleUp, scaleDown, nullptr);
    seq->setTag(kAttackEffectTag);
    _bodySprite->runAction(seq);
}




void DefenceBuilding::findTarget() {
    std::vector<Soldier*> fallback;
    const std::vector<Soldier*>* candidates = getEnemySoldiers(fallback);
    if (!candidates || candidates->empty()) {
        return;
    }

    Soldier* nearest = nullptr;
    float nearestDist = 999999.0f;
    const auto selfPos = this->getPosition();
    float attackRange = getCurrentATK_RANGE();

    for (auto* soldier : *candidates) {
        if (!canTargetSoldier(soldier)) {
            continue;
        }

        float dist = selfPos.distance(soldier->getPosition());
        if (attackRange > 0.0f && dist > attackRange) {
            continue;
        }
        if (dist < nearestDist) {
            nearestDist = dist;
            nearest = soldier;
        }
    }

    setTarget(nearest);
}

void DefenceBuilding::takeDamage(float damage) {
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;

    EffectUtils::playHitFlash(_bodySprite);
    updateHealthBar(true);

    if (_currentHP <= 0) {
        this->removeFromParent();
    }
}

void DefenceBuilding::attackTarget() {
    auto* soldier = dynamic_cast<Soldier*>(_target);
    if (!canTargetSoldier(soldier)) {
        setTarget(nullptr);
        return;
    }

    float currentTime = Director::getInstance()->getTotalFrames() / 60.0f;
    float attackSpeed = getCurrentATK_SPEED();
    if (attackSpeed <= 0.0f) {
        attackSpeed = 0.5f;
    }

    if (currentTime - _lastAttackTime >= attackSpeed) {
        playAnimation(_config->anim_attack, _config->anim_attack_frames, _config->anim_attack_delay, false);
        _lastAttackTime = currentTime;

        float damage = getCurrentATK();

        if (!_config->bulletSpriteFrameName.empty() && _config->bulletSpeed > 0.0f) {
            auto* bullet = DefenceBullet::create(
                _config->bulletSpriteFrameName,
                damage,
                _config->bulletSpeed,
                _config->bulletIsAOE,
                _config->bulletAOERange,
                _config->SKY_ABLE,
                _config->GROUND_ABLE,
                s_enemySoldiers
            );
            if (bullet) {
                auto* parent = this->getParent();
                if (parent) {
                    parent->addChild(bullet, 20);
                    bullet->setPosition(this->getPosition());
                    bullet->setTarget(soldier);
                    return;
                }
            }
        }

        if (_config->bulletIsAOE && _config->bulletAOERange > 0.0f) {
            applyAoeDamage(soldier->getPosition(), _config->bulletAOERange, damage);
        }
        else {
            soldier->takeDamage(damage);
        }
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
    }
    else {
        _healthBar->setScaleX(targetScaleX);
    }

    if (pct > 0.5f) {
        _healthBar->setColor(Color3B::GREEN);
    }
    else if (pct > 0.2f) {
        _healthBar->setColor(Color3B::YELLOW);
    }
    else {
        _healthBar->setColor(Color3B::RED);
    }

    if (pct <= 0.0f) {
        _healthBar->setVisible(false);
    }
    else {
        _healthBar->setVisible(true);
    }
}

void DefenceBuilding::playAnimation(const std::string& animType, int frameCount, float delay, bool loop) {
    if (!_bodySprite || !_config) {
        return;
    }

    std::string key = animType;
    if (_currentActionKey == key) {
        return;
    }

    // Tree类型建筑优先使用现有序列帧资源
    if (isTreeSprite()) {
        int resolvedFrames = frameCount;
        if (resolvedFrames <= 1) {
            resolvedFrames = 16;
        }
        float resolvedDelay = delay;
        if (resolvedDelay <= 0.0f) {
            resolvedDelay = 0.1f;
        }
        if (playTreeAnimation(resolvedFrames, resolvedDelay, loop)) {
            _currentActionKey = key;
            return;
        }
    }

    std::string baseName = _config->spriteFrameName;
    if (baseName.size() > 4) {
        std::string suffix = baseName.substr(baseName.size() - 4);
        if (suffix == ".png" || suffix == ".PNG") {
            baseName = baseName.substr(0, baseName.size() - 4);
        }
    }

    auto anim = AnimationUtils::buildAnimationFromFrames(baseName, animType, frameCount, delay);
    if (!anim) {
        // 没有找到对应序列帧时，使用简易攻击表现
        if (animType == _config->anim_attack || animType == "attack") {
            playFallbackAttackEffect();
        }
        return;
    }

    _bodySprite->stopAllActions();

    if (loop) {
        auto act = RepeatForever::create(Animate::create(anim));
        _bodySprite->runAction(act);
    }
    else {
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

void DefenceBuilding::tryPlayIdleAnimation() {
    if (!_bodySprite || !_config) {
        return;
    }

    if (!isTreeSprite()) {
        return;
    }

    int frameCount = _config->anim_idle_frames;
    if (frameCount <= 1) {
        frameCount = 16;
    }
    float delay = _config->anim_idle_delay;
    if (delay <= 0.0f) {
        delay = 0.1f;
    }

    if (playTreeAnimation(frameCount, delay, true)) {
        _currentActionKey = "idle";
    }
}
