# 第18章 Android 平台适配

> **本章目标**：理解 VoidKings 的 Android 构建流程，掌握 NDK/Gradle 配置、资源打包、调试技巧。

---

## 18.1 工程结构

### 18.1.1 Gradle 工程解析

```
proj.android/
├── app/
│   ├── build.gradle           # 应用模块构建配置
│   ├── src/
│   │   └── main/
│   │       ├── java/          # Java 代码
│   │       │   └── org/cocos2dx/cpp/
│   │       │       └── AppActivity.java
│   │       ├── jni/           # NDK 配置
│   │       │   └── Android.mk
│   │       ├── res/           # Android 资源
│   │       └── AndroidManifest.xml
│   └── assets/                # 游戏资源（构建时复制）
├── build.gradle               # 项目级构建配置
├── settings.gradle            # 项目设置
├── gradle.properties          # Gradle 属性
├── local.properties           # 本地 SDK/NDK 路径
└── gradlew(.bat)              # Gradle 包装器
```

### 18.1.2 app/build.gradle 配置

```groovy
// proj.android/app/build.gradle
apply plugin: 'com.android.application'

android {
    compileSdkVersion 33
    
    defaultConfig {
        applicationId "com.yourcompany.voidkings"
        minSdkVersion 21
        targetSdkVersion 33
        versionCode 1
        versionName "1.0"
        
        // NDK 配置
        externalNativeBuild {
            cmake {
                arguments "-DCMAKE_TOOLCHAIN_FILE=${android.ndkDirectory}/build/cmake/android.toolchain.cmake"
                cppFlags "-std=c++14 -frtti -fexceptions"
            }
        }
        
        ndk {
            abiFilters "armeabi-v7a", "arm64-v8a"
        }
    }
    
    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android.txt')
        }
    }
    
    externalNativeBuild {
        cmake {
            path "../../CMakeLists.txt"
        }
    }
    
    sourceSets {
        main {
            assets.srcDirs = ['assets']
            jniLibs.srcDirs = ['libs']
        }
    }
}

dependencies {
    implementation fileTree(dir: 'libs', include: ['*.jar'])
}
```

### 18.1.3 NDK/CMake 集成

CMakeLists.txt 中的 Android 特定配置：

```cmake
# CMakeLists.txt
if(ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -frtti -fexceptions")
    
    # 添加 Cocos2d-x 库
    add_subdirectory(${COCOS2DX_ROOT}/cocos ${ENGINE_BINARY_PATH}/cocos/core)
    
    # 创建共享库
    add_library(${APP_NAME} SHARED
        ${GAME_SRC}
    )
    
    target_link_libraries(${APP_NAME}
        cocos2d
        android
        log
        GLESv2
        EGL
    )
endif()
```

---

## 18.2 原生层桥接

### 18.2.1 AppActivity 入口

```java
// proj.android/app/src/main/java/org/cocos2dx/cpp/AppActivity.java
package org.cocos2dx.cpp;

import android.os.Bundle;
import org.cocos2dx.lib.Cocos2dxActivity;

public class AppActivity extends Cocos2dxActivity {
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.setEnableVirtualButton(false);
        super.onCreate(savedInstanceState);
    }
    
    // 加载原生库
    static {
        System.loadLibrary("cocos2dcpp");
    }
}
```

### 18.2.2 JNI 调用基础

从 C++ 调用 Java：

```cpp
// 示例：调用 Android 震动功能
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <jni.h>
#include "platform/android/jni/JniHelper.h"

void AndroidUtils::vibrate(int milliseconds) {
    JniMethodInfo methodInfo;
    
    if (JniHelper::getStaticMethodInfo(
            methodInfo,
            "org/cocos2dx/cpp/AppActivity",
            "vibrate",
            "(I)V")) {
        
        methodInfo.env->CallStaticVoidMethod(
            methodInfo.classID,
            methodInfo.methodID,
            milliseconds);
        
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
}
#endif
```

对应的 Java 代码：

```java
// AppActivity.java
public static void vibrate(int milliseconds) {
    Activity activity = (Activity) Cocos2dxHelper.getActivity();
    Vibrator vibrator = (Vibrator) activity.getSystemService(Context.VIBRATOR_SERVICE);
    if (vibrator != null) {
        vibrator.vibrate(milliseconds);
    }
}
```

### 18.2.3 libcocos2dcpp.so 加载

```java
static {
    // 加载 Cocos2d-x 引擎和游戏代码
    System.loadLibrary("cocos2dcpp");
}
```

这会加载：
- `libcocos2dcpp.so`：包含引擎和游戏代码的共享库

---

## 18.3 资源打包

### 18.3.1 assets 目录复制

Gradle 构建时自动复制资源：

```groovy
// app/build.gradle
android {
    sourceSets {
        main {
            assets.srcDirs = ['assets']
        }
    }
}

// 构建前复制资源
task copyAssets(type: Copy) {
    from '../../Resources'
    into 'assets'
}

preBuild.dependsOn copyAssets
```

也可以手动复制：

```powershell
# Windows PowerShell
Copy-Item -Recurse -Force "..\..\Resources\*" "app\assets\"
```

### 18.3.2 资源路径处理

Android 上的资源路径自动映射：

```cpp
// 代码中的相对路径
auto sprite = Sprite::create("res/player.png");

// 实际对应 APK 中的
// assets/res/player.png

// FileUtils 自动处理
auto fileUtils = FileUtils::getInstance();
std::string fullPath = fileUtils->fullPathForFilename("res/player.png");
// 返回: assets/res/player.png
```

### 18.3.3 APK/AAB 输出

```powershell
# 构建 APK
cd proj.android
.\gradlew assembleDebug
# 输出: app/build/outputs/apk/debug/app-debug.apk

.\gradlew assembleRelease
# 输出: app/build/outputs/apk/release/app-release.apk

# 构建 AAB (Android App Bundle)
.\gradlew bundleRelease
# 输出: app/build/outputs/bundle/release/app-release.aab
```

---

## 18.4 构建与调试

### 18.4.1 gradlew assembleDebug 流程

```
┌────────────────────────────────────────────────┐
│           Gradle 构建流程                       │
├────────────────────────────────────────────────┤
│                                                │
│  1. 检查依赖                                    │
│       ↓                                        │
│  2. 复制资源到 assets/                          │
│       ↓                                        │
│  3. 编译 C++ 代码 (CMake + NDK)                │
│       ↓                                        │
│  4. 生成 libcocos2dcpp.so                      │
│       ↓                                        │
│  5. 编译 Java 代码                             │
│       ↓                                        │
│  6. 打包资源和代码                             │
│       ↓                                        │
│  7. 签名 APK                                   │
│       ↓                                        │
│  8. 输出 app-debug.apk                         │
│                                                │
└────────────────────────────────────────────────┘
```

### 18.4.2 Logcat 日志查看

```powershell
# 查看所有日志
adb logcat

# 过滤 Cocos2d-x 日志
adb logcat | findstr "cocos2d"

# 过滤游戏日志
adb logcat | findstr "VoidKings"

# 过滤崩溃信息
adb logcat | findstr "FATAL\|Exception\|Error"

# 清除日志
adb logcat -c
```

在代码中输出日志：

```cpp
CCLOG("[BattleScene] 战斗开始: levelId=%d", levelId);
// 输出到 Logcat，可用 "cocos2d-x debug" 标签过滤
```

### 18.4.3 常见构建问题排查

**问题1：NDK 未找到**
```
NDK not found at xxxxxx
```
解决：在 `local.properties` 中指定 NDK 路径：
```properties
ndk.dir=C\:\\Android\\ndk\\25.1.8937393
sdk.dir=C\:\\Android\\Sdk
```

**问题2：ABI 不兼容**
```
java.lang.UnsatisfiedLinkError: dlopen failed
```
解决：检查 `build.gradle` 中的 `abiFilters`：
```groovy
ndk {
    abiFilters "armeabi-v7a", "arm64-v8a"
}
```

**问题3：资源加载失败**
```
[FileUtils] File not found: xxx.png
```
解决：确保资源已复制到 `app/assets/`：
```powershell
.\gradlew clean
.\gradlew assembleDebug
```

**问题4：内存不足**
```
java.lang.OutOfMemoryError
```
解决：增加 Gradle 内存：
```properties
# gradle.properties
org.gradle.jvmargs=-Xmx4096m
```

---

## 18.5 沙盒路径

### 18.5.1 getWritablePath() 实现

```cpp
// Android 上的可写路径
std::string path = FileUtils::getInstance()->getWritablePath();
// 返回类似: /data/data/com.yourcompany.voidkings/files/

// 存档文件路径
std::string savePath = path + "voidkings_saves.db";
// /data/data/com.yourcompany.voidkings/files/voidkings_saves.db

// 分享目录路径
std::string sharePath = path + "share/";
// /data/data/com.yourcompany.voidkings/files/share/
```

### 18.5.2 存档与分享目录

```cpp
// 确保目录存在
void ensureShareDirExists() {
    std::string shareDir = FileUtils::getInstance()->getWritablePath() + "share/";
    
    if (!FileUtils::getInstance()->isDirectoryExist(shareDir)) {
        FileUtils::getInstance()->createDirectory(shareDir);
    }
}
```

**访问外部存储（需要权限）**：

```xml
<!-- AndroidManifest.xml -->
<uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE"/>
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"/>
```

```java
// 请求权限
if (ContextCompat.checkSelfPermission(this, 
        Manifest.permission.WRITE_EXTERNAL_STORAGE) 
        != PackageManager.PERMISSION_GRANTED) {
    ActivityCompat.requestPermissions(this,
        new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
}
```

---

## 本章小结

1. **Gradle 工程** 使用标准 Android 构建系统
2. **NDK/CMake** 编译 C++ 代码为 .so 文件
3. **AppActivity** 是 Android 入口，加载原生库
4. **资源** 打包到 APK 的 assets/ 目录
5. **Logcat** 是主要的调试工具

---

## 练习题

1. 添加 JNI 接口实现设备震动功能
2. 实现 Android 通知功能
3. 添加广告 SDK 集成

---

**下一章**：[第19章 Windows 平台构建](第19章-Windows平台构建.md)
