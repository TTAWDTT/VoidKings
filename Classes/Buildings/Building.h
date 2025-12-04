// Soldier.h
#include "cocos2d.h"
#include "BuildingData.h"

// 此处开始写士兵类
// 说明 - 关于为什么继承Node而不是Sprite：
// Soldier从绘图层面来说，不只有Soldier本身进行绘制，还有血条，可能还有阴影
// 但是士兵缩放、变色时，血条不应该进行变换
// 所以创建一个Node，Sprite会是其子节点，根据渲染逻辑，会能够渲染出来
class Soldier : public cocos2d::Node {
public:
    // 标准 Cocos create 方法
	static Soldier* create(const UnitConfig* config, int level = 0); // 创建士兵实例,默认等级为0

    virtual bool init(const UnitConfig* config, int level = 0); // 初始化、绑定子节点
    virtual void update(float dt) override;

    // 状态控制
    void takeDamage(float damage);
    
    // 等级相关方法
    int getLevel() const { return _level; }
    void setLevel(int level);
    
    // 获取当前等级的属性
    float getCurrentHP() const;
    float getCurrentMaxHP() const;
    float getCurrentSpeed() const;
    float getCurrentATK() const;
    float getCurrentRange() const;

private:
    // 核心：持有一个指向配置的指针 (享元模式,不要拷贝整个结构体)
    const UnitConfig* _config;

    // 运行时数据
    int _level;                   // 当前等级
    float _currentHP;
    cocos2d::Sprite* _bodySprite; // 这个会定义它作为精灵,当时应渲染的图片
    cocos2d::Node* _target;       // 当前锁定的攻击目标（也是一个Node）

    // 内部行为逻辑
    void findTarget();            // 索敌逻辑
    void moveToTarget(float dt);  // 移动逻辑
    void attackTarget();          // 攻击逻辑
};