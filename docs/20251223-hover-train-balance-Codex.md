# 悬浮面板与训练界面扩展说明

日期: 2025-12-23
作者: Codex

## 目的
- 修复悬浮信息文字不可见的问题
- 训练界面一次展示更多兵种
- 调整数值以避免远程过强

## 变更概述
- 悬浮信息面板改用系统字体，提升对比度与可读性
- 训练面板改为 4 列布局，面板尺寸增大，可覆盖 12 个兵种
- 进一步下调远程单位攻击范围（弩手/法师/大法师）

## 涉及文件
- Classes/Scenes/BaseScene.cpp
- Classes/Scenes/BattleScene.cpp
- Classes/UI/TrainPanel.h
- Resources/res/units_config.json
- Resources/res/units.json
