#include "cocos2d.h"
#include "OthersBuildingData.h"

// 此处开始写其他建筑类
class OthersBuilding : public cocos2d::Node {
public:
    // 标准 Cocos create 方法
	static OthersBuilding* create(const OthersBuildingConfig* config, int level = 0); // 创建建筑实例,默认等级为0
    virtual bool init(const OthersBuildingConfig* config, int level = 0); // 初始化、绑定子节点
    virtual void update(float dt) override;

    // 状态控制
    void takeDamage(float damage);
    
    // 等级相关方法
    int getLevel() const { return _level; }
    void setLevel(int level);
    // 运行时数据
    int _level;                   // 当前等级
    float _currentHP;
    cocos2d::Sprite* _bodySprite; // 以后会定义这个为动画,暂时应该渲染成图片
    cocos2d::Sprite* _healthBar;  // 血条精灵
    // 动画支持
    std::string _currentActionKey;     // 当前动画的键
    // 获取当前等级的属性
    float getCurrentMaxHP() const;
    float getCurrentDP() const;
    
    //获取占地尺寸
    int getLength() const;
    int getWidth() const;

private:
    // 核心：持有一个指向配置的指针 (享元模式,不要拷贝整个结构体)
    const OthersBuildingConfig* _config;

    // 运行时数据
    int _level;                   // 当前等级
    float _currentHP;
    cocos2d::Sprite* _bodySprite; // 这个会定义它作为精灵,当时应渲染的图片

    // 内部行为逻辑
    void getAttacked(float damage); // 受击逻辑
    void die();                     // 死亡逻辑
    void updateHealthBar(bool animate = true); // 血条更新
    void button();                  // 按钮逻辑
};