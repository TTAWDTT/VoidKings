# MainMenuScene 场景开发工作总结

## 📋 文档信息
- **创建日期**: 2025年
- **项目**: VoidKings
- **版本**: v1.0.0

---

## 🔍 问题诊断与解答

### 问题 1: 是否已将 MainMenu 设置为启动场景？

**✅ 是**

在 `AppDelegate.cpp` 中已正确配置：
```cpp
#include "Scenes/MainMenuScene.h"
// ...
auto scene = MainMenuScene::createScene();
director->runWithScene(scene);
```

### 问题 2: MainMenu 是否能正常运行？

**⚠️ 修复后可正常运行**

原存在的问题：
1. `MainMenuScene.cpp` 未被添加到 CMake 构建系统
2. 缺少 `CREATE_FUNC(MainMenuScene)` 宏导致 `create()` 方法未定义
3. 缺少 `onReturn` 方法声明（但实现已存在）
4. cpp 文件头部包含了错误的 CMake 格式注释（以 `#` 开头）

### 问题 3: 报错原因分析

**🔴 LNK2019 链接错误**

```
error LNK2019: 无法解析的外部符号 
"public: static class cocos2d::Scene * __cdecl MainMenuScene::createScene(void)"
```

**根本原因**: `CMakeLists.txt` 中的 `GAME_SOURCE` 列表缺少 `MainMenuScene.cpp`，导致该文件未被编译，链接时找不到符号定义。

### 问题 4: MainMenu 与 HelloWorld 是否冲突？

**✅ 不冲突**

- `HelloWorldScene.h` 和 `HelloWorldScene.cpp` 的内容已被完全注释
- 两个场景类是独立的，不会产生任何符号冲突
- CMakeLists 中仍保留 HelloWorld 文件引用是安全的（注释后的空文件不影响编译）

---

## 🛠️ 已执行的修复

### 修复 1: CMakeLists.txt

在源文件列表中添加 MainMenuScene：

```cmake
# 修改前
list(APPEND GAME_SOURCE
     Classes/AppDelegate.cpp
     Classes/HelloWorldScene.cpp
     )
list(APPEND GAME_HEADER
     Classes/AppDelegate.h
     Classes/HelloWorldScene.h
     )

# 修改后
list(APPEND GAME_SOURCE
     Classes/AppDelegate.cpp
     Classes/HelloWorldScene.cpp
     Classes/Scenes/MainMenuScene.cpp
     )
list(APPEND GAME_HEADER
     Classes/AppDelegate.h
     Classes/HelloWorldScene.h
     Classes/Scenes/MainMenuScene.h
     )
```

### 修复 2: MainMenuScene.h

添加 `CREATE_FUNC` 宏和 `onReturn` 方法声明：

```cpp
class MainMenuScene : public Scene
{
public:
    static Scene* createScene();
    virtual bool init() override;
    
    // 实现 create() 方法
    CREATE_FUNC(MainMenuScene);
    
    void onStart(Ref* sender);
    void onSettings(Ref* sender);
    void onRule(Ref* sender);
    void onReturn(Ref* sender);  // 新增声明
    void onExit(Ref* sender);
    // ...
};
```

### 修复 3: MainMenuScene.cpp

移除了文件头部错误的 CMake 格式注释（以 `#` 开头的注释在 C++ 文件中是无效的）。

---

## 📁 当前文件结构

```
Classes/
├── AppDelegate.cpp          # 应用入口，配置启动场景
├── AppDelegate.h
├── HelloWorldScene.cpp      # 已注释（保留文件）
├── HelloWorldScene.h        # 已注释（保留文件）
├── core/
│   ├── GameDefines.h
│   ├── ResourceManager.cpp
│   └── ResourceManager.h
└── Scenes/
    ├── MainMenuScene.cpp    # 主菜单场景实现
    ├── MainMenuScene.h      # 主菜单场景声明
    └── SceneWorkSummary.md  # 本文档
```

---

## ✅ 重新构建步骤

### 方法 1: Visual Studio (推荐)

1. **删除 CMake 缓存**
   - 右键点击 `CMakeLists.txt` → **Delete Cache and Reconfigure**
   - 或手动删除 `out/build/` 目录

2. **重新配置项目**
   - **Project** → **Configure Cache** 或等待 VS 自动重新配置

3. **重新编译**
   - **Build** → **Rebuild All** 或按 `Ctrl+Shift+B`

4. **运行程序**
   - 按 `F5` 启动调试

### 方法 2: 命令行

```powershell
# 进入项目根目录
cd D:\VoidKings

# 删除旧构建目录
Remove-Item -Recurse -Force out/build

# 重新配置 CMake
cmake -S . -B out/build -G "Ninja"

# 编译
cmake --build out/build --config Debug

# 运行
./out/build/bin/VoidKings/VoidKings.exe
```

---

## 🎯 MainMenuScene 功能概述

| 功能 | 描述 | 状态 |
|------|------|------|
| 背景显示 | 加载并显示 `background.png` | ✅ 完成 |
| 标题 Logo | 显示 "Void Kings" 标题 | ✅ 完成 |
| Start Game | 切换到游戏主场景 | ⏳ 待 BaseScene 实现 |
| Settings | 切换到设置层 | ✅ 完成 |
| Rule | 切换到规则层 | ✅ 完成 |
| Exit | 退出游戏 | ✅ 完成 |
| Return | 从子层返回主菜单 | ✅ 完成 |
| 版本信息 | 显示 "Version 1.0.0" | ✅ 完成 |

---

## 📌 后续待办事项

- [ ] 实现 `BaseScene` 游戏主场景
- [ ] 完善 Settings 设置功能（音量、分辨率等）
- [ ] 完善 Rule 规则说明内容
- [ ] 添加 `background.png` 资源文件到 Resources 目录（如尚未添加）
- [ ] 添加按钮悬停/点击音效
- [ ] 优化按钮创建代码（提取辅助方法减少重复）

---

## ⚠️ 重要提醒

> **每次添加新的 `.cpp` 文件时，务必同步更新 `CMakeLists.txt` 中的源文件列表！**
> 
> 这是 Cocos2d-x 项目中最常见的链接错误原因。

### 添加新场景的标准步骤

1. 在 `Classes/Scenes/` 目录创建 `.h` 和 `.cpp` 文件
2. 在头文件中添加 `CREATE_FUNC(YourScene)` 宏
3. 在 `CMakeLists.txt` 的 `GAME_SOURCE` 和 `GAME_HEADER` 中添加文件路径
4. 重新配置 CMake 缓存
5. 重新编译

---

## 📚 参考资料

- [Cocos2d-x Scene 文档](https://docs.cocos2d-x.org/cocos2d-x/v4/en/scenes/)
- [CREATE_FUNC 宏说明](https://docs.cocos2d-x.org/cocos2d-x/v4/en/basic_concepts/)
- [CMake 基础教程](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)

---

*文档生成时间: 自动生成*
