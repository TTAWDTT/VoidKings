// Soldier.h
#ifndef __SOLDIER_H__
#define __SOLDIER_H__

#include "cocos2d.h"
#include "UnitData.h"

// 此处开始写士兵类
// 说明 - 这里为什么继承Node而不是Sprite？
// Soldier从画图的角度来说，不只有Soldier本身需要绘制，还有血条，可能还有阴影
// 当士兵缩放、旋转时，血条都应该进行变换
// 所以创建一个Node，Sprite作为其子节点，加入渲染逻辑，就能够渲染动画
class Soldier : public cocos2d::Node {
public:
    // 标准 Cocos create 方法
    static Soldier* create(const UnitConfig* config, int level = 0); // 创建士兵实例,默认等级为0

    virtual bool init(const UnitConfig* config, int level = 0); // 初始化并添加子节点
    virtual void update(float dt) override;

    // 状态操作
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

    // 获取当前方向
    Direction getDirection() const { return _direction; }

private:
    // 配置模板——一个指针引用的指针 (享元模式,不需要保存配置结构体)
    const UnitConfig* _config;

    // 运行时数据
    int _level;                   // 当前等级
    float _currentHP;
    cocos2d::Sprite* _bodySprite; // 以后会定义这个为动画,暂时应该渲染成图片
    cocos2d::Sprite* _healthBar;  // 血条精灵
    cocos2d::Node* _target;       // 当前锁定的攻击目标（也是一个Node）
    // 动画支持
    std::string _currentActionKey;     // 当前动画的键
    Direction _direction;              // 当前方向 (LEFT/RIGHT)

    // 带方向动画接口 - 只需要一个方向(RIGHT),LEFT方向通过翻转实现
    void playAnimation(const std::string& animType, int frameCount, float delay, bool loop);
    void stopCurrentAnimation();
    void updateSpriteDirection(Direction dir); // 更新图片翻转
    
    // 方向转换辅助函数
    Direction calcDirection(const cocos2d::Vec2& from, const cocos2d::Vec2& to);

    // 内部行为逻辑
    void findTarget();            // 寻敌逻辑
    void moveToTarget(float dt);  // 移动逻辑
    void attackTarget();          // 攻击逻辑
    void updateHealthBar(bool animate = true); // 血条更新
};

#endif  // __SOLDIER_H__