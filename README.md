# VoidKings
一个基于 Cocos2d-x 开发的类“部落冲突”策略游戏项目，为 2025 年下半学期同济大学软件工程（精英班）课程大作业。项目成员：孟圣雨、罗臻、尹佳玮。

## 文档索引
项目 `docs/` 目录现包含以下参考/项目文档：

- `开发说明.md`：项目结构、模块职责与任务分工。
- `操作指南.md`：运行、调试与资源更新指引。
- `CODING_STYLE.md`：继承自 Cocos2d-x 的 C++ 代码规范（来源：Cocos2d-x 官方仓库）。
- `RELEASE_NOTES.md` / `RELEASE_NOTES_CN.md`：引擎官方更新记录（仅作为兼容性参考）。
- `RELEASE_ENGINEERING.md`：引擎发布工程流程参考。
- `文档.md`：课程需求/设计背景的汇总（从根目录同步）。

> 说明：`CODING_STYLE.md` 及引擎相关 *RELEASE* 文档来自 Cocos2d-x v4 开源仓库，仅做学习与规范参照，不代表本项目单独发布或分发引擎。

## 快速开始（Win32）
```powershell
cd D:\Desktop\程序设计范式\部落冲突\VoidKings
mkdir build_win32
cd build_win32
cmake .. -G "Visual Studio 17 2022" -A Win32
cmake --build . --config Debug --target HelloCpp
./bin/HelloCpp/Debug/HelloCpp.exe
```

## 目录结构概要
```
Classes/        游戏逻辑 C++ 源码（场景、实体、系统）
Resources/      资源（纹理、字体、音效、配置）
cocos2d/        引擎源码（通过模板复制）
docs/           项目与参考文档
proj.win32/     VS 原生工程（如需手工打开）
out/            构建输出（按架构/配置划分）
```

## 代码规范简述
遵循 `CODING_STYLE.md`：
- 头文件使用 include guard；
- 类名使用 PascalCase，成员变量使用小写+下划线或尾随下划线；
- 函数短小（≤10 行）可 inline；
- 输入参数在前，输出参数（指针/引用）在后；
- 优先使用智能指针与 RAII 管理生命周期。

## 后续计划（示例）
- 基础战斗单元与建筑系统抽象。
- 地图格子寻路与障碍布局。
- 异步资源加载与简单热更新。
- UI 层：主界面、建造面板、战斗回放。

## License / 版权说明
引擎相关文档与代码遵循其原始开源许可（参见仓库内 `licenses/`）。本项目新增源码与逻辑将统一在后续补充的 LICENSE 中声明（待添加）。

## 贡献指南
1. 新建分支：`feature/<模块>` 或 `fix/<问题>`。
2. 保持单一职责提交，附简洁说明。 
3. 如修改引擎层，需在 PR 描述中标注影响范围与测试场景。

## 问题反馈
欢迎通过 Issues 描述复现步骤、期望行为与日志（如有）。

---
> 若需英文版本或进一步拆分文档，可在 `docs/` 中新增对应文件。
