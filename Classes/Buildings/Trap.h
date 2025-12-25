#ifndef __TRAP_H__
#define __TRAP_H__

#include "cocos2d.h"
#include <vector>

class GridMap;
class Soldier;

class TrapBase : public cocos2d::Node {
public:
    static void setEnemySoldiers(const std::vector<Soldier*>* soldiers);
    void setGridContext(GridMap* gridMap, int gridX, int gridY, int width, int height);

protected:
    bool initTrapBase(const std::string& firstFrame,
                      const std::string& framePrefix,
                      int frameStart,
                      int frameEnd,
                      float frameDelay,
                      bool loop);
    cocos2d::Rect getTriggerRect() const;
    cocos2d::Vec2 getSoldierLocalPos(const Soldier* soldier) const;
    bool getSoldierGridPos(const Soldier* soldier, int& outX, int& outY) const;
    void freeGridIfNeeded();

    GridMap* _gridMap = nullptr;
    int _gridX = 0;
    int _gridY = 0;
    int _gridWidth = 0;
    int _gridHeight = 0;
    bool _gridBound = false;
    bool _gridFreed = false;

    cocos2d::Sprite* _bodySprite = nullptr;

    static const std::vector<Soldier*>* s_enemySoldiers;

    void onExit() override;
};

class SpikeTrap : public TrapBase {
public:
    static SpikeTrap* create();
    bool init() override;
    void update(float dt) override;

private:
    float _damageTimer = 0.0f;
};

class SnapTrap : public TrapBase {
public:
    static SnapTrap* create();
    bool init() override;
    void update(float dt) override;

private:
    bool _triggered = false;
    void triggerOnSoldier(Soldier* soldier);
};

#endif // __TRAP_H__
