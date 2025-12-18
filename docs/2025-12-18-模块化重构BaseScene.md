# BaseScene 场景模块化重构说明

## 📋 文档信息
- **创建日期**: 2025年12月18日
- **项目**: VoidKings
- **版本**: v1.1.0

---

## 🎯 改动目标

根据需求进行以下改进：

1. **场景模块化** - 将BaseScene中的各功能提取为独立组件
2. **修复建筑格子占用** - 按照部落冲突设定调整建筑尺寸
3. **实现格子颜色提示** - 建造时显示绿色/红色格子指示可否放置

---

## 🛠️ 模块化架构

### 新增组件目录

```
Classes/Scenes/Components/
├── BaseUIPanel.cpp/.h         # UI面板组件（进攻、建造、退出按钮）
├── BuildShopPanel.cpp/.h      # 建筑商店面板组件
├── PlacementManager.cpp/.h    # 建筑放置管理器组件
└── GridBackground.cpp/.h      # 网格背景组件
```

### 组件职责

| 组件 | 职责 | 主要功能 |
|------|------|---------|
| `BaseUIPanel` | UI管理 | 进攻/建造/退出按钮，资源显示 |
| `BuildShopPanel` | 建筑商店 | 显示建筑列表，选择建筑 |
| `PlacementManager` | 放置管理 | 预览显示，格子颜色，放置确认 |
| `GridBackground` | 背景绘制 | 黑白虚线网格绘制 |

### BaseScene 改动

重构后的 `BaseScene` 只负责协调各组件的初始化和交互：

```cpp
// 核心组件（模块化）
GridMap* _gridMap;                     // 网格地图
GridBackground* _gridBackground;       // 网格背景组件
BaseUIPanel* _uiPanel;                 // UI面板组件
BuildShopPanel* _buildShopPanel;       // 建筑商店组件
PlacementManager* _placementManager;   // 放置管理器组件
TrainPanel* _trainPanel;               // 训练面板
```

---

## 🏗️ 建筑格子尺寸修复

### 原问题

原代码中建筑尺寸设定与部落冲突不符，导致格子占用不合理。

### 修复方案

按照部落冲突的设定调整建筑尺寸：

| 建筑类型 | 原尺寸 | 新尺寸 | 说明 |
|---------|--------|--------|------|
| 箭塔 (Arrow Tower) | 2x2 | **3x3** | 防御塔标准尺寸 |
| 炮塔 (Boom Tower) | 2x2 | **3x3** | 防御塔标准尺寸 |
| 装饰树 (Tree) | 2x2 | 2x2 | 装饰物保持不变 |
| 雪人仓库 (Snowman) | 2x2 | **3x3** | 资源建筑标准尺寸 |
| 兵营 (Soldier Builder) | 3x3 | **5x5** | 兵营较大 |
| 大本营 (Base) | 4x4 | 4x4 | 保持不变 |

### 代码位置

建筑尺寸配置在 `BuildShopPanel.cpp` 的 `initBuildingOptions()` 方法中：

```cpp
void BuildShopPanel::initBuildingOptions() {
    _buildingOptions = {
        {1, "Arrow Tower (100G)", 100, 3, 3, "buildings/ArrowTower.png"},
        {2, "Boom Tower (150G)", 150, 3, 3, "buildings/BoomTower.png"},
        {3, "Tree (80G)", 80, 2, 2, "buildings/Tree/sprite_0000.png"},
        {4, "Snowman Storage (200G)", 200, 3, 3, "buildings/snowman.png"},
        {5, "Soldier Builder (300G)", 300, 5, 5, "buildings/soldierbuilder.png"}
    };
}
```

---

## 🎨 格子颜色提示系统

### 功能描述

在建造模式下，地图会显示格子颜色来指示放置状态：

### 全地图格子状态

| 状态 | 颜色 | 说明 |
|------|------|------|
| 空闲 | 淡绿色 (0, 0.5, 0, 0.2) | 可以放置建筑 |
| 已占用 | 灰色 (0.5, 0.5, 0.5, 0.3) | 已有建筑占用 |
| 禁止区域 | 深灰色 (0.3, 0.3, 0.3, 0.3) | 地图边界禁止建造 |

### 当前放置位置

| 状态 | 填充颜色 | 边框颜色 |
|------|---------|---------|
| 可放置 | 绿色 (0, 0.8, 0, 0.4) | 亮绿色 (0, 1, 0, 0.8) |
| 不可放置 | 红色 (0.8, 0, 0, 0.4) | 亮红色 (1, 0, 0, 0.8) |

### 实现位置

颜色配置在 `PlacementManager.h` 的 `PlacementConfig` 命名空间中：

```cpp
namespace PlacementConfig {
    const Color4F GRID_COLOR_CAN_PLACE = Color4F(0.0f, 0.8f, 0.0f, 0.4f);
    const Color4F GRID_COLOR_CANNOT_PLACE = Color4F(0.8f, 0.0f, 0.0f, 0.4f);
    const Color4F GRID_COLOR_OCCUPIED = Color4F(0.5f, 0.5f, 0.5f, 0.3f);
    const Color4F GRID_COLOR_EMPTY = Color4F(0.0f, 0.5f, 0.0f, 0.2f);
    const Color4F GRID_COLOR_FORBIDDEN = Color4F(0.3f, 0.3f, 0.3f, 0.3f);
}
```

---

## 📁 文件变更列表

### 新增文件

- `Classes/Scenes/Components/BaseUIPanel.h`
- `Classes/Scenes/Components/BaseUIPanel.cpp`
- `Classes/Scenes/Components/BuildShopPanel.h`
- `Classes/Scenes/Components/BuildShopPanel.cpp`
- `Classes/Scenes/Components/PlacementManager.h`
- `Classes/Scenes/Components/PlacementManager.cpp`
- `Classes/Scenes/Components/GridBackground.h`
- `Classes/Scenes/Components/GridBackground.cpp`

### 修改文件

- `Classes/Scenes/BaseScene.h` - 重构为使用模块化组件
- `Classes/Scenes/BaseScene.cpp` - 完全重写，使用模块化架构
- `CMakeLists.txt` - 添加新源文件
- `proj.win32/VoidKings.vcxproj` - 添加新源文件
- `proj.win32/VoidKings.vcxproj.filters` - 添加新文件筛选器

---

## 🔧 项目配置更新

### Visual Studio 项目

新文件已添加到 `VoidKings.vcxproj`：

```xml
<ClCompile Include="..\Classes\Scenes\Components\BuildShopPanel.cpp" />
<ClCompile Include="..\Classes\Scenes\Components\PlacementManager.cpp" />
<ClCompile Include="..\Classes\Scenes\Components\BaseUIPanel.cpp" />
<ClCompile Include="..\Classes\Scenes\Components\GridBackground.cpp" />
```

### CMake 构建

新文件已添加到 `CMakeLists.txt` 的源文件列表中。

---

## ✅ 验证步骤

1. **Visual Studio 打开项目**
   - 打开 `proj.win32/VoidKings.sln`
   - 右键解决方案 → 重新生成

2. **验证建造功能**
   - 进入基地场景
   - 点击建造按钮，打开建筑商店
   - 选择一个建筑，观察格子颜色显示
   - 移动鼠标，观察当前位置的绿色/红色指示
   - 在有效位置释放，确认建筑放置成功

3. **验证建筑尺寸**
   - 放置一个箭塔（3x3）
   - 放置一个兵营（5x5）
   - 确认占用格子数符合预期

---

## 📌 后续优化建议

- [ ] 添加建筑拖拽功能（可移动已放置的建筑）
- [ ] 添加建筑升级界面
- [ ] 添加建筑信息提示面板
- [ ] 优化格子颜色渐变动画
- [ ] 添加建造音效

---

## 📚 代码风格说明

本次改动遵循以下代码风格：

- 采用易懂的中文注释
- 使用命名空间管理配置常量
- 组件采用回调函数模式解耦
- 保持与现有代码风格一致

---

*文档创建: Copilot Agent*
*最后更新: 2025年12月18日*
