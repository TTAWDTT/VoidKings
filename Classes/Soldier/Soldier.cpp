// Soldier.cpp
#include "Soldier.h"

// 从config中创建
Soldier* Soldier::create(const UnitConfig* config, int level) {
    Soldier* pRet = new(std::nothrow) Soldier(); // 在堆内存（Heap）中申请一块内存来存放 Soldier 对象 - nothrow会安全一点,内存不够时返回nullptr,而不是直接崩溃
    if (pRet && pRet->init(config, level)) { // 调用init进行初始化
        pRet->autorelease(); // 意思为：这一帧结束时,如果没有被addChild,就自动滚
        return pRet;
    }
    delete pRet;
    return nullptr; // 没内存就滚
}

bool Soldier::init(const UnitConfig* config, int level) {
    // 底层数据初始化
    if (!Node::init()) return false;

    // 1. 绑定配置
    _config = config; // 这个写指针绑定,而不是拷贝,节约内存

    // 2. 设置等级并验证
    _level = level;
    if (_level < 0) _level = 0;
    if (_level > _config->MAXLEVEL) _level = _config->MAXLEVEL;
    
    // 3. 初始化运行时状态 - 使用level索引访问属性
    _currentHP = getCurrentMaxHP();
    // 省略一部分

    // 4. 创建外观 (根据配置里的图片路径)
    _bodySprite = cocos2d::Sprite::createWithSpriteFrameName(_config->spriteFrameName);
	_healthBar = cocos2d::Sprite::createWithSpriteFrameName("health_bar.png"); // 假设有个通用的血条图片
    this->addChild(_bodySprite); // 这个大概就体现了逻辑实体和显示实体的组合
	this->addChild(_healthBar);

    // 实现血条初始视图
	// 对于其他类的血条实现,可以参考这里的思路，注意调整位置和缩放
    if (_healthBar) {
        // 使前景的锚点为左中，以便 scaleX 从左向右裁剪
        _healthBar->setAnchorPoint(cocos2d::Vec2(0.0f, 0.5f));

        // 把血条放置在士兵头顶偏上位置（根据 bodySprite 大小调整 offset）
        float offsetY = 10.0f; // 根据画面微调
        if (_bodySprite) {
            float y = _bodySprite->getContentSize().height * 0.5f + offsetY;
            // 由于 anchor.x=0，把血条左端对齐士兵中心的左侧: x = -width/2
            float x = -_healthBar->getContentSize().width * 0.5f;
            _healthBar->setPosition(cocos2d::Vec2(x, y));
        }
        else {
            _healthBar->setPosition(cocos2d::Vec2(0, 30.0f));
        }

        // 确保初始为满血显示
        _healthBar->setScaleX(1.0f);
        // 默认颜色（可自定义）
        _healthBar->setColor(cocos2d::Color3B::GREEN);
    }

	updateHealthBar(false); // 创建后调用一次，确保血条显示正确

    // 5. 开启 Update 循环
    this->scheduleUpdate();

    return true;
}

void Soldier::setLevel(int level) {
    if (level < 0) level = 0;
    if (level > _config->MAXLEVEL) level = _config->MAXLEVEL;
    
    //// 保存血量百分比
    //float hpPercent = _currentHP / getCurrentMaxHP();
    //
    _level = level;
    //
    //// 按新等级的最大血量恢复相同百分比的血量
    //_currentHP = getCurrentMaxHP() * hpPercent;
}

float Soldier::getCurrentMaxHP() const {
    if (_level >= 0 && _level < _config->HP.size()) {
        return _config->HP[_level];
    }
    return _config->HP.empty() ? 0.0f : _config->HP[0];
}

float Soldier::getSpeed() const {
    if (_level >= 0 && _level < _config->SPEED.size()) {
        return _config->SPEED[_level];
    }
    return _config->SPEED.empty() ? 0.0f : _config->SPEED[0];
}

float Soldier::getATK() const {
    if (_level >= 0 && _level < _config->ATK.size()) {
        return _config->ATK[_level];
    }
    return _config->ATK.empty() ? 0.0f : _config->ATK[0];
}

float Soldier::getRange() const {
    if (_level >= 0 && _level < _config->RANGE.size()) {
        return _config->RANGE[_level];
    }
    return _config->RANGE.empty() ? 0.0f : _config->RANGE[0];
}

float Soldier::getHP() const {
    return _currentHP;
}

void Soldier::update(float dt) {
    // 简单的状态机逻辑
    if (!_target) {
        findTarget(); // 如果没有目标,去寻找目标
    }
    else {
        // 计算距离
        float dist = this->getPosition().distance(_target->getPosition());

        // 读配置里的射程 - 使用当前等级的属性
        if (dist <= getCurrentRange()) {
            attackTarget(); // 攻击
        }
        else {
            moveToTarget(dt); // 移动
        }
    }
}

// 这里体现了哥布林和野蛮人的区别（索敌）
void Soldier::findTarget() {
    // 获取场景里所有的建筑 (伪代码,假设有个 GameScene 单例管理建筑)
    auto buildings = GameWorld::getInstance()->getAllBuildings();

    // 根据配置里的 aiType 进行筛选
    switch (_config->aiType) {
    case TargetPriority::RESOURCE:
        // 哥布林逻辑：遍历 buildings,找到距离最近的 && 类型是资源的
        _target = findNearestBuilding(buildings, BuildingType::RESOURCE);
        break;

    case TargetPriority::DEFENSE:
        // 胖子逻辑：找防御塔
        _target = findNearestBuilding(buildings, BuildingType::DEFENSE);
        break;

    default:
        // 野蛮人逻辑：找任意最近的
        _target = findNearestBuilding(buildings, BuildingType::ANY);
        break;
    }
}

void Soldier::moveToTarget(float dt) {
    if (!_target) return;

    cocos2d::Vec2 direction = (_target->getPosition() - this->getPosition()).getNormalized();
    // 使用配置里的 moveSpeed - 使用当前等级的速度
    cocos2d::Vec2 newPos = this->getPosition() + (direction * getCurrentSpeed() * dt);
    this->setPosition(newPos);
}

void Soldier::takeDamage(float damage) {
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;

    // 更新血条（带动画）
    updateHealthBar();

    // 死亡处理
    if (_currentHP <= 0) {
        // 处理死亡逻辑
    }
}

void Soldier::attackTarget() {
    if (!_target) return;
    
    // 使用当前等级的攻击力
    float attackPower = getCurrentATK();
    // 攻击逻辑...
}

// 更新血条显示
void Soldier::updateHealthBar() {
    if (!_healthBar || !_config) return;

    // 获取最大血量并计算比例
    float maxHP = getCurrentMaxHP();
    float pct = 0.0f;
    if (maxHP > 0.00001f) {
        pct = _currentHP / maxHP;
    }
    // clamp 到 [0,1]
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;

    float targetScaleX = pct;

    // 如果 animate，则平滑过渡；否则直接设置
    // 这个我觉得直接设置就好了，如果要改为平滑，记得在参数中添加一个  bool animate
    //if (animate) {
    //    _healthBar->stopAllActions();
    //    // 使用短动画平滑变化
    //    auto action = cocos2d::ScaleTo::create(0.12f, targetScaleX, 1.0f);
    //    _healthBar->runAction(action);
    //}
    //else {
    //    _healthBar->setScaleX(targetScaleX);
    //}
    _healthBar->setScaleX(targetScaleX)

    // 可选：根据百分比改变颜色（绿色->黄色->红色）
    if (pct > 0.5f) {
        _healthBar->setColor(cocos2d::Color3B::GREEN);
    }
    else if (pct > 0.2f) {
        _healthBar->setColor(cocos2d::Color3B::YELLOW);
    }
    else {
        _healthBar->setColor(cocos2d::Color3B::RED);
    }

    // 如果血量为0，可以隐藏或播放特殊效果
    if (pct <= 0.0f) {
        // 选项：隐藏血条
        //_healthBar->setVisible(false);
        // 或播放闪烁/破碎效果，这里只隐藏为示例
        _healthBar->setVisible(false);
    }
    else {
        _healthBar->setVisible(true);
    }
}