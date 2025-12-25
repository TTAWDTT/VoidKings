#ifndef __PRODUCTION_BUILDING_H__
#define __PRODUCTION_BUILDING_H__

#include "cocos2d.h"
#include "ProductionBuildingData.h"
#include <functional>

enum class ResourceType;

class ProductionBuilding : public cocos2d::Node {
public:
    static ProductionBuilding* create(const ProductionBuildingConfig* config, int level = 0);
    virtual bool init(const ProductionBuildingConfig* config, int level = 0);
    virtual void update(float dt) override;

    void takeDamage(float damage);
    
    int getLevel() const { return _level; }
    void setLevel(int level);
    
    float getCurrentHP() const { return _currentHP; }
    float getCurrentMaxHP() const;
    float getCurrentDP() const;
    float getCurrentPRODUCE_ELIXIR() const;
    float getCurrentSTORAGE_ELIXIR_CAPACITY() const;
    float getCurrentPRODUCE_GOLD() const;
    float getCurrentSTORAGE_GOLD_CAPACITY() const;
    
    int getLength() const;
    int getWidth() const;

    void setCollectCallback(const std::function<void(ProductionBuilding*, ResourceType, int, const cocos2d::Vec2&)>& callback);
    void refreshCollectIconPosition();

    void refreshHealthBarPosition();
    
    // 获取建筑ID和名称
    int getId() const { return _config ? _config->id : 0; }
    const std::string& getName() const { 
        static std::string empty = "";
        return _config ? _config->name : empty; 
    }

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
    float getProduceInterval() const;
    void spawnProduceEffect(ResourceType type);
    void playProducePulse();

    bool isCollectorBuilding() const;
    ResourceType getCollectType() const;
    void addPendingCollect(ResourceType type, int amount);
    void updateCollectIcon();
    void clearCollectIcon();
    void collectPending();

    std::function<void(ProductionBuilding*, ResourceType, int, const cocos2d::Vec2&)> _collectCallback;
    cocos2d::Sprite* _collectSprite = nullptr;
    cocos2d::EventListenerTouchOneByOne* _collectListener = nullptr;
    int _pendingCollectAmount = 0;
    ResourceType _collectType;
};

#endif // __PRODUCTION_BUILDING_H__
