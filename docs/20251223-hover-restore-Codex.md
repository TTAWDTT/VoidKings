# 悬浮信息可视化与基地状态保持修正

日期: 2025-12-23
作者: Codex

## 目的
- 提升建筑悬浮信息面板可读性
- 解决从 Attack 界面返回后基地建筑丢失的问题

## 变更概述
- 悬浮信息面板放大，增加边框与阴影，提升文字/背景对比度
- 悬浮信息面板设置最小尺寸，避免内容过小
- 基地场景新增已建造建筑的内存态保存与恢复

## 涉及文件
- Classes/Scenes/BaseScene.h
- Classes/Scenes/BaseScene.cpp
- Classes/Scenes/BattleScene.cpp

## 备注
- 基地状态为内存级缓存（不落盘），仅用于场景切换时保持
