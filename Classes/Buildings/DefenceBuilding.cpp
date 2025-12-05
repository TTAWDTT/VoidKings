// Defence.cpp
#include "DefenceBuilding.h"

// 从config中创建
DefenceBuilding* DefenceBuilding::create(const DefenceBuildingConfig* config, int level) {
    DefenceBuilding* pRet = new(std::nothrow) DefenceBuilding(); // 在堆内存（Heap）中申请一块内存来存放 DefenceBuilding 对象 - nothrow会安全一点,内存不够时返回nullptr,而不是直接崩溃
    if (pRet && pRet->init(config, level)) { // 调用init进行初始化
        pRet->autorelease(); // 意思为：这一帧结束时,如果没有被addChild,就自动滚
        return pRet;
    }
    delete pRet;
    return nullptr; // 没内存就滚
}

bool DefenceBuilding::init(const DefenceBuildingConfig* config, int level) {
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
    _currentDP = getCurrentDP();
    _currentATK_SPEED = getCurrentATK_SPEED();
    _currentATK = getCurrentATK();
    _currentATK_RANGE = getCurrentATK_RANGE();

    // 4. 获得占地尺寸
    _length = getLength();
    _width = getWidth();

    // 5. 创建外观 (根据配置里的图片路径)
    _bodySprite = cocos2d::Sprite::createWithSpriteFrameName(_config->spriteFrameName);
    this->addChild(_bodySprite); // 这个大概就体现了逻辑实体和显示实体的组合

    // 6. 开启 Update 循环
    this->scheduleUpdate();

    return true;
}

void DefenceBuilding::setLevel(int level) {
    if (level < 0) level = 0;
    if (level > _config->MAXLEVEL) level = _config->MAXLEVEL;
    
    // 保存血量百分比
    float hpPercent = _currentHP / getCurrentMaxHP();
    
    _level = level;
}

float DefenceBuilding::getCurrentMaxHP() const {
    if (_level >= 0 && _level < _config->HP.size()) {
        return _config->HP[_level];
    }
    return _config->HP.empty() ? 0.0f : _config->HP[0];
}
float DefenceBuilding::getCurrentDP() const {
    if (_level >= 0 && _level < _config->DP.size()) {
        return _config->DP[_level];
    }
    return _config->DP.empty() ? 0.0f : _config->DP[0];
}
float DefenceBuilding::getCurrentATK_SPEED() const {
    if (_level >= 0 && _level < _config->ATK_SPEED.size()) {
        return _config->ATK_SPEED[_level];
    }
    return _config->ATK_SPEED.empty() ? 0.0f : _config->ATK_SPEED[0];
}
float DefenceBuilding::getCurrentATK() const {
    if (_level >= 0 && _level < _config->ATK.size()) {
        return _config->ATK[_level];
    }
    return _config->ATK.empty() ? 0.0f : _config->ATK[0];
}
float DefenceBuilding::getCurrentATK_RANGE() const {
    if (_level >= 0 && _level < _config->ATK_RANGE.size()) {
        return _config->ATK_RANGE[_level];
    }
    return _config->ATK_RANGE.empty() ? 0.0f : _config->ATK_RANGE[0];
}
float DefenceBuilding::getCurrentHP() const {
    return _currentHP;
}
int DefenceBuilding::getLength() const {
    return _config->length;
}
int DefenceBuilding::getWidth() const {
    return _config->width;
}

void DefenceBuilding::update(float dt) {
    // 简单的状态机逻辑
    if (!_target) {
        findTarget(); // 如果没有目标,去寻找目标
    }
    else {
        // 计算距离
        float dist = this->getPosition().distance(_target->getPosition());
        // 读配置里的射程 - 使用当前等级的属性
        if (dist <= getCurrentATK_RANGE()) {
            attackTarget(); // 攻击
        }

    }
}

// 索敌）
void DefenceBuilding::findTarget() {
    // 获取场景里所有的Soldier (伪代码,假设有个 GameScene 单例管理士兵)
    auto soldiers = GameWorld::getInstance()->getAllSoldiers();

    // 根据配置里的 SKY_ABLE 进行筛选
    if (_config->SKY_ABLE) {
        // 可以攻击空中单位，直接选择最近的士兵
        _target = findNearestSoldier(soldiers, true);
        return;
    }
    // 不能攻击空中单位，优先选择地面单位
    else {
        _target = findNearestSoldier(soldiers, false);
        if (_target) return; // 找到地面单位就返回
    }
}


void DefenceBuilding::takeDamage(float damage) {
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;
    
    // 死亡处理
    if (_currentHP <= 0) {
        // 处理死亡逻辑
    }
}

void DefenceBuilding::attackTarget() {
    if (!_target) return;
    
    // 使用当前等级的攻击力
    float attackPower = getCurrentATK();
    // 攻击逻辑...
}