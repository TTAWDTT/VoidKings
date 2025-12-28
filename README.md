# VoidKings

*Github: https://github.com/TTAWDTT/VoidKings*  
*Release: https://github.com/TTAWDTT/VoidKings/releases/tag/1.0*

基于 Cocos2d-x 3.17 的类《部落冲突》项目，由同济大学国豪书院软件工程精英班的 **罗臻、孟圣雨、尹佳玮** 在 2025/11/30 ~ 2025/12/28 四周内完成。游戏采用 C++ 编写，当前支持 **Win32** 与 **Android** 平台，并提供可直接运行的 Release 包。

## 1. 项目概览
- 主题：基地建设（大兴土木）、资源管理（非法敛财）、兵种训练（大操兵役）、战斗攻防（鬼子进村 / 核打击）。
- 运行方式：从 Release 页面直接下载 `win32.zip` 或 `Android.apk`，即可体验 PC/移动端版本。
- 艺术素材：全部来自开源站点；除引擎外其余逻辑代码均为原创。

### 1.1 核心特性
- **网格化建造系统**：围绕 `GridMap` 实现 72×72 的多层网格，支持拖拽排布、占地检测、拆除、升级与特效反馈。
- **模块化 UI**：`BaseUIPanel`、`BuildShopPanel`、`TrainPanel` 等组件按需挂载，配合统一的 ScienceGothic 字体和按钮交互动画。
- **兵种与战斗**：`UnitManager` 提供训练队列、等级成长；`BattleScene` 支持攻防双方单位 AI、结算、战局回放。
- **六槽存档**：`SaveManager` 利用 `local-storage`(SQLite) 维护 6 个槽位，包含删除、自动保存与同步提示。
- **异步攻防（伪联机）**：可导出基地快照与战斗回放，通过共享目录互换 JSON，完成“玩家 A 设计防守，玩家 B 进攻复盘”的玩法。
- **跨平台构建**：Win32 依赖 VS/CMake，Android 通过 `proj.android/gradlew assembleDebug` 输出 APK/AAB（默认 APK）。

## 2. 技术架构

| 模块 | 关键类 | 说明 |
| --- | --- | --- |
| 核心数据 | `Core`, `SaveManager` | 单例维护资源、等级、关卡星级以及本地存档 |
| 建筑系统 | `BuildingManager`, `BuildingCatalog`, `PlacementManager` | 负责加载 JSON 配置、生成建筑、处理网格占用与建造逻辑 |
| 兵种系统 | `UnitManager`, `Soldier` 子类 | 加载 `units_config.json`，实现训练、等级、AI 属性 |
| 战斗系统 | `BattleScene`, `ReplayManager` | 启动关卡，维护士兵/建筑交互，记录/播放回放 |
| UI 组件 | `BaseUIPanel`, `BuildShopPanel`, `TrainPanel` | 统一按钮手感、字体与布局，负责 HUD、商店、训练等面板 |
| 异步分享 | `BattleShareManager` | 导出/导入 `my_base_snapshot.json`、`last_replay.json` 等文件，封装分享目录 |
| 平台接口 | `proj.win32`, `proj.android` | 平台特性、打包脚本与入口工程 |

### 2.1 设计理念
- **面向对象 + 数据驱动**：配置全部放在 `Resources/res/*.json`，在运行时加载，便于调整数值与热更新。
- **单例模式**：`Core` / `BuildingManager` / `UnitManager` / `SaveManager` / `BattleShareManager` 统一对外提供 `getInstance()`，保持状态一致。
- **工厂模式**：`BuildingManager::createDefenceBuilding()` 等封装具体建筑生成，让调用端专注于布局与逻辑。
- **模块化 UI**：每个面板都是独立 `Layer/Node`，便于在不同场景复用或隐藏。

## 3. 数据驱动与资源

| 数据 | 路径 | 负责类 | 说明 |
| --- | --- | --- | --- |
| 建筑配置 | `Resources/res/buildings_config.json` | `BuildingManager` | 包含类型、尺寸、升级区间、造价等 |
| 兵种配置 | `Resources/res/units_config.json` | `UnitManager` | 载入攻击力、血量、训练时间、特性 |
| 战斗关卡 | `BattleScene` 内部 C++ 脚本 | 计划迁移至 JSON（当前为手写波次） |
| 字体 | `Resources/fonts/ScienceGothic.ttf` | `BaseScene` 等 | 全局 UI 使用统一字体 |
| 音频 | `Resources/music`, `Resources/source` | `AudioManager` | 背景音乐、点击/收集音效 |

## 4. 关键系统

### 4.1 建筑与网格
- `GridMap` 负责世界坐标与网格坐标转换，并提供占用检测。
- `PlacementManager` 结合触摸事件绘制绿/红色可用格提示，调用 `BuildingManager` 生成实例。
- 建筑保存：通过 `BaseScene::savePlacedBuilding()` 将 `BuildingOption + gridX/Y + level` 写入 `BaseSavedBuilding`，供存档、回放与异步快照使用。

### 4.2 兵种与战斗
- `UnitManager::loadConfig("res/units_config.json")` 在 `BaseScene::init()` 中调用，保证训练面板立即可用。
- 训练面板允许批量排队；`onUnitTrainComplete` 在定时器内刷新 HUD 并推送到 `BattleScene`。
- `BattleScene` 支持普通攻关、异步攻打（快照），以及回放模式。`ReplayManager` 提供 `setLastReplay/hasLastReplay/getLastReplay`、`exportReplayTo/importReplayFrom` 等接口。

### 4.3 多存档
- `SaveManager` 初始化 SQLite：`FileUtils::getInstance()->getWritablePath() + "voidkings_saves.db"`（Windows 示例：`C:\Users\<用户名>\AppData\Local\VoidKings\voidkings_saves.db`）。
- 存档槽位：`vk_save_slot_1` ~ `vk_save_slot_6`；每槽存储 `Core` 资源、建筑阵型、兵种训练状态等。
- 主菜单 Start -> Save Slots 打开槽位界面，可创建、读取或删除；切换场景时自动保存。

### 4.4 异步攻防（伪联机）
- UI：基地界面的 `Async Ops` 按钮拉起 `setupAsyncPanel()` 创建的遮罩层（`Layout` + `LayerColor` 卡片），所有文案、按钮均为英文显示并使用 ScienceGothic。
- Share 目录：`FileUtils::getWritablePath() + "share/"`（PC 例：`C:\Users\<用户名>\AppData\Local\VoidKings\share\`；Android：`Android/data/com.yourcompany.voidkings/files/share/`）。
- 文件规范：
  - `my_base_snapshot.json`：导出本地基地；
  - `target_base_snapshot.json`：对方发来的基地；
  - `last_replay.json`：最新战斗回放；
  - `target_replay.json`：外部导入回放。
- 操作流程：
  1. 在异步面板中点击 **EXPORT MY BASE**，获得 `my_base_snapshot.json`，发送给其他玩家；
  2. 对方将文件重命名为 `target_base_snapshot.json` 后放入 share 目录，你在面板选择 **LOAD TARGET & ATTACK**，系统通过 `BattleScene::createSnapshotScene()` 载入并直接进入战斗；
  3. 战斗后用 **EXPORT LAST REPLAY** 导出 `last_replay.json` 供对方复盘，对方放为 `target_replay.json` 后按 **PLAY IMPORTED REPLAY** 播放；
  4. 面板底部的状态条会实时提示导入/导出结果，并在 4 秒后淡出。
- 技术细节参见 `docs/async-share-guide.md`。

### 4.5 UI/音频统一
- 所有 UI Label/Button 均调用 `createBaseLabel()` 或指定 `fonts/ScienceGothic.ttf`，保证全局视觉统一。
- 按钮默认启用 `PressedAction + ZoomScale`，并使用纯色 Layer 作为背景，减轻纹理依赖。
- `AudioManager` 负责按钮音、收集音、背景音乐，集成 Win32/Android 平台的混音逻辑。

## 5. 构建与运行

### 5.1 Windows (Visual Studio)
1. 打开 `proj.win32/VoidKings.sln`
2. 选择 `Debug|Win32` 或 `Release|Win32`
3. 直接 F5 运行

### 5.2 CMake (桌面通用)
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 5.3 Android
1. 安装 Android SDK/NDK，并设置 `ANDROID_SDK_ROOT`（或放在默认位置 `C:\Android\Sdk` 等）
2. 进入 `proj.android`
3. 执行：
   ```powershell
   .\gradlew assembleDebug
   ```
4. 产物：`proj.android/app/build/outputs/apk/debug/VoidKings-Android.apk`（若需要 AAB，可改执行 `bundleDebug`）

### 5.4 运行数据路径
- 配置/资源：`Resources/`
- Share 目录：`<WritablePath>/share/`
- 存档数据库：`<WritablePath>/voidkings_saves.db`
- 日志：在 VS Output 或 Android Logcat 中搜索 `[BaseScene] / [Save] / [Share]`

## 6. 项目目录结构
```
VoidKing-Fake
├─ build/            # CMake 构建输出
├─ Classes/
│  ├─ Buildings/     # 建筑实体与管理
│  ├─ Bullet/
│  ├─ Core/
│  ├─ Map/
│  ├─ Replay/
│  ├─ Save/
│  ├─ Scenes/
│  │   └─ components/
│  ├─ Soldier/
│  ├─ UI/
│  └─ Utils/
├─ cocos2d/          # 引擎源码
├─ docs/             # 设计/操作/变更文档
├─ proj.android/     # Android 工程（Gradle）
├─ proj.win32/       # Windows VS 工程
├─ Resources/        # 纹理、音频、配置
├─ out/              # 运行期资源拷贝
├─ CMakeLists.txt
└─ README.md
```

## 7. 未来规划 / 改进方向
1. **关卡 JSON 化**：把 `BattleScene` 中硬编码的波次迁移到 `Resources/res/levels/*.json`，便于策划与热更新。
2. **异步攻防云同步**：在 `BattleShareManager` 基础上增加网络上传/下载，将 JSON 推送到服务器，实现真正的“异步 PvP”。
3. **战斗数值回放增强**：围绕 `ReplayManager` 增加事件标签、关键帧跳转，支持分享链接。
4. **UI 主题切换**：在保持 ScienceGothic 主字体的同时，支持夜间/亮色两套主题提升可读性。
5. **安卓体验优化**：加入触摸引导、震动反馈，并探索 AAB 分发以及 64-bit ABI。

---