# UI 交互体验优化说明

日期: 2025-12-22
作者: Codex

## 目的
- 提升按钮点击反馈一致性
- 避免弹窗/建造放置过程中误触底层按钮
- 清理无用变量与重复回调

## 变更概述
- 为主菜单、基地 UI、建造商店、训练面板、关卡选择、战斗部署按钮增加按压缩放反馈与触摸吞噬
- 新增 BaseUIPanel::setButtonsEnabled，弹窗/建造放置时禁用主按钮
- 工具提示在按钮禁用时不显示
- 清理 MainMenuScene 未使用变量

## 涉及文件
- Classes/Scenes/MainMenuScene.cpp
- Classes/Scenes/components/BaseUIPanel.h
- Classes/Scenes/components/BaseUIPanel.cpp
- Classes/Scenes/components/BuildShopPanel.cpp
- Classes/UI/TrainPanel.cpp
- Classes/Scenes/LevelSelectScene.cpp
- Classes/Scenes/BattleScene.cpp
- Classes/Scenes/BaseScene.cpp
