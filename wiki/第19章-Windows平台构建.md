# 第19章 Windows 平台构建

> **本章目标**：掌握 VoidKings 的 Windows 构建方式，包括 Visual Studio 工程和 CMake 构建。

---

## 19.1 Visual Studio 工程

### 19.1.1 proj.win32/VoidKings.sln

工程结构：

```
proj.win32/
├── VoidKings.sln              # 解决方案文件
├── VoidKings.vcxproj          # 项目文件
├── VoidKings.vcxproj.filters  # 文件过滤器
├── main.cpp                   # Windows 入口
└── resource.h                 # 资源头文件
```

**main.cpp**：

```cpp
// proj.win32/main.cpp
#include "main.h"
#include "AppDelegate.h"
#include "cocos2d.h"

USING_NS_CC;

int WINAPI _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 创建应用实例
    AppDelegate app;
    
    // 运行
    return Application::getInstance()->run();
}
```

### 19.1.2 项目依赖配置

在 Visual Studio 中配置：

**包含目录**：
```
$(SolutionDir)..\cocos2d\cocos
$(SolutionDir)..\cocos2d\external
$(SolutionDir)..\Classes
```

**库目录**：
```
$(SolutionDir)..\cocos2d\prebuilt\win32\$(Configuration)
```

**附加依赖项**：
```
libcocos2d.lib
libcurl_imp.lib
glew32.lib
opengl32.lib
libbox2d.lib
```

### 19.1.3 资源文件复制

**生成后事件**：

```batch
xcopy /Y /E "$(SolutionDir)..\Resources\*" "$(OutDir)"
```

或使用自定义生成步骤：

```xml
<!-- VoidKings.vcxproj -->
<PostBuildEvent>
  <Command>
    xcopy /Y /E /I "$(ProjectDir)..\Resources\*" "$(OutDir)"
  </Command>
</PostBuildEvent>
```

---

## 19.2 CMake 构建

### 19.2.1 CMakeLists.txt 解析

```cmake
cmake_minimum_required(VERSION 3.10)

project(VoidKings)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cocos2d-x 根目录
set(COCOS2DX_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/cocos2d)

# 包含 Cocos2d-x 构建脚本
include(${COCOS2DX_ROOT}/cmake/Cocos2dxBuildHelpers.cmake)

# 收集源文件
file(GLOB_RECURSE GAME_SRC
    Classes/*.cpp
    Classes/*.h
)

# Windows 入口
if(WIN32)
    list(APPEND GAME_SRC proj.win32/main.cpp)
endif()

# 创建可执行文件
add_executable(${PROJECT_NAME} ${GAME_SRC})

# 链接 Cocos2d-x
target_link_libraries(${PROJECT_NAME} cocos2d)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Classes
)

# 复制资源
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
```

### 19.2.2 Debug/Release 配置

```powershell
# 创建构建目录
mkdir build
cd build

# Debug 配置
cmake .. -G "Visual Studio 17 2022" -A Win32
cmake --build . --config Debug

# Release 配置
cmake --build . --config Release
```

**配置区别**：

| 配置 | 优化 | 调试信息 | 断言 |
|------|------|----------|------|
| Debug | 无 (-Od) | 完整 | 启用 |
| Release | 最高 (-O2) | 无 | 禁用 |

### 19.2.3 输出目录设置

```cmake
# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)
```

---

## 19.3 调试技巧

### 19.3.1 断点调试

在 Visual Studio 中：

1. **设置断点**：点击代码行号左侧
2. **启动调试**：F5 或 调试 → 开始调试
3. **单步执行**：
   - F10：逐过程（不进入函数）
   - F11：逐语句（进入函数）
   - Shift+F11：跳出函数
4. **查看变量**：
   - 监视窗口
   - 局部变量窗口
   - 数据提示（鼠标悬停）

**条件断点**：
1. 右键断点 → 条件
2. 输入条件表达式，如 `i == 100`

### 19.3.2 CCLOG 输出查看

```cpp
// 使用 CCLOG 输出调试信息
CCLOG("[BattleScene] 士兵数量: %zu", _soldiers.size());
CCLOG("[Core] 金币: %d, 钻石: %d", 
      getResource(ResourceType::COIN),
      getResource(ResourceType::DIAMOND));
```

查看输出：
- **Visual Studio**：调试 → 窗口 → 输出
- **控制台**：如果启用了控制台窗口

**启用控制台**：

```cpp
// main.cpp
#if defined(_DEBUG)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif
```

### 19.3.3 内存泄漏检测

**使用 Visual Studio 内置检测**：

```cpp
// main.cpp
#if defined(_DEBUG)
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif

int WINAPI _tWinMain(...) {
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    
    AppDelegate app;
    return Application::getInstance()->run();
}
```

程序退出时，内存泄漏会在输出窗口显示：

```
Detected memory leaks!
Dumping objects ->
{1234} normal block at 0x00ABCDEF, 64 bytes long.
```

**定位泄漏**：

```cpp
// 设置断点在第 1234 次分配
_CrtSetBreakAlloc(1234);
```

---

## 常见问题

### 问题1：找不到 DLL

```
The program can't start because xxx.dll is missing
```

**解决**：
1. 确保 Cocos2d-x 的 DLL 已复制到输出目录
2. 在项目属性中设置工作目录为输出目录

### 问题2：资源加载失败

```
[FileUtils] File not found: xxx.png
```

**解决**：
1. 检查生成后事件是否执行
2. 确认资源路径正确
3. 检查工作目录设置

### 问题3：链接错误

```
LNK2019: unresolved external symbol
```

**解决**：
1. 检查库文件路径
2. 确保链接了所有必要的库
3. 检查平台配置（x86 vs x64）

---

## 本章小结

1. **Visual Studio 工程** 是最常用的 Windows 开发方式
2. **CMake** 提供跨平台的构建配置
3. **资源复制** 需要在构建后执行
4. **断点调试** 是主要的问题排查手段
5. **内存泄漏检测** 帮助发现资源管理问题

---

## 练习题

1. 尝试使用 CMake 构建项目
2. 添加自定义的日志输出宏
3. 使用性能分析器检测瓶颈

---

**下一章**：[第20章 代码阅读与调试技巧](第20章-代码阅读与调试技巧.md)
