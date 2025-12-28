# 附录C：资源命名规范

> **本附录**：规范 VoidKings 项目中的资源文件命名方式。

---

## 目录结构

```
Resources/
├── res/                         # 配置与通用 UI
│   ├── buildings_config.json
│   ├── units_config.json
│   ├── health_bar.png
│   ├── health_bar_bg.png
│   ├── coin_spin_1.png          # 金币动画帧
│   ├── coin_spin_2.png
│   └── ...
│
├── buildings/                   # 建筑资源
│   ├── ArrowTower/
│   │   ├── arrow_tower.png      # 主贴图
│   │   ├── arrow_tower_idle_1.png  # 待机动画
│   │   ├── arrow.png            # 子弹贴图
│   │   └── ...
│   ├── BoomTower/
│   ├── MagicTower/
│   └── ...
│
├── unit/                        # 兵种资源
│   ├── Spearman_output/
│   │   ├── spearman.png         # 默认贴图
│   │   ├── spearman_idle_1.png  # 待机帧
│   │   ├── spearman_idle_2.png
│   │   ├── spearman_walk_1.png  # 行走帧
│   │   ├── spearman_attack_1.png # 攻击帧
│   │   ├── spearman_death_1.png  # 死亡帧
│   │   └── ...
│   ├── Archer_output/
│   └── ...
│
├── UI/                          # UI 资源
│   ├── button_normal.png
│   ├── button_pressed.png
│   ├── panel_bg.png
│   ├── exit.png
│   ├── retry.png
│   └── ...
│
├── effects/                     # 特效资源
│   ├── explosion.png
│   ├── ice_hit.png
│   ├── heal_glow.png
│   └── ...
│
├── fonts/                       # 字体
│   └── ScienceGothic.ttf
│
├── music/                       # 背景音乐
│   ├── bgm_main.mp3
│   ├── bgm_battle.mp3
│   └── ...
│
└── source/                      # 音效
    ├── click.wav
    ├── arrow_shoot.wav
    ├── arrow_hit.wav
    ├── boom.wav
    └── ...
```

---

## 命名规则

### 通用规则

1. **全小写**：文件名全部使用小写字母
2. **下划线分隔**：单词之间使用下划线 `_`
3. **无空格**：不使用空格或特殊字符
4. **有意义**：名称应清晰表达内容

### 动画帧命名

格式：`{baseName}_{animationType}_{frameNumber}.png`

| 部分 | 说明 | 示例 |
|------|------|------|
| baseName | 基础名称 | spearman |
| animationType | 动画类型 | idle, walk, attack, death |
| frameNumber | 帧序号（从1开始） | 1, 2, 3, ... |

**示例**：
```
spearman_idle_1.png
spearman_idle_2.png
spearman_walk_1.png
spearman_walk_2.png
spearman_attack_1.png
spearman_death_1.png
```

### 建筑资源命名

格式：`{buildingName}_{resourceType}.png`

| 资源类型 | 说明 |
|----------|------|
| (无后缀) | 主贴图 |
| _idle_N | 待机动画帧 |
| _attack_N | 攻击动画帧 |
| _destroyed | 被摧毁状态 |
| _bullet | 子弹贴图 |

**示例**：
```
arrow_tower.png
arrow_tower_idle_1.png
arrow_tower_attack_1.png
arrow.png              # 子弹
```

### UI 资源命名

格式：`{elementType}_{state}.png`

| 元素类型 | 说明 |
|----------|------|
| button | 按钮 |
| panel | 面板 |
| icon | 图标 |
| bar | 进度条 |

| 状态 | 说明 |
|------|------|
| normal | 正常状态 |
| pressed | 按下状态 |
| disabled | 禁用状态 |
| hover | 悬浮状态 |

**示例**：
```
button_normal.png
button_pressed.png
button_disabled.png
panel_bg.png
icon_coin.png
bar_health.png
bar_health_bg.png
```

### 音效命名

格式：`{action}_{detail}.wav`

**示例**：
```
click.wav
button_cancel.wav
arrow_shoot.wav
arrow_hit.wav
sword_swing.wav
explosion.wav
coin_collect.wav
victory.wav
defeat.wav
```

### 音乐命名

格式：`bgm_{scene}.mp3`

**示例**：
```
bgm_main.mp3
bgm_battle.mp3
bgm_victory.mp3
bgm_boss.mp3
```

---

## 代码中的路径引用

```cpp
// 建筑贴图
Sprite::create("buildings/ArrowTower/arrow_tower.png");

// 兵种帧（使用配置）
config.spriteFrameName = "unit/Spearman_output/spearman.png";
config.spriteBaseName = "unit/Spearman_output/spearman";

// 动画帧自动拼接
std::string framePath = StringUtils::format("%s_%s_%d.png",
    config.spriteBaseName.c_str(),  // "unit/Spearman_output/spearman"
    "idle",                          // 动画类型
    1);                              // 帧号
// 结果: "unit/Spearman_output/spearman_idle_1.png"

// 音效
AudioManager::playEffect("source/arrow_shoot.wav");

// 音乐
AudioManager::playBgm("music/bgm_battle.mp3");

// 字体
Label::createWithTTF("Hello", "fonts/ScienceGothic.ttf", 24);
```

---

## 资源检查清单

添加新资源时：

- [ ] 文件名符合命名规范
- [ ] 使用正确的格式（PNG/WAV/MP3）
- [ ] 放置在正确的目录下
- [ ] 动画帧序号连续
- [ ] 配置文件中的路径正确
- [ ] Windows 和 Android 都能加载

---

## 常见问题

### 问题1：Android 上资源加载失败

**原因**：Android 文件系统区分大小写

**解决**：确保代码中的路径与实际文件名大小写完全一致

```cpp
// 错误
Sprite::create("Unit/Spearman.png");  // ❌

// 正确
Sprite::create("unit/Spearman_output/spearman.png");  // ✓
```

### 问题2：动画帧缺失

**检查点**：
1. 帧文件是否存在
2. 帧序号是否连续（1,2,3 而非 1,2,4）
3. 配置中的 frameCount 是否正确

### 问题3：纹理过大

**建议**：
- 单张纹理不超过 2048×2048
- 使用纹理图集合并小图
- 使用压缩格式（ETC2/PVRTC）
