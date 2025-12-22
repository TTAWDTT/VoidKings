# 待机动画与弓箭手数值调整说明

日期: 2025-12-23  
作者: Codex

## 目的
- 部署栏选中兵种时展示待机动画
- 弓箭手数值更合理，降低过强表现

## 变更概述
- 部署栏按钮持续显示待机动画图标
- 训练面板移除顶部预览框，卡片内持续播放待机动画
- 弓箭手的生命、防御、攻击、射程与速度下调

## 涉及文件
- Classes/Scenes/BattleScene.cpp
- Classes/UI/TrainPanel.cpp
- Classes/UI/TrainPanel.h
- Resources/res/units_config.json
- Resources/res/units.json
