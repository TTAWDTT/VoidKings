#include "cocos2d.h"
#include "StorageBuildingData.h"

// 此处开始写存储建筑类
class StorageBuilding : public cocos2d::Node {
public:
    // 标准 Cocos create 方法
	static StorageBuilding* create(const StorageBuildingConfig* config, int level = 0); // 创建建筑实例,默认等级为0
    virtual bool init(const StorageBuildingConfig* config, int level = 0); // 初始化、绑定子节点
    virtual void update(float dt) override;

    // 状态控制
    void takeDamage(float damage);
    
    // 等级相关方法
    int getLevel() const { return _level; }
    void setLevel(int level);
    
    // 获取当前等级的属性
    float getCurrentHP() const;
    float getCurrentMaxHP() const;
    float getCurrentDP() const;
    float getCurrentADD_STORAGE_ELIXIR_CAPACITY() const;
    float getCurrentADD_STORAGE_GOLD_CAPACITY() const;
    
    //获取占地尺寸
    int getLength() const;
    int getWidth() const;

private:
    // 核心：持有一个指向配置的指针 (享元模式,不要拷贝整个结构体)
    const StorageBuildingConfig* _config;

    // 运行时数据
    int _level;                   // 当前等级
    float _currentHP;
    cocos2d::Sprite* _bodySprite; // 这个会定义它作为精灵,当时应渲染的图片

    // 内部行为逻辑
    void storage();                 // 存储逻辑
    void getAttacked(float damage); // 受击逻辑
    void die();                     // 死亡逻辑
    void bar();                     // 显示血条
    void button();                  // 按钮逻辑
};