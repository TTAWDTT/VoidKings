# 附录E：常见问题解答 (FAQ)

> **本附录**：收集 VoidKings 开发中常见的问题和解决方案。

---

## 构建问题

### Q1: Visual Studio 编译失败：找不到 cocos2d.h

**原因**：包含目录未正确配置

**解决**：
1. 右键项目 → 属性 → C/C++ → 附加包含目录
2. 添加：`$(SolutionDir)..\cocos2d\cocos`

---

### Q2: 链接错误：unresolved external symbol

**原因**：库文件未链接

**解决**：
1. 右键项目 → 属性 → 链接器 → 输入 → 附加依赖项
2. 添加：`libcocos2d.lib`

---

### Q3: Android 构建失败：NDK not found

**原因**：`local.properties` 中未配置 NDK 路径

**解决**：
在 `proj.android/local.properties` 中添加：
```properties
ndk.dir=C\:\\Android\\ndk\\25.1.8937393
sdk.dir=C\:\\Android\\Sdk
```

---

### Q4: Gradle 构建内存不足

**原因**：默认内存配置过低

**解决**：
在 `proj.android/gradle.properties` 中添加：
```properties
org.gradle.jvmargs=-Xmx4096m
```

---

## 运行时问题

### Q5: 游戏启动后白屏/黑屏

**可能原因**：
1. 资源文件未复制
2. 首个场景创建失败

**解决**：
1. 检查 Resources 是否复制到输出目录
2. 查看日志输出，定位错误

---

### Q6: 资源加载失败：File not found

**可能原因**：
1. 路径错误
2. 大小写不匹配（Android）
3. 搜索路径未配置

**解决**：
```cpp
// 检查路径
CCLOG("完整路径: %s", 
    FileUtils::getInstance()->fullPathForFilename("player.png").c_str());

// 添加搜索路径
FileUtils::getInstance()->addSearchPath("Resources");
```

---

### Q7: 触摸无响应

**可能原因**：
1. 监听器未注册
2. 被其他节点吞噬
3. 节点不可见或超出屏幕

**解决**：
```cpp
// 检查节点状态
CCLOG("visible: %d, opacity: %d, pos: (%.0f, %.0f)",
    node->isVisible(), node->getOpacity(),
    node->getPositionX(), node->getPositionY());

// 确保不被吞噬
listener->setSwallowTouches(false);
```

---

### Q8: 动画不播放

**可能原因**：
1. 动画帧文件不存在
2. 帧数配置错误
3. Sprite 被其他动作覆盖

**解决**：
```cpp
// 检查帧是否存在
for (int i = 1; i <= frameCount; ++i) {
    std::string path = StringUtils::format("%s_%s_%d.png", 
        baseName.c_str(), animKey.c_str(), i);
    bool exists = FileUtils::getInstance()->isFileExist(path);
    CCLOG("帧 %d: %s - %s", i, path.c_str(), exists ? "存在" : "不存在");
}
```

---

### Q9: 音效无声

**可能原因**：
1. 音频文件格式不支持
2. 静音状态
3. 音量为 0

**解决**：
```cpp
// 检查静音状态
CCLOG("BGM muted: %d, SFX muted: %d", 
    AudioManager::isBgmMuted(), AudioManager::isSfxMuted());

// 检查音量
CCLOG("BGM volume: %.2f, SFX volume: %.2f",
    SimpleAudioEngine::getInstance()->getBackgroundMusicVolume(),
    SimpleAudioEngine::getInstance()->getEffectsVolume());
```

---

## 逻辑问题

### Q10: 士兵不攻击/不移动

**可能原因**：
1. 目标列表未设置
2. update 未调度
3. 状态逻辑错误

**解决**：
```cpp
// 检查目标列表
CCLOG("敌方建筑数量: %zu", s_enemyBuildings ? s_enemyBuildings->size() : 0);

// 检查 update 是否被调用
void Soldier::update(float dt) {
    CCLOG("[Soldier %d] update called, dt=%.3f", _id, dt);
    // ...
}
```

---

### Q11: 建筑放置位置不对

**可能原因**：
1. 坐标转换错误
2. 锚点设置不正确

**解决**：
```cpp
// 调试坐标转换
Vec2 touchWorld = touch->getLocation();
Vec2 localPos = gridMap->convertToNodeSpace(touchWorld);
Vec2 gridPos = gridMap->worldToGrid(localPos);

CCLOG("触摸世界: (%.0f, %.0f)", touchWorld.x, touchWorld.y);
CCLOG("地图本地: (%.0f, %.0f)", localPos.x, localPos.y);
CCLOG("网格坐标: (%.0f, %.0f)", gridPos.x, gridPos.y);
```

---

### Q12: 存档加载后数据不对

**可能原因**：
1. 序列化/反序列化字段不匹配
2. JSON 解析错误
3. 版本兼容问题

**解决**：
```cpp
// 打印读取的 JSON
CCLOG("存档内容:\n%s", jsonData.c_str());

// 检查解析结果
if (doc.HasParseError()) {
    CCLOG("JSON 解析错误: offset=%zu, code=%d",
        doc.GetErrorOffset(), doc.GetParseError());
}
```

---

## 性能问题

### Q13: 帧率过低

**排查步骤**：
1. 开启性能统计：`Director::getInstance()->setDisplayStats(true);`
2. 检查 GL calls 数量
3. 使用性能分析器定位瓶颈

**常见优化**：
- 使用纹理图集减少 GL calls
- 对象池减少 new/delete
- 降低 update 频率

---

### Q14: 内存占用过高

**排查步骤**：
1. 检查纹理缓存大小
2. 检查是否有内存泄漏

**解决**：
```cpp
// 清理未使用的纹理
Director::getInstance()->getTextureCache()->removeUnusedTextures();

// 清理未使用的帧
SpriteFrameCache::getInstance()->removeUnusedSpriteFrames();
```

---

## Android 特定问题

### Q15: APK 闪退

**排查步骤**：
1. 查看 Logcat：`adb logcat | findstr "FATAL\|Exception"`
2. 检查原生库是否加载：`adb logcat | findstr "System.loadLibrary"`

---

### Q16: 存档路径访问失败

**原因**：Android 权限问题

**解决**：
使用内部存储（无需权限）：
```cpp
std::string path = FileUtils::getInstance()->getWritablePath();
// /data/data/com.yourcompany.app/files/
```

---

## 调试技巧

### 日志模板

```cpp
// 统一日志格式
#define LOG_TAG "[MyClass]"
#define LOG_INFO(fmt, ...) CCLOG(LOG_TAG " " fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) CCLOG(LOG_TAG " ERROR: " fmt, ##__VA_ARGS__)

// 使用
LOG_INFO("初始化完成");
LOG_ERROR("配置加载失败: %s", path.c_str());
```

### 可视化调试

```cpp
// 绘制碰撞框
void debugDrawBounds(Node* node) {
    auto draw = DrawNode::create();
    Rect bounds = node->getBoundingBox();
    draw->drawRect(bounds.origin, 
        bounds.origin + Vec2(bounds.size.width, bounds.size.height),
        Color4F::RED);
    node->getParent()->addChild(draw, 1000);
}

// 绘制攻击范围
void debugDrawRange(Node* node, float range) {
    auto draw = DrawNode::create();
    draw->drawCircle(Vec2::ZERO, range, 0, 32, false, Color4F::GREEN);
    node->addChild(draw, -1);
}
```

---

## 获取更多帮助

1. **查看日志**：使用 CCLOG 输出详细信息
2. **断点调试**：在 Visual Studio 中设置断点
3. **简化问题**：创建最小复现案例
4. **查看源码**：Cocos2d-x 引擎源码在 `cocos2d/` 目录

---

## 反馈问题

如果遇到无法解决的问题：

1. 记录详细的错误信息
2. 提供复现步骤
3. 附上相关代码片段
4. 说明运行环境（系统、编译器版本等）
