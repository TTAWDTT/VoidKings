# 训练兵种扩展与战斗同步说明

日期: 2025-12-22
作者: Codex

## 目的
- 训练界面显示更多配置兵种
- 战斗场景部署兵种与训练结果保持一致

## 变更概述
- 训练面板不再过滤资源缺失的兵种，使用占位图保证列表完整
- 待机动画加载改为使用配置中的 anim_idle 名称
- 战斗场景新增参数控制是否使用默认兵种
- 当无训练兵种时，战斗部署区域显示提示并不再注入默认兵种

## 涉及文件
- Classes/UI/TrainPanel.cpp
- Classes/Scenes/BattleScene.h
- Classes/Scenes/BattleScene.cpp
- Classes/Scenes/LevelSelectScene.cpp
