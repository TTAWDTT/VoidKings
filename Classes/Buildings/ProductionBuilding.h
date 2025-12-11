#ifndef __PRODUCTION_BUILDING_H__
#define __PRODUCTION_BUILDING_H__

#include "cocos2d.h"
#include "ProductionBuildingData.h"

class ProductionBuilding : public cocos2d::Node {
public:
    static ProductionBuilding* create(const ProductionBuildingConfig* config, int level = 0);
    virtual bool init(const ProductionBuildingConfig* config, int level = 0);
    virtual void update(float dt) override;

    void takeDamage(float damage);
    
    int getLevel() const { return _level; }
    void setLevel(int level);
    
    float getCurrentMaxHP() const;
    float getCurrentDP() const;
    float getCurrentPRODUCE_ELIXIR() const;
    float getCurrentSTORAGE_ELIXIR_CAPACITY() const;
    float getCurrentPRODUCE_GOLD() const;
    float getCurrentSTORAGE_GOLD_CAPACITY() const;
    
    int getLength() const;
    int getWidth() const;

private:
    const ProductionBuildingConfig* _config;
    int _level;
    float _currentHP;
    float _lastProduceTime;
    cocos2d::Sprite* _bodySprite;
    cocos2d::Sprite* _healthBar;
    std::string _currentActionKey;

    void produce(float dt);
    void updateHealthBar(bool animate = true);
    void playAnimation(const std::string& animType, int frameCount, float delay, bool loop);
    void stopCurrentAnimation();
};

#endif // __PRODUCTION_BUILDING_H__
