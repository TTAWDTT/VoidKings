# Void Kings 开发指南

## 目录

1. [资源管理与替换](#1-资源管理与替换)
2. [类的修改指南](#2-类的修改指南)
3. [常用函数速查](#3-常用函数速查)
4. [进一步优化方略](#4-进一步优化方略)
5. [调试与问题排查](#5-调试与问题排查)

---

## 1. 资源管理与替换

### 1.1 资源目录结构

```
Resources/
├── config/                 # 配置文件
│   ├── buildings.json      # 建筑配置
│   ├── units.json          # 兵种配置
│   ├── potions.json        # 药水配置
│   └── settings.json       # 游戏设置
├── fonts/                  # 字体文件
│   └── Marker Felt.ttf
├── res/                    # 新增资源存放目录
│   └── .gitkeep
├── CloseNormal.png         # 默认关闭按钮
├── CloseSelected.png       # 选中状态关闭按钮
└── HelloWorld.png          # 示例图片
```

### 1.2 添加新纹理资源

#### 步骤1：准备资源文件

将图片资源放入对应目录：
- 建筑纹理：`Resources/res/buildings/`
- 单位纹理：`Resources/res/units/`
- UI纹理：`Resources/res/ui/`
- 特效纹理：`Resources/res/effects/`

**推荐格式**：PNG（支持透明度）
**推荐尺寸**：2的幂次方（如64x64, 128x128）以获得最佳性能

#### 步骤2：在代码中加载资源

**加载精灵示例**：
```cpp
// 方式1：直接创建精灵
auto sprite = Sprite::create("res/buildings/cannon.png");
if (sprite) {
    this->addChild(sprite);
}

// 方式2：使用SpriteFrameCache（推荐用于动画）
auto cache = SpriteFrameCache::getInstance();
cache->addSpriteFramesWithFile("res/ui/buttons.plist");
auto button = Sprite::createWithSpriteFrameName("button_normal.png");
```

#### 步骤3：修改现有类以使用新资源

以修改 `Building.cpp` 为例：

```cpp
void Building::initSprite() {
    std::string texturePath = "res/buildings/";  // 修改为新的资源路径
    
    switch (_buildingType) {
        case BuildingType::CANNON:
            texturePath += "cannon.png";
            break;
        // ... 其他建筑类型
    }
    
    _sprite = Sprite::create(texturePath);
    if (_sprite) {
        // 资源加载成功
        float width = _gridWidth * GRID_SIZE;
        float height = _gridHeight * GRID_SIZE;
        _sprite->setContentSize(Size(width, height));
        this->addChild(_sprite, 1);
    } else {
        // 资源不存在，使用占位图形
        createPlaceholderGraphic();
    }
}
```

### 1.3 添加新音效/音乐

```cpp
// 播放背景音乐
#include "audio/include/AudioEngine.h"

// 播放背景音乐（循环）
int bgmId = AudioEngine::play2d("res/audio/bgm.mp3", true, 0.5f);

// 播放音效（一次）
AudioEngine::play2d("res/audio/attack.wav", false, 1.0f);

// 停止背景音乐
AudioEngine::stop(bgmId);
```

### 1.4 配置文件修改

JSON配置文件支持热更新，修改配置文件后重启游戏即可生效。

**修改建筑配置示例** (`Resources/config/buildings.json`)：
```json
{
    "type": "CANNON",
    "name": "Cannon",
    "name_cn": "加农炮",
    "gridWidth": 3,
    "gridHeight": 3,
    "texturePath": "res/buildings/cannon.png",  // 修改纹理路径
    "levels": [
        {"level": 1, "hp": 400, "damage": 60, ...}
    ]
}
```

---

## 2. 类的修改指南

### 2.1 核心类层次结构

```
cocos2d::Node
├── Building (建筑基类)
│   ├── DefenseBuilding (防御建筑)
│   ├── ProductionBuilding (生产建筑)
│   ├── StorageBuilding (存储建筑)
│   └── Barracks (兵营)
├── Unit (单位基类)
├── Projectile (投射物)
├── Potion (药水)
└── Scene
    ├── MainMenuScene (主菜单)
    ├── BaseScene (基地场景)
    └── BattleScene (战斗场景)
```

### 2.2 添加新建筑类型

#### 步骤1：在 `GameDefines.h` 中添加枚举

```cpp
enum class BuildingType {
    // ... 现有类型
    INFERNO_TOWER,     // 新增：地狱塔
    COUNT
};
```

#### 步骤2：在基类 `Building.cpp` 中添加属性初始化

```cpp
void Building::initAttributes() {
    switch (_buildingType) {
        // ... 现有类型
        
        case BuildingType::INFERNO_TOWER:
            _buildingName = "Inferno Tower";
            _gridWidth = 2;
            _gridHeight = 2;
            _maxHP = 800;
            _goldCost = 5000;
            _buildTime = 300;
            _maxLevel = 8;
            break;
    }
}
```

#### 步骤3：如果需要特殊行为，创建派生类

```cpp
// InfernoTower.h
class InfernoTower : public DefenseBuilding {
public:
    static InfernoTower* create(Faction faction);
    
    // 地狱塔特殊能力：持续攻击伤害递增
    void updateAttack(float dt) override;
    
private:
    float _damageMultiplier = 1.0f;
    float _attackDuration = 0.0f;
};
```

### 2.3 添加新兵种类型

#### 步骤1：在 `GameDefines.h` 中添加枚举

```cpp
enum class UnitType {
    // ... 现有类型
    PEKKA,             // 新增：皮卡
    COUNT
};
```

#### 步骤2：在 `Unit.cpp` 中添加属性

```cpp
void Unit::initAttributes() {
    switch (_unitType) {
        // ... 现有类型
        
        case UnitType::PEKKA:
            _unitName = "P.E.K.K.A";
            _maxHP = 600;
            _damage = 150;
            _attackRange = 40.0f;
            _attackSpeed = 0.5f;
            _moveSpeed = 40.0f;
            _isAirUnit = false;
            _population = 25;
            _targetPriority = TargetPriority::NEAREST;
            break;
    }
}
```

#### 步骤3：更新配置文件

在 `Resources/config/units.json` 中添加新兵种配置。

### 2.4 修改场景布局

**修改UI布局的推荐方法**：

```cpp
void BaseScene::createResourceDisplay() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // 使用相对定位
    float panelWidth = 400;
    float panelHeight = 50;
    float margin = 10;
    
    // 左上角定位
    _resourcePanel->setPosition(
        Vec2(origin.x + margin, 
             origin.y + visibleSize.height - panelHeight - margin)
    );
    
    // 居中定位
    _buildMenu->setPosition(
        Vec2(origin.x + visibleSize.width / 2 - panelWidth / 2,
             origin.y + visibleSize.height / 2 - panelHeight / 2)
    );
}
```

---

## 3. 常用函数速查

### 3.1 坐标与位置

```cpp
// 获取可视区域大小
auto visibleSize = Director::getInstance()->getVisibleSize();
auto origin = Director::getInstance()->getVisibleOrigin();

// 屏幕坐标转节点坐标
Vec2 nodePos = node->convertToNodeSpace(touchLocation);

// 世界坐标转换
Vec2 worldPos = node->convertToWorldSpace(localPos);

// 格子坐标转像素坐标
float pixelX = gridX * GRID_SIZE + (gridWidth * GRID_SIZE) / 2;
float pixelY = gridY * GRID_SIZE + (gridHeight * GRID_SIZE) / 2;
```

### 3.2 动画与动作

```cpp
// 移动动画
auto moveTo = MoveTo::create(duration, targetPos);
auto moveBy = MoveBy::create(duration, Vec2(dx, dy));

// 缩放动画
auto scaleTo = ScaleTo::create(duration, scale);
auto scaleBy = ScaleBy::create(duration, scaleX, scaleY);

// 淡入淡出
auto fadeIn = FadeIn::create(duration);
auto fadeOut = FadeOut::create(duration);

// 组合动画
auto spawn = Spawn::create(moveTo, fadeIn, nullptr);
auto sequence = Sequence::create(moveTo, scaleTo, callback, nullptr);
auto repeat = RepeatForever::create(sequence);

// 回调函数
auto callback = CallFunc::create([this]() {
    // 动画完成后的操作
});

// 延迟
auto delay = DelayTime::create(seconds);
```

### 3.3 绘图与UI

```cpp
// 创建绘图节点
auto drawNode = DrawNode::create();

// 绘制矩形
drawNode->drawSolidRect(Vec2(0, 0), Vec2(width, height), Color4F::RED);
drawNode->drawRect(Vec2(0, 0), Vec2(width, height), Color4F::WHITE);

// 绘制圆形
drawNode->drawSolidCircle(center, radius, 0, segments, Color4F::BLUE);
drawNode->drawCircle(center, radius, 0, segments, false, Color4F::WHITE);

// 绘制线条
drawNode->drawLine(startPos, endPos, Color4F::GREEN);

// 绘制多边形
Vec2 vertices[] = {Vec2(0, 0), Vec2(100, 0), Vec2(50, 100)};
drawNode->drawPolygon(vertices, 3, fillColor, borderWidth, borderColor);

// 清除绘制内容
drawNode->clear();
```

### 3.4 标签与文本

```cpp
// 创建系统字体标签
auto label = Label::createWithSystemFont("Hello", "Arial", 24);

// 创建TTF字体标签
auto label = Label::createWithTTF("Hello", "fonts/Marker Felt.ttf", 24);

// 设置标签属性
label->setColor(Color3B::WHITE);
label->setOpacity(200);
label->enableShadow(Color4B::BLACK, Size(2, -2), 3);
label->enableOutline(Color4B::BLACK, 2);

// 更新文本
label->setString("New Text");
```

### 3.5 事件监听

```cpp
// 触摸事件
auto touchListener = EventListenerTouchOneByOne::create();
touchListener->setSwallowTouches(true);
touchListener->onTouchBegan = [](Touch* touch, Event* event) -> bool {
    return true;  // 返回true表示处理此事件
};
touchListener->onTouchMoved = [](Touch* touch, Event* event) {
    // 处理移动
};
touchListener->onTouchEnded = [](Touch* touch, Event* event) {
    // 处理结束
};
_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

// 鼠标事件
auto mouseListener = EventListenerMouse::create();
mouseListener->onMouseScroll = [](Event* event) {
    auto mouseEvent = static_cast<EventMouse*>(event);
    float scrollY = mouseEvent->getScrollY();
};
_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

// 键盘事件
auto keyboardListener = EventListenerKeyboard::create();
keyboardListener->onKeyPressed = [](EventKeyboard::KeyCode keyCode, Event* event) {
    if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE) {
        // 处理ESC键
    }
};
_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
```

### 3.6 定时器与更新

```cpp
// 启用帧更新
scheduleUpdate();  // 每帧调用 update(float dt)

// 自定义定时器
schedule([this](float dt) {
    // 每0.5秒执行一次
}, 0.5f, "timer_key");

// 延迟执行一次
scheduleOnce([this](float dt) {
    // 2秒后执行一次
}, 2.0f, "once_key");

// 取消定时器
unschedule("timer_key");
unscheduleUpdate();
```

### 3.7 场景管理

```cpp
// 切换场景
auto newScene = NewScene::createScene();
Director::getInstance()->replaceScene(newScene);

// 带过渡效果的切换
auto transition = TransitionFade::create(0.5f, newScene, Color3B::BLACK);
Director::getInstance()->replaceScene(transition);

// 推入新场景（可返回）
Director::getInstance()->pushScene(newScene);

// 弹出场景
Director::getInstance()->popScene();
```

### 3.8 资源管理器API

```cpp
// 获取单例
auto rm = ResourceManager::getInstance();

// 资源操作
int gold = rm->getResource(ResourceType::GOLD);
rm->addResource(ResourceType::GOLD, amount);
rm->consumeResource(ResourceType::GOLD, amount);
bool hasEnough = rm->hasEnoughResources(goldNeeded, elixirNeeded);

// 工人管理
bool success = rm->useWorker();
rm->releaseWorker();
int available = rm->getAvailableWorkers();

// 人口管理
bool success = rm->usePopulation(amount);
rm->releasePopulation(amount);
int current = rm->getCurrentPopulation();
int max = rm->getMaxPopulation();
```

---

## 4. 进一步优化方略

### 4.1 性能优化

#### 纹理优化
```cpp
// 使用纹理图集（Texture Atlas）减少draw call
auto cache = SpriteFrameCache::getInstance();
cache->addSpriteFramesWithFile("textures/atlas.plist");

// 使用压缩纹理格式
// 在构建时将PNG转换为PVR/ETC2格式
```

#### 批量渲染
```cpp
// 使用SpriteBatchNode批量渲染相同纹理
auto batchNode = SpriteBatchNode::create("textures/units.png");
this->addChild(batchNode);

for (int i = 0; i < 100; i++) {
    auto sprite = Sprite::createWithTexture(batchNode->getTexture());
    batchNode->addChild(sprite);
}
```

#### 对象池模式
```cpp
// 实现对象池以复用频繁创建/销毁的对象
class ProjectilePool {
public:
    static ProjectilePool* getInstance();
    
    Projectile* getProjectile(ProjectileType type);
    void returnProjectile(Projectile* proj);
    
private:
    std::vector<Projectile*> _pool;
};
```

### 4.2 代码结构优化

#### 使用组件模式
```cpp
// 将功能拆分为独立组件
class HealthComponent : public Component {
public:
    void takeDamage(int damage);
    void heal(int amount);
private:
    int _currentHP;
    int _maxHP;
};

class AttackComponent : public Component {
public:
    void attack(Node* target);
private:
    int _damage;
    float _range;
};

// 在Unit中使用组件
auto unit = Unit::create();
unit->addComponent(HealthComponent::create(100, 100));
unit->addComponent(AttackComponent::create(20, 50.0f));
```

#### 配置驱动设计
```cpp
// 从JSON加载配置而不是硬编码
void Unit::initFromConfig(const std::string& configPath) {
    std::string content = FileUtils::getInstance()->getStringFromFile(configPath);
    rapidjson::Document doc;
    doc.Parse(content.c_str());
    
    _maxHP = doc["hp"].GetInt();
    _damage = doc["damage"].GetInt();
    // ...
}
```

### 4.3 用户体验优化

#### 添加加载界面
```cpp
class LoadingScene : public Scene {
public:
    static Scene* createScene();
    bool init() override;
    
private:
    void loadResources();
    void onLoadComplete();
    
    Label* _progressLabel;
    int _totalResources;
    int _loadedResources;
};
```

#### 添加教程系统
```cpp
class TutorialManager {
public:
    static TutorialManager* getInstance();
    
    void startTutorial(const std::string& tutorialId);
    void nextStep();
    bool isCompleted(const std::string& tutorialId);
    
private:
    void showHighlight(Node* target);
    void showTip(const std::string& text, const Vec2& position);
};
```

### 4.4 扩展功能建议

1. **存档系统**
   - 使用UserDefault保存简单数据
   - 使用JSON文件保存复杂存档

2. **多语言支持**
   - 创建语言配置文件
   - 使用键值对加载文本

3. **成就系统**
   - 定义成就条件
   - 监听游戏事件触发成就

4. **排行榜系统**
   - 本地排行榜使用文件存储
   - 在线排行榜需要服务器支持

---

## 5. 调试与问题排查

### 5.1 常见问题

#### 资源加载失败
```cpp
// 检查资源路径
std::string fullPath = FileUtils::getInstance()->fullPathForFilename("res/test.png");
CCLOG("Full path: %s", fullPath.c_str());

// 检查文件是否存在
bool exists = FileUtils::getInstance()->isFileExist("res/test.png");
```

#### 内存泄漏
```cpp
// 使用autorelease自动管理内存
auto sprite = Sprite::create("test.png");  // 自动autorelease

// 手动管理时注意配对
node->retain();
// ... 使用node
node->release();

// 检查引用计数
CCLOG("Ref count: %d", node->getReferenceCount());
```

#### 性能问题
```cpp
// 显示FPS和draw call信息
Director::getInstance()->setDisplayStats(true);

// 使用profiler
// 在Debug模式下查看Console输出
```

### 5.2 调试工具

```cpp
// 日志输出
CCLOG("Debug message: %d", value);
CCLOGWARN("Warning message");
CCLOGERROR("Error message");

// 断言
CCASSERT(pointer != nullptr, "Pointer should not be null");

// 在Visual Studio中：
// - 设置断点
// - 查看Watch窗口
// - 使用内存诊断工具
```

### 5.3 构建问题

#### Visual Studio构建失败
1. 右键解决方案 -> 重定解决方案目标
2. 清理解决方案后重新构建
3. 检查项目属性中的包含目录和库目录

#### CMake构建失败
```bash
# 删除build目录重新构建
rm -rf build
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A Win32
cmake --build . --config Release
```

---

## 附录：快捷键

| 快捷键 | 功能 |
|-------|------|
| F5 | 运行（Visual Studio） |
| F7 | 构建（Visual Studio） |
| Ctrl+Shift+B | 构建解决方案 |

---

*最后更新：2025年12月*
