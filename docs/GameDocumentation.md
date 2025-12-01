# Void Kings 游戏开发文档

## 项目概述

Void Kings 是一款基于 Cocos2d-x 3.17.2 开发的类《部落冲突》策略塔防游戏。本项目为单机版 Win32 端，支持玩家调整窗口大小。

### 核心玩法
- **基地建设**: 建造各类建筑，发展自己的村庄
- **资源管理**: 管理金币、圣水、工人和人口
- **兵种养成**: 训练各种兵种，组建进攻部队
- **攻防对战**: 进攻敌方基地，掠夺资源

---

## 类功能说明

### 1. 核心系统 (Classes/Core/)

#### GameDefines.h
**功能**: 游戏全局定义和常量
- 设计分辨率常量 (1280x720)
- 地图格子大小和尺寸
- 各种枚举类型定义:
  - `ResourceType`: 资源类型 (金币/圣水/工人/人口)
  - `BuildingType`: 建筑类型
  - `UnitType`: 兵种类型
  - `PotionType`: 药水类型
  - `ProjectileType`: 弹道类型
  - `DamageType`: 伤害类型
  - `TargetPriority`: 目标优先级
  - `Faction`: 阵营
  - `GameState`: 游戏状态
  - `UnitState`: 单位状态
  - `BuildingState`: 建筑状态
- 配置数据结构

#### ResourceManager.h/cpp
**功能**: 资源管理器 (单例模式)
- 资源的生产、存储、消耗、损失和获得
- 人口管理
- 工人管理
- 资源变化回调通知
- UI实时更新支持

### 2. 建筑系统 (Classes/Buildings/)

#### Building.h/cpp
**功能**: 建筑基类
- 所有建筑的基础属性管理
- 血量/耐久系统
- 等级升级系统
- 建造/升级进度
- 拆除返还资源
- UI绑定 (血条、进度条、选中指示器)
- 触摸事件处理

#### DefenseBuilding.h/cpp
**功能**: 防御建筑类
- 继承自 Building
- 攻击范围和可视化
- 索敌系统 (支持多种优先级)
- 攻击冷却机制
- 弹道发射
- 支持建筑类型: 加农炮、箭塔、迫击炮

#### ProductionBuilding.h/cpp
**功能**: 生产建筑类
- 继承自 Building
- 定时资源产出
- 本地资源存储
- 资源收集接口
- 支持建筑类型: 金矿、圣水收集器

#### StorageBuilding.h/cpp
**功能**: 存储建筑类
- 继承自 Building
- 全局存储容量管理
- 升级扩容规则
- 支持建筑类型: 金库、圣水库

#### Barracks.h/cpp
**功能**: 兵营类
- 继承自 Building
- 兵种训练队列
- 训练进度管理
- 等级解锁新兵种
- 人口和资源消耗

### 3. 单位系统 (Classes/Units/)

#### Unit.h/cpp
**功能**: 兵种/单位基类
- 单位属性管理 (血量/攻击力/移速等)
- 状态机 (待机/移动/攻击/死亡)
- 索敌系统
- 路径移动
- 攻击逻辑
- 动画效果
- 支持兵种: 野蛮人、弓箭手、巨人、哥布林、法师、治疗者

### 4. 投射物系统 (Classes/Projectiles/)

#### Projectile.h/cpp
**功能**: 投射物/子弹类
- 多种弹道类型:
  - 直线弹道
  - 抛物线弹道
  - 追踪弹道
- 伤害计算 (单体/范围)
- 命中检测
- 命中特效

### 5. 药水系统 (Classes/Potions/)

#### Potion.h/cpp
**功能**: 药水/法术类
- 范围效果
- 持续时间
- 效果类型:
  - 治疗药水: 范围治疗友军
  - 狂暴药水: 范围增益效果
  - 闪电法术: 即时范围伤害
  - 冰冻法术: 范围冰冻敌人
- 视觉特效

### 6. 场景系统 (Classes/Scenes/)

#### MainMenuScene.h/cpp
**功能**: 主菜单场景
- 游戏标题显示
- 开始游戏按钮
- 设置按钮
- 退出按钮
- 背景动画

#### BaseScene.h/cpp
**功能**: 基地场景
- 地图网格系统
- 建筑放置和管理
- 资源显示
- 建筑菜单
- 建筑信息面板
- 地图拖动和缩放
- 资源收集

#### BattleScene.h/cpp
**功能**: 战斗场景
- 敌方基地生成
- 兵种部署系统
- 战斗进度显示
- 摧毁百分比计算
- 星级评定
- 资源掠夺
- 结算界面

### 7. 应用程序入口

#### AppDelegate.h/cpp
**功能**: 应用程序代理
- OpenGL上下文初始化
- 窗口创建和配置
- 设计分辨率设置 (1280x720)
- 窗口大小调整支持
- 场景启动

---

## 资源来源说明

### 1. 字体资源 (Resources/fonts/)
- `Marker Felt.ttf`: Cocos2d-x 默认字体
- `arial.ttf`: 系统字体

### 2. 图片资源
本项目当前使用程序化绘制的占位图形，未使用外部图片资源。
所有视觉元素通过 Cocos2d-x 的 DrawNode 组件绘制:
- 建筑使用彩色矩形表示
- 单位使用彩色圆形表示
- UI元素使用矩形和文字组合

### 3. 如需添加自定义图片资源
将图片放入以下目录:
- `Resources/textures/buildings/` - 建筑纹理
- `Resources/textures/units/` - 单位纹理
- `Resources/textures/ui/` - UI纹理

---

## 修改指南

### 1. 修改游戏参数

#### 修改设计分辨率
文件: `Classes/Core/GameDefines.h`
```cpp
const float DESIGN_WIDTH = 1280.0f;
const float DESIGN_HEIGHT = 720.0f;
```

#### 修改地图大小
文件: `Classes/Core/GameDefines.h`
```cpp
const int GRID_SIZE = 32;
const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 30;
```

#### 修改初始资源
文件: `Classes/Core/ResourceManager.cpp`
在 `initDefaultResources()` 函数中修改:
```cpp
_resources[ResourceType::GOLD] = 500;     // 初始金币
_resources[ResourceType::ELIXIR] = 500;   // 初始圣水
_maxResources[ResourceType::GOLD] = 5000; // 金币上限
```

### 2. 添加新建筑类型

1. 在 `GameDefines.h` 的 `BuildingType` 枚举中添加新类型
2. 在 `Building.cpp` 的 `initAttributes()` 中添加属性配置
3. 在 `Building.cpp` 的 `initSprite()` 中添加纹理路径
4. 根据需要创建派生类 (如防御/生产/存储)

### 3. 添加新兵种

1. 在 `GameDefines.h` 的 `UnitType` 枚举中添加新类型
2. 在 `Unit.cpp` 的 `initAttributes()` 中添加属性配置
3. 在 `Unit.cpp` 的 `initSprite()` 中添加纹理路径
4. 在 `Barracks.cpp` 的相关函数中添加训练配置

### 4. 添加新药水

1. 在 `GameDefines.h` 的 `PotionType` 枚举中添加新类型
2. 在 `Potion.cpp` 的 `initPotionAttributes()` 中添加属性配置
3. 在 `Potion.cpp` 中添加效果实现函数

### 5. 修改UI布局

#### 主菜单
文件: `Classes/Scenes/MainMenuScene.cpp`
- `createBackground()`: 修改背景
- `createTitle()`: 修改标题样式
- `createMenuButtons()`: 修改按钮布局

#### 基地界面
文件: `Classes/Scenes/BaseScene.cpp`
- `createResourceDisplay()`: 修改资源显示
- `createToolbar()`: 修改工具栏
- `createBuildMenu()`: 修改建筑菜单

#### 战斗界面
文件: `Classes/Scenes/BattleScene.cpp`
- `createUnitBar()`: 修改兵种选择栏
- `createBattleInfo()`: 修改战斗信息显示
- `showResultPanel()`: 修改结算界面

### 6. 添加自定义纹理

1. 将图片文件放入 `Resources/textures/` 对应目录
2. 修改对应类的 `initSprite()` 函数
3. 使用 `Sprite::create("textures/xxx/image.png")` 加载

### 7. 修改窗口大小调整行为

文件: `Classes/AppDelegate.cpp`
```cpp
// 修改是否可调整大小
glview = GLViewImpl::createWithRect("Void Kings", 
    cocos2d::Rect(0, 0, width, height),
    1.0f,
    true);  // true=可调整, false=固定大小
```

---

## 编译说明

### Visual Studio 编译
1. 打开 `proj.win32/VoidKings.sln`
2. 设置为 Debug 或 Release 配置
3. 右键项目 -> 生成

### CMake 编译
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A Win32
cmake --build . --config Release
```

---

## 注意事项

1. **编码问题**: 所有源文件使用 UTF-8 编码，中文注释在 VS 中可能需要设置正确的编码
2. **资源路径**: 资源文件在运行时会被复制到输出目录的 Resources 文件夹
3. **窗口大小**: 窗口可自由调整，UI会自动适配
4. **性能优化**: 当前使用程序化绘图，如需提升性能可替换为预渲染纹理

---

## 版本历史

### v1.0.0
- 初始版本
- 实现基地建设系统
- 实现资源管理系统
- 实现兵种训练系统
- 实现战斗系统
- 支持窗口大小调整
