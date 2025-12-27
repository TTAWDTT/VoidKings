# VoidKings

基于 Cocos2d-x 的类“部落冲突”策略游戏课程项目，核心玩法为基地经营、兵种训练与攻防关卡挑战。主要开发区域为 `Classes/` 与 `proj.win32/`。

## 主要功能
- 基地建设：建筑商店、网格放置、升级与拆除（防御/生产/仓库/陷阱）
- 兵种训练：训练队列与兵种等级管理，战斗前选择已训练兵种
- 战斗系统：进攻/防守两种模式，关卡星级记录与结算面板
- 资源系统：金币/钻石产出与消耗，基地等级影响训练与产速
- 存档系统：6 个存档槽，支持手动删除，切场景自动保存

## 运行与构建

### Windows (Visual Studio)
1. 打开 `proj.win32/VoidKings.sln`
2. 选择 `Debug|Win32` 或 `Release|Win32`
3. 运行（F5）

### CMake (桌面)
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Android (APK)
1. 安装 Android SDK/NDK（确保 `sdkmanager` 可用）
2. 进入 `proj.android` 目录执行：
```bash
gradlew assembleDebug
```
输出 APK：`proj.android/app/build/outputs/apk/debug/app-debug.apk`

## 配置与数据
- 建筑配置：`Resources/res/buildings_config.json`
- 兵种配置：`Resources/res/units_config.json`
- 配置加载：`Classes/Buildings/BuildingManager.*` 与 `Classes/Soldier/UnitManager.*`

## 存档机制
- 基于 Cocos2d-x `local-storage`（SQLite）
- 数据库存放：`FileUtils::getWritablePath() + "voidkings_saves.db"`
- 启动日志会输出实际路径：`[Save] local-storage db: ...`
- 入口：主菜单 Start -> Save Slots（Load/New + Delete）

## 目录结构
```
Classes/        游戏逻辑（场景、实体、系统）
Resources/      资源（纹理、字体、音效、配置）
cocos2d/        引擎源码（勿改）
docs/           项目与参考文档
proj.win32/     VS 工程（Windows 入口）
proj.android/   Android 工程
out/            构建输出（勿改）
```

## 文档索引
`docs/` 目录内常用文档：
- `开发说明.md`：项目结构、模块职责与注意事项
- `操作指南.md`：提交规范与操作习惯

## 版权说明
引擎相关代码遵循其原始开源许可（参见仓库内 `licenses/`）。项目新增源码与逻辑将统一在后续补充的 LICENSE 中声明。
