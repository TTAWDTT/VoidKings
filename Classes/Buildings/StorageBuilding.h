#ifndef __STORAGE_BUILDING_H__
#define __STORAGE_BUILDING_H__

#include "cocos2d.h"
#include "StorageBuildingData.h"

class StorageBuilding : public cocos2d::Node {
public:
    static StorageBuilding* create(const StorageBuildingConfig* config, int level = 0);
    virtual bool init(const StorageBuildingConfig* config, int level = 0);
    virtual void update(float dt) override;

    void takeDamage(float damage);
    
    int getLevel() const { return _level; }
    void setLevel(int level);
    
    float getCurrentHP() const { return _currentHP; }
    float getCurrentMaxHP() const;
    float getCurrentDP() const;
    float getCurrentADD_STORAGE_ELIXIR_CAPACITY() const;
    float getCurrentADD_STORAGE_GOLD_CAPACITY() const;
    
    int getLength() const;
    int getWidth() const;

    void refreshHealthBarPosition();

    int getId() const { return _config ? _config->id : 0; }
    const std::string& getName() const {
        static std::string empty = "";
        return _config ? _config->name : empty;
    }

private:
    const StorageBuildingConfig* _config;
    int _level;
    float _currentHP;
    cocos2d::Sprite* _bodySprite;
    cocos2d::Sprite* _healthBar;
    std::string _currentActionKey;

    void updateHealthBar(bool animate = true);
    void playAnimation(const std::string& animType, int frameCount, float delay, bool loop);
    void stopCurrentAnimation();
};

#endif // __STORAGE_BUILDING_H__
