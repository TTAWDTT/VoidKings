# 附录D：代码规范

> **本附录**：VoidKings 项目的代码风格规范。

---

## 命名规范

### 类名

- 使用 **PascalCase**（大驼峰）
- 名词或名词短语

```cpp
// 正确
class Soldier;
class DefenceBuilding;
class BattleScene;
class SaveManager;

// 错误
class soldier;         // ❌ 小写开头
class defence_building; // ❌ 下划线
class Battle;          // ❌ 动词
```

### 函数名

- 使用 **camelCase**（小驼峰）
- 动词或动词短语

```cpp
// 正确
void update(float dt);
bool canPlaceBuilding(int x, int y);
Soldier* findNearestTarget();
void playAnimation(AnimationType type);

// 错误
void Update(float dt);      // ❌ 大写开头
void can_place();           // ❌ 下划线
```

### 变量名

- 成员变量：`_camelCase`（下划线前缀）
- 局部变量：`camelCase`
- 静态变量：`s_camelCase`
- 全局常量：`UPPER_SNAKE_CASE`

```cpp
class Soldier {
private:
    float _currentHP;          // 成员变量
    int _level;
    static Soldier* s_instance; // 静态成员
};

void someFunction() {
    int localCount = 0;        // 局部变量
    const int MAX_COUNT = 100; // 常量
}
```

### 枚举

- 枚举类型：`PascalCase`
- 枚举值：`UPPER_SNAKE_CASE` 或 `PascalCase`

```cpp
// 推荐（C++11 enum class）
enum class ResourceType {
    COIN,
    DIAMOND
};

enum class AnimationType {
    IDLE,
    WALK,
    ATTACK,
    DEATH
};
```

---

## 文件组织

### 头文件结构

```cpp
#ifndef __SOLDIER_H__    // 头文件保护
#define __SOLDIER_H__

#include "cocos2d.h"      // 引擎头文件
#include "UnitData.h"     // 项目头文件

USING_NS_CC;              // 命名空间

// 前向声明
class Bullet;
class DefenceBuilding;

// 类定义
class Soldier : public Node {
public:
    // 静态工厂方法
    static Soldier* create(const UnitConfig* config, int level);
    
    // 公共方法
    bool init(const UnitConfig* config, int level);
    void update(float dt) override;
    
    // Getter/Setter
    float getCurrentHP() const { return _currentHP; }
    bool isDead() const { return _currentHP <= 0; }
    
protected:
    // 继承接口
    void onEnter() override;
    void onExit() override;
    
private:
    // 私有辅助方法
    void findTarget();
    void moveToTarget(float dt);
    
    // 成员变量
    const UnitConfig* _config;
    float _currentHP;
    int _level;
};

#endif
```

### 源文件结构

```cpp
#include "Soldier.h"
#include "Bullet.h"
#include "Utils/AnimationUtils.h"
#include "Utils/AudioManager.h"

// 匿名命名空间用于文件内常量和辅助函数
namespace {
    constexpr float TARGET_REFRESH_INTERVAL = 0.25f;
    constexpr float MIN_ATTACK_DISTANCE = 5.0f;
}

// 静态成员初始化
std::vector<Soldier*>* Soldier::s_enemySoldiers = nullptr;

// 工厂方法
Soldier* Soldier::create(const UnitConfig* config, int level) {
    Soldier* ret = new (std::nothrow) Soldier();
    if (ret && ret->init(config, level)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

// 成员函数实现
bool Soldier::init(const UnitConfig* config, int level) {
    if (!Node::init()) return false;
    
    _config = config;
    _level = level;
    _currentHP = getCurrentMaxHP();
    
    // ...
    return true;
}
```

---

## 注释规范

### 文件头注释

```cpp
/**
 * @file Soldier.cpp
 * @brief 士兵类实现
 * @author Your Name
 * @date 2024-12-28
 */
```

### 类注释

```cpp
/**
 * @brief 士兵基类
 * 
 * 负责士兵的移动、攻击、动画等逻辑。
 * 使用享元模式共享配置数据。
 */
class Soldier : public Node {
    // ...
};
```

### 函数注释

```cpp
/**
 * @brief 查找攻击目标
 * @return 找到的目标建筑，未找到返回 nullptr
 * 
 * 按距离优先选择最近的有效目标。
 * 考虑 AI 类型偏好（防御/资源/任意）。
 */
Node* Soldier::findTarget();
```

### 行内注释

```cpp
void Soldier::update(float dt) {
    if (isDead()) return;
    
    // 每 0.25 秒刷新目标
    _targetTimer += dt;
    if (_targetTimer >= 0.25f) {
        _targetTimer = 0.0f;
        refreshTarget();
    }
    
    // 移动或攻击
    if (_target) {
        float dist = getDistanceToTarget();
        if (dist <= getAttackRange()) {
            attackTarget(dt);  // 在攻击范围内
        } else {
            moveToTarget(dt);   // 靠近目标
        }
    }
}
```

---

## 最佳实践

### 使用 const

```cpp
// 不修改成员的函数标记为 const
float getCurrentHP() const { return _currentHP; }
bool isDead() const { return _currentHP <= 0; }

// 不修改的参数使用 const 引用
void setConfig(const UnitConfig* config);
void processEvent(const ReplayDeployEvent& event);
```

### 使用 auto

```cpp
// 类型明显时使用 auto
auto sprite = Sprite::create("player.png");
auto director = Director::getInstance();

// 迭代器
for (auto& soldier : _soldiers) {
    soldier->update(dt);
}

// 但类型不明显时显式声明
int count = getCount();  // 不要用 auto
```

### 使用智能指针（可选）

Cocos2d-x 有自己的引用计数系统，但 STL 智能指针可用于非 Node 对象：

```cpp
// 用于非 Cocos2d-x 对象
std::unique_ptr<Config> _config;
std::shared_ptr<NetworkManager> _network;

// Cocos2d-x 对象使用 retain/release
Sprite* _sprite;  // 需要手动管理或依赖父节点
```

### 避免魔法数字

```cpp
// 错误
if (_timer >= 0.25f) { ... }

// 正确
constexpr float TARGET_REFRESH_INTERVAL = 0.25f;
if (_timer >= TARGET_REFRESH_INTERVAL) { ... }
```

### 早期返回

```cpp
// 好
void Soldier::update(float dt) {
    if (isDead()) return;
    if (!_target) return;
    
    // 主逻辑...
}

// 不好（嵌套过深）
void Soldier::update(float dt) {
    if (!isDead()) {
        if (_target) {
            // 主逻辑...
        }
    }
}
```

---

## 代码格式化

### 缩进

- 使用 4 空格缩进
- 不使用 Tab

### 大括号

```cpp
// 同一行开始
if (condition) {
    // ...
}

// 函数
void doSomething() {
    // ...
}

// 类
class MyClass {
public:
    // ...
};
```

### 空格

```cpp
// 运算符两侧
int x = a + b;
bool result = x > 0 && x < 100;

// 逗号后
func(a, b, c);

// 控制语句
if (condition) { }
for (int i = 0; i < n; ++i) { }
while (running) { }
```

---

## 错误处理

```cpp
// 使用 CCLOG 记录错误
if (!config) {
    CCLOG("[Soldier] 配置为空");
    return false;
}

// 返回 nullptr 表示失败
Soldier* Soldier::create(...) {
    Soldier* ret = new (std::nothrow) Soldier();
    if (ret && ret->init(...)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;  // 创建失败
}

// 调用方检查返回值
auto* soldier = Soldier::create(config, level);
if (!soldier) {
    CCLOG("[BattleScene] 士兵创建失败");
    return;
}
```
