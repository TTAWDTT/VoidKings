# VoidKings - Building System Implementation

## 项目概述 / Project Overview

VoidKings 是一个类似部落冲突的策略游戏，本次更新实现了完整的建筑系统和基地场景。

VoidKings is a Clash of Clans-like strategy game. This update implements a complete building system and base scene.

## 最新更新 / Latest Updates

### 已完成功能 / Completed Features

1. **核心系统 / Core Systems**
   - ✅ 子弹系统 (Bullet System) - 箭和炸弹
   - ✅ 资源系统 (Resource System) - 金币和钻石，带动画
   - ✅ 网格地图 (GridMap) - 40x40网格，支持建筑放置验证
   - ✅ 建筑管理器 (BuildingManager) - 单例模式，统一管理所有建筑

2. **建筑类型 / Building Types**
   - ✅ 防御建筑 (Defence Buildings): 箭塔、炮塔、树
   - ✅ 生产建筑 (Production Buildings): 大本营、兵营
   - ✅ 仓库建筑 (Storage Buildings): 雪人金库

3. **基地场景 / Base Scene**
   - ✅ 草地背景自动生成
   - ✅ UI按钮 (攻击、建造、退出)
   - ✅ 建筑商店
   - ✅ 建筑放置预览系统
   - ✅ 初始大本营自动放置

## 如何运行 / How to Run

### 方式1：Visual Studio (推荐 / Recommended)
1. 打开 `proj.win32` 文件夹
2. 双击 `VoidKings.sln`
3. 选择 Debug 或 Release 配置
4. 点击运行 (F5)

### 方式2：CMake
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## 游戏操作 / Game Controls

### 主菜单 (Main Menu)
- **Start** - 进入基地场景
- **Settings** - 设置（待实现）
- **Rules** - 规则说明
- **Exit** - 退出游戏

### 基地场景 (Base Scene)
- **Build** - 打开建筑商店
  - 选择建筑类型
  - 拖动到空地上
  - 绿色=可放置，红色=不可放置
- **Attack** - 进入战斗场景（待实现）
- **Exit** - 返回主菜单

## 建筑信息 / Building Information

### 防御建筑 / Defence Buildings

| 建筑 | ID | 大小 | 特性 |
|------|-----|------|------|
| 箭塔 ArrowTower | 2001 | 2x2 | 空地双攻，远程攻击 |
| 炮塔 BoomTower | 2002 | 2x2 | 地面群伤，范围伤害 |
| 树 Tree | 2003 | 2x2 | 毒素伤害，持续掉血 |

### 生产建筑 / Production Buildings

| 建筑 | ID | 大小 | 特性 |
|------|-----|------|------|
| 大本营 Base | 3001 | 4x4 | 资源存储中心 |
| 兵营 SoldierBuilder | 3002 | 3x3 | 训练士兵 |

### 仓库建筑 / Storage Buildings

| 建筑 | ID | 大小 | 特性 |
|------|-----|------|------|
| 雪人 Snowman | 4001 | 2x2 | 增加存储容量 |

## 资源文件 / Resource Files

### 必需资源 / Required Resources
所有必需的资源文件已在 `Resources/` 目录中：
- ✅ 建筑图片 (Building sprites)
- ✅ 草地图块 (Grass tiles)
- ✅ 资源动画 (Resource animations)
- ✅ 子弹图片 (Bullet sprites)
- ✅ 配置文件 (JSON configs)

### 可选资源 / Optional Resources
- ⚠️ `res/health_bar.png` - 血条图片 (当前使用程序生成)

## 项目结构 / Project Structure

```
VoidKings/
├── Classes/              # C++ 源代码
│   ├── Buildings/       # 建筑系统
│   ├── Bullet/          # 子弹系统
│   ├── Map/             # 地图系统
│   ├── Resource/        # 资源系统
│   ├── Scenes/          # 场景
│   ├── Soldier/         # 士兵系统
│   └── Utils/           # 工具类
├── Resources/           # 游戏资源
│   ├── buildings/       # 建筑图片
│   ├── bullet/          # 子弹图片
│   ├── grass/           # 草地图块
│   ├── source/          # 资源图标
│   └── res/             # 配置文件
└── proj.win32/          # Windows 项目
    └── VoidKings.sln
```

## 技术细节 / Technical Details

### 开发环境 / Development Environment
- **引擎**: Cocos2d-x 3.x
- **语言**: C++11
- **IDE**: Visual Studio 2019/2022
- **工具集**: v143 或 v145

### 核心系统设计 / Core System Design

1. **网格系统** (GridMap)
   - 40x40 网格
   - 每格 32 像素
   - 自动坐标转换
   - 建筑占用检测

2. **建筑管理** (BuildingManager)
   - 单例模式
   - 工厂方法创建建筑
   - 统一放置逻辑
   - 与网格系统集成

3. **动画系统** (AnimationUtils)
   - 基于精灵帧
   - 支持循环和单次播放
   - 缓存机制

## 已知问题 / Known Issues

1. 建筑攻击暂未实现子弹发射
2. 资源生产系统暂未启用
3. 建筑动画需要加载对应的精灵帧
4. 缺少存档系统

## 下一步计划 / Next Steps

1. 实现建筑攻击和子弹系统
2. 添加资源生产逻辑
3. 实现建筑升级系统
4. 添加音效
5. 创建建筑销毁特效
6. 添加教程系统

## 贡献者 / Contributors

- TTAWDTT - 项目负责人
- GitHub Copilot - AI 辅助开发

## 许可 / License

本项目使用 Cocos2d-x 引擎，遵循 MIT 许可证。

---

最后更新时间 / Last Updated: 2025-12-11
