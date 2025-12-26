#include "Trap.h"
#include "Map/GridMap.h"
#include "Soldier/Soldier.h"
#include "Utils/AudioManager.h"
#include <cmath>

USING_NS_CC;

namespace {
constexpr int kSpikeFrameStart = 1;
constexpr int kSpikeFrameEnd = 14;
constexpr float kSpikeFrameDelay = 0.08f;

constexpr int kTrapFrameStart = 1;
constexpr int kTrapFrameEnd = 4;
constexpr float kTrapFrameDelay = 0.1f;

constexpr float kSpikeDamageInterval = 0.5f;
constexpr float kSpikeDamagePerTick = 18.0f;

Animation* buildNumberedAnimation(const std::string& prefix, int start, int end, float delay) {
    Vector<SpriteFrame*> frames;
    for (int i = start; i <= end; ++i) {
        std::string framePath = StringUtils::format("%s%d.png", prefix.c_str(), i);
        auto texture = Director::getInstance()->getTextureCache()->addImage(framePath);
        if (!texture) {
            continue;
        }
        Size size = texture->getContentSize();
        auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, size.width, size.height));
        if (frame) {
            frames.pushBack(frame);
        }
    }
    if (frames.empty()) {
        return nullptr;
    }
    return Animation::createWithSpriteFrames(frames, delay);
}
} // namespace

const std::vector<Soldier*>* TrapBase::s_enemySoldiers = nullptr;

void TrapBase::setEnemySoldiers(const std::vector<Soldier*>* soldiers) {
    s_enemySoldiers = soldiers;
}

void TrapBase::setGridContext(GridMap* gridMap, int gridX, int gridY, int width, int height) {
    _gridMap = gridMap;
    _gridX = gridX;
    _gridY = gridY;
    _gridWidth = width;
    _gridHeight = height;
    _gridBound = true;
}

bool TrapBase::initTrapBase(const std::string& firstFrame,
                            const std::string& framePrefix,
                            int frameStart,
                            int frameEnd,
                            float frameDelay,
                            bool loop) {
    if (!Node::init()) {
        return false;
    }

    _bodySprite = Sprite::create(firstFrame);
    if (!_bodySprite) {
        _bodySprite = Sprite::create();
        auto marker = DrawNode::create();
        marker->drawSolidRect(Vec2(-18.0f, -18.0f), Vec2(18.0f, 18.0f), Color4F(0.3f, 0.3f, 0.3f, 1.0f));
        _bodySprite->addChild(marker);
    }
    _bodySprite->setName("bodySprite");
    this->addChild(_bodySprite);

    auto anim = buildNumberedAnimation(framePrefix, frameStart, frameEnd, frameDelay);
    if (anim && anim->getFrames().size() > 1) {
        auto animate = Animate::create(anim);
        if (loop) {
            _bodySprite->runAction(RepeatForever::create(animate));
        }
        else {
            _bodySprite->runAction(animate);
        }
    }

    return true;
}

Rect TrapBase::getTriggerRect() const {
    if (!_gridMap || !_gridBound) {
        return Rect::ZERO;
    }
    float cellSize = _gridMap->getCellSize();
    return Rect(_gridX * cellSize, _gridY * cellSize, _gridWidth * cellSize, _gridHeight * cellSize);
}

Rect getWorldRectFromLocal(Node* node, const Rect& localRect) {
    if (!node) {
        return Rect::ZERO;
    }
    Vec2 bl = node->convertToWorldSpace(localRect.origin);
    Vec2 tr = node->convertToWorldSpace(localRect.origin + Vec2(localRect.size.width, localRect.size.height));
    float minX = std::min(bl.x, tr.x);
    float minY = std::min(bl.y, tr.y);
    float maxX = std::max(bl.x, tr.x);
    float maxY = std::max(bl.y, tr.y);
    return Rect(minX, minY, maxX - minX, maxY - minY);
}

Rect getWorldRectFromNodeBounds(Node* node) {
    if (!node) {
        return Rect::ZERO;
    }
    Rect localRect = node->getBoundingBox();
    Node* parent = node->getParent();
    Vec2 bl = parent ? parent->convertToWorldSpace(localRect.origin) : localRect.origin;
    Vec2 tr = parent ? parent->convertToWorldSpace(localRect.origin + Vec2(localRect.size.width, localRect.size.height))
                     : localRect.origin + Vec2(localRect.size.width, localRect.size.height);
    float minX = std::min(bl.x, tr.x);
    float minY = std::min(bl.y, tr.y);
    float maxX = std::max(bl.x, tr.x);
    float maxY = std::max(bl.y, tr.y);
    return Rect(minX, minY, maxX - minX, maxY - minY);
}

Rect getLocalRectFromWorldRect(Node* node, const Rect& worldRect) {
    if (!node) {
        return Rect::ZERO;
    }
    Vec2 bl = node->convertToNodeSpace(worldRect.origin);
    Vec2 tr = node->convertToNodeSpace(worldRect.origin + Vec2(worldRect.size.width, worldRect.size.height));
    float minX = std::min(bl.x, tr.x);
    float minY = std::min(bl.y, tr.y);
    float maxX = std::max(bl.x, tr.x);
    float maxY = std::max(bl.y, tr.y);
    return Rect(minX, minY, maxX - minX, maxY - minY);
}

Vec2 TrapBase::getSoldierLocalPos(const Soldier* soldier) const {
    if (!soldier || !_gridMap) {
        return Vec2::ZERO;
    }
    auto* parent = soldier->getParent();
    Vec2 worldPos = parent ? parent->convertToWorldSpace(soldier->getPosition()) : soldier->getPosition();
    return _gridMap->convertToNodeSpace(worldPos);
}

bool TrapBase::getSoldierGridPos(const Soldier* soldier, int& outX, int& outY) const {
    if (!soldier || !_gridMap) {
        return false;
    }
    Vec2 localPos = getSoldierLocalPos(soldier);
    Vec2 gridPos = _gridMap->worldToGrid(localPos);
    outX = static_cast<int>(gridPos.x);
    outY = static_cast<int>(gridPos.y);
    return true;
}

void TrapBase::freeGridIfNeeded() {
    if (_gridFreed || !_gridMap || !_gridBound) {
        return;
    }
    _gridMap->freeCell(_gridX, _gridY, _gridWidth, _gridHeight);
    _gridFreed = true;
}

void TrapBase::onExit() {
    freeGridIfNeeded();
    Node::onExit();
}

SpikeTrap* SpikeTrap::create() {
    auto* trap = new(std::nothrow) SpikeTrap();
    if (trap && trap->init()) {
        trap->autorelease();
        return trap;
    }
    CC_SAFE_DELETE(trap);
    return nullptr;
}

bool SpikeTrap::init() {
    if (!initTrapBase("buildings/spike/spike_1.png",
        "buildings/spike/spike_",
        kSpikeFrameStart,
        kSpikeFrameEnd,
        kSpikeFrameDelay,
        true)) {
        return false;
    }

    AudioManager::playSpikeAppear();
    _damageTimer = 0.0f;
    this->scheduleUpdate();
    return true;
}

void SpikeTrap::update(float dt) {
    if (!_gridMap || !_gridBound || !s_enemySoldiers) {
        return;
    }

    _damageTimer += dt;
    if (_damageTimer < kSpikeDamageInterval) {
        return;
    }
    _damageTimer = 0.0f;

    Rect triggerRect = getTriggerRect();
    Rect worldRect = getWorldRectFromLocal(_gridMap, triggerRect);
    for (auto* soldier : *s_enemySoldiers) {
        if (!soldier || !soldier->getParent()) {
            continue;
        }
        if (soldier->getCurrentHP() <= 0.0f) {
            continue;
        }
        Rect soldierWorldRect = getWorldRectFromNodeBounds(soldier);
        if (soldierWorldRect.size.width <= 0.0f || soldierWorldRect.size.height <= 0.0f) {
            Vec2 worldPos = soldier->getParent()->convertToWorldSpace(soldier->getPosition());
            if (worldRect.containsPoint(worldPos)) {
                soldier->takeDamage(kSpikeDamagePerTick);
            }
            continue;
        }
        if (worldRect.intersectsRect(soldierWorldRect)) {
            soldier->takeDamage(kSpikeDamagePerTick);
        }
    }
}

SnapTrap* SnapTrap::create() {
    auto* trap = new(std::nothrow) SnapTrap();
    if (trap && trap->init()) {
        trap->autorelease();
        return trap;
    }
    CC_SAFE_DELETE(trap);
    return nullptr;
}

bool SnapTrap::init() {
    if (!initTrapBase("buildings/trap/trap_1.png",
        "buildings/trap/trap_",
        kTrapFrameStart,
        kTrapFrameStart,
        kTrapFrameDelay,
        false)) {
        return false;
    }

    _triggered = false;
    this->scheduleUpdate();
    return true;
}

void SnapTrap::update(float dt) {
    if (_triggered || !_gridMap || !_gridBound || !s_enemySoldiers) {
        return;
    }

    Rect triggerRect = getTriggerRect();
    std::vector<Soldier*> victims;
    for (auto* soldier : *s_enemySoldiers) {
        if (!soldier || !soldier->getParent()) {
            continue;
        }
        if (soldier->getCurrentHP() <= 0.0f) {
            continue;
        }

        bool shouldTrigger = false;
        Rect soldierWorldRect = getWorldRectFromNodeBounds(soldier);
        if (soldierWorldRect.size.width > 0.0f && soldierWorldRect.size.height > 0.0f) {
            Rect soldierLocalRect = getLocalRectFromWorldRect(_gridMap, soldierWorldRect);
            shouldTrigger = triggerRect.intersectsRect(soldierLocalRect);
        }
        else {
            int gridX = 0;
            int gridY = 0;
            if (getSoldierGridPos(soldier, gridX, gridY)) {
                shouldTrigger = (gridX == _gridX && gridY == _gridY);
            }
        }

        if (shouldTrigger) {
            victims.push_back(soldier);
        }
    }

    if (!victims.empty()) {
        // 捕兽夹一次吞噬格子内所有敌人，避免多人叠加时漏触发
        triggerOnSoldiers(victims);
    }
}

void SnapTrap::triggerOnSoldiers(const std::vector<Soldier*>& soldiers) {
    _triggered = true;

    AudioManager::playSnapTrap();
    for (auto* soldier : soldiers) {
        if (soldier) {
            soldier->takeDamage(soldier->getCurrentHP() + 1.0f);
        }
    }

    if (_bodySprite) {
        _bodySprite->stopAllActions();
        auto anim = buildNumberedAnimation("buildings/trap/trap_", kTrapFrameStart, kTrapFrameEnd, 0.06f);
        if (anim) {
            _bodySprite->runAction(Animate::create(anim));
            float duration = anim->getDuration();
            this->runAction(Sequence::create(DelayTime::create(duration), RemoveSelf::create(), nullptr));
            return;
        }
    }

    this->removeFromParent();
}
