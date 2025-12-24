# 20251224 UI与效果优化 - Codex

## 改动概览
- 主菜单/训练/建筑商店等界面加入过渡动画与悬停反馈
- 基地/战斗场景增强底纹与放置/命中/部署反馈
- 修复资源路径与兵种待机动画配置

## 主要修改
1. UI交互与动效
- 主菜单：按钮入场动画、弹窗遮罩与缩放反馈
- 基地UI：按钮悬停高亮，资源图标动画
- 建筑商店：展示/隐藏动画，费用展示增加金币图标
- 训练面板：展示/隐藏动画，招募/升级按钮随资源高亮

2. 画面与效果
- 基地/战斗网格：草地纹理铺底 + 细网格线
- 放置系统：预览信息浮窗与轻微呼吸缩放
- 建筑放置：落位缩放反馈
- 战斗部署：落点环形提示
- 子弹与受击：命中冲击环 + 受击闪烁

3. 配置与调试
- Core 资源动画改为直接读取纹理路径
- units_config.json 修复 HalberdMan idle 动画关键字

## 影响范围
- Classes/Scenes/MainMenuScene.cpp
- Classes/Scenes/BaseScene.cpp
- Classes/Scenes/LevelSelectScene.cpp
- Classes/Scenes/BattleScene.cpp
- Classes/Scenes/components/BaseUIPanel.cpp
- Classes/Scenes/components/BuildShopPanel.cpp
- Classes/Scenes/components/PlacementManager.cpp
- Classes/Scenes/components/GridBackground.cpp
- Classes/UI/TrainPanel.cpp
- Classes/UI/IDCardPanel.cpp
- Classes/Bullet/Bullet.cpp
- Classes/Buildings/DefenceBuilding.cpp
- Classes/Buildings/ProductionBuilding.cpp
- Classes/Buildings/StorageBuilding.cpp
- Classes/Soldier/Soldier.cpp
- Classes/Core/Core.cpp
- Resources/res/units_config.json

## 测试建议
- 主菜单切换、基地建造、训练面板与建筑商店交互
- 战斗部署与防御塔攻击效果
- 资源数值更新与动画显示
