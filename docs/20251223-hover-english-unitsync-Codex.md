# 悬浮文字英文化与兵种保留逻辑说明

日期: 2025-12-23
作者: Codex

## 目的
- 避免悬浮信息面板中文编码问题
- 兵种在战斗死亡后返回基地仍可保留

## 变更概述
- 悬浮信息面板全部改为英文标签
- 战斗部署不再消耗训练库存，仅作为出战上限

## 涉及文件
- Classes/Scenes/BaseScene.cpp
- Classes/Scenes/BattleScene.cpp
