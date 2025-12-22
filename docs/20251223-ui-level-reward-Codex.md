# 关卡扩展与UI/战斗奖励调整说明

日期: 2025-12-23
作者: Codex

## 目的
- 扩展关卡数量并优化选关排版
- 提升战斗难度并加入胜利奖励
- 建筑悬浮信息可视化，辅助策略决策
- 资源显示与实际资源保持同步

## 变更概述
- 关卡数量扩展至 12 关，选关界面改为 4x3 网格排布
- 新增 6-12 关战斗布局，提升防御塔数量与等级，战斗时限缩短
- 战斗胜利后根据关卡与摧毁率发放金币/钻石奖励
- 基地与战斗场景支持建筑悬浮信息展示（名称/等级/HP/攻击/范围/产量/占地）
- 弓箭手攻击范围进一步下调，避免远程过强
- 资源 UI 改为读取 Core 实际数值，建造扣费与训练消耗保持一致
- 移除 IDCardPanel 中的调试输出与冗余资源变量

## 涉及文件
- Classes/Scenes/LevelSelectScene.h
- Classes/Scenes/LevelSelectScene.cpp
- Classes/Scenes/BattleScene.h
- Classes/Scenes/BattleScene.cpp
- Classes/Scenes/BaseScene.h
- Classes/Scenes/BaseScene.cpp
- Classes/Scenes/components/BaseUIPanel.cpp
- Classes/UI/IDCardPanel.h
- Classes/UI/IDCardPanel.cpp
- Classes/Buildings/DefenceBuilding.h
- Classes/Buildings/ProductionBuilding.h
- Classes/Buildings/StorageBuilding.h
- Resources/res/units_config.json
- Resources/res/units.json

## 备注
- 建筑悬浮范围以虚线圈示意攻击范围，占地显示实线框
- 若后续需要关卡掉落/结算界面可独立扩展面板
