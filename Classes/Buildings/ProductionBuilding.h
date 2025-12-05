#include "cocos2d.h"
#include "ProductionBuildingData.h"

// 此处开始写生产建筑类
class ProductionBuilding : public cocos2d::Node {
public:
    // 标准 Cocos create 方法
	static ProductionBuilding* create(const ProductionBuildingConfig* config, int level = 0); // 创建建筑实例,默认等级为0
    virtual bool init(const ProductionBuildingConfig* config, int level = 0); // 初始化、绑定子节点
    virtual void update(float dt) override;

    // 状态控制
    void takeDamage(float damage);
    
    // 等级相关方法
    int getLevel() const { return _level; }
    void setLevel(int level);
    // 获取当前血量
    float getCurrentHP() const;
    // 获取当前等级的属性
    float getCurrentMaxHP() const;
    float getCurrentDP() const;
    float getCurrentPRODUCE_ELIXIR() const;
    float getCurrentSTORAGE_ELIXIR_CAPACITY() const;
    float getCurrentPRODUCE_GOLD() const;
    float getCurrentSTORAGE_GOLD_CAPACITY() const;

    //获取占地尺寸
    int getLength() const;
    int getWidth() const;


private:
    // 核心：持有一个指向配置的指针 (享元模式,不要拷贝整个结构体)
    const ProductionBuildingConfig* _config;

    // 运行时数据
    int _level;                   // 当前等级
    float _currentHP;
    cocos2d::Sprite* _bodySprite; // 这个会定义它作为精灵,当时应渲染的图片

    // 内部行为逻辑
    void produce(int dt);           // 生产与存储逻辑
    void getAttacked(float damage); // 受击逻辑
    void die();                     // 死亡逻辑
    void bar();                     // 显示血条
    void button();                  // 按钮逻辑
};