/**
 * @file BaseScene.cpp
 * @brief 基地场景实现文件（模块化重构版本）
 *
 * 使用模块化组件实现基地场景的各项功能：
 * - BuildShopPanel: 建筑商店面板
 * - PlacementManager: 建筑放置管理器（含格子颜色显示）
 * - BaseUIPanel: UI面板
 * - GridBackground: 网格背景
 */

#include "BaseScene.h"
#include "MainMenuScene.h"
#include "LevelSelectScene.h"
#include "Buildings/DefenceBuilding.h"
#include "Buildings/ProductionBuilding.h"
#include "Buildings/StorageBuilding.h"
#include "Buildings/Trap.h"
#include "UI/TrainPanel.h"
#include "Core/Core.h"
#include "BattleScene.h"
#include "Share/BattleShareManager.h"
#include "Save/SaveManager.h"
#include "Soldier/UnitManager.h"
#include "Utils/AudioManager.h"
#include "Utils/GameSettings.h"
#include "Utils/NodeUtils.h"
#include <algorithm>
#include <cmath>

USING_NS_CC;

namespace {
const char* kBaseFont = "fonts/ScienceGothic.ttf";

Label* createBaseLabel(const std::string& text, float fontSize) {
    auto label = Label::createWithTTF(text, kBaseFont, fontSize);
    if (!label) {
        label = Label::create();
        label->setString(text);
        label->setSystemFontSize(fontSize);
    }
    return label;
}

constexpr float kHoverPanelPaddingX = 16.0f;
constexpr float kHoverPanelPaddingY = 14.0f;
constexpr float kHoverPanelMinWidth = 260.0f;
constexpr float kHoverPanelMinHeight = 140.0f;
constexpr float kHoverUpgradeWidth = 78.0f;
constexpr float kHoverUpgradeHeight = 18.0f;
constexpr float kHoverSellWidth = 78.0f;
constexpr float kHoverSellHeight = 18.0f;
constexpr float kHoverSellGap = 6.0f;
constexpr float kHoverPanelOffsetX = 16.0f;
constexpr float kUpgradeCostFactor = 0.6f;
constexpr float kSellRefundRate = 0.5f;
constexpr int kSpikeTrapType = 11;
constexpr int kSnapTrapType = 12;
constexpr int kBaseAnchorX = 36;
constexpr int kBaseAnchorY = 36;
constexpr int kBarracksAnchorX = 30;
constexpr int kBarracksAnchorY = 36;
constexpr int kSavedBuildingTagBase = 10000;

Vec2 s_baseAnchor(static_cast<float>(kBaseAnchorX), static_cast<float>(kBaseAnchorY));
Vec2 s_barracksAnchor(static_cast<float>(kBarracksAnchorX), static_cast<float>(kBarracksAnchorY));
int s_barracksLevel = 0;
std::vector<BaseSavedBuilding> s_savedBuildings;

Vec2 getHoverAnchorWorldPos(cocos2d::Node* building) {
    if (!building) {
        return cocos2d::Vec2::ZERO;
    }
    const auto* bodySprite = NodeUtils::findBodySprite(building);
    cocos2d::Rect localBounds = bodySprite ? bodySprite->getBoundingBox() : building->getBoundingBox();
    if (localBounds.size.width <= 0.0f || localBounds.size.height <= 0.0f) {
        auto* parent = building->getParent();
        return parent ? parent->convertToWorldSpace(building->getPosition()) : building->getPosition();
    }
    cocos2d::Vec2 localRightCenter(localBounds.getMaxX(), localBounds.getMidY());
    return building->convertToWorldSpace(localRightCenter);
}

cocos2d::Rect getBuildingWorldRect(cocos2d::Node* building) {
    if (!building) {
        return cocos2d::Rect::ZERO;
    }
    const auto* bodySprite = NodeUtils::findBodySprite(building);
    if (bodySprite) {
        cocos2d::Rect localRect = bodySprite->getBoundingBox();
        cocos2d::Vec2 worldBL = building->convertToWorldSpace(localRect.origin);
        cocos2d::Vec2 worldTR = building->convertToWorldSpace(
            localRect.origin + cocos2d::Vec2(localRect.size.width, localRect.size.height));
        float minX = std::min(worldBL.x, worldTR.x);
        float minY = std::min(worldBL.y, worldTR.y);
        float maxX = std::max(worldBL.x, worldTR.x);
        float maxY = std::max(worldBL.y, worldTR.y);
        return cocos2d::Rect(minX, minY, maxX - minX, maxY - minY);
    }
    auto* parent = building->getParent();
    if (parent) {
        cocos2d::Rect localRect = building->getBoundingBox();
        cocos2d::Vec2 worldBL = parent->convertToWorldSpace(localRect.origin);
        cocos2d::Vec2 worldTR = parent->convertToWorldSpace(
            localRect.origin + cocos2d::Vec2(localRect.size.width, localRect.size.height));
        float minX = std::min(worldBL.x, worldTR.x);
        float minY = std::min(worldBL.y, worldTR.y);
        float maxX = std::max(worldBL.x, worldTR.x);
        float maxY = std::max(worldBL.y, worldTR.y);
        return cocos2d::Rect(minX, minY, maxX - minX, maxY - minY);
    }
    return cocos2d::Rect::ZERO;
}

cocos2d::Rect getHoverPanelWorldRect(const cocos2d::Node* panel,
                                    const cocos2d::LayerColor* bg) {
    if (!panel || !bg) {
        return cocos2d::Rect::ZERO;
    }
    cocos2d::Size panelSize = bg->getContentSize();
    cocos2d::Vec2 panelPos = panel->getPosition();
    return cocos2d::Rect(panelPos.x,
                         panelPos.y - panelSize.height * 0.5f,
                         panelSize.width,
                         panelSize.height);
}

cocos2d::Rect getHoverKeepRect(cocos2d::Node* building,
                              const cocos2d::Node* panel,
                              const cocos2d::LayerColor* bg) {
    cocos2d::Rect buildingRect = getBuildingWorldRect(building);
    cocos2d::Rect panelRect = getHoverPanelWorldRect(panel, bg);
    if (buildingRect.size.width <= 0.0f || buildingRect.size.height <= 0.0f) {
        return panelRect;
    }
    if (panelRect.size.width <= 0.0f || panelRect.size.height <= 0.0f) {
        return buildingRect;
    }
    float minX = std::min(buildingRect.getMinX(), panelRect.getMinX());
    float minY = std::min(buildingRect.getMinY(), panelRect.getMinY());
    float maxX = std::max(buildingRect.getMaxX(), panelRect.getMaxX());
    float maxY = std::max(buildingRect.getMaxY(), panelRect.getMaxY());
    return cocos2d::Rect(minX, minY, maxX - minX, maxY - minY);
}

} // namespace

// ==================== 场景创建与初始化 ====================

Scene* BaseScene::createScene() {
    return BaseScene::create();
}

bool BaseScene::init() {
    if (!Scene::init()) {
        return false;
    }
    GameSettings::applyBattleSpeed(false);

    // 清理战斗场景的静态引用，避免野指针
    DefenceBuilding::setEnemySoldiers(nullptr);
    TrapBase::setEnemySoldiers(nullptr);

    // 按模块化方式初始化各个组件
    initGridMap();
    initBaseBuilding();
    initUIComponents();
    initBuildingSystem();
    _effectLayer = Node::create();
    this->addChild(_effectLayer, 150);

    // 加载兵种配置
    UnitManager::getInstance()->loadConfig("res/units_config.json");

    createTrainPanel();
    initTouchListener();
    initHoverInfo();
    AudioManager::playMainBgm();

    // 定时刷新资源显示，确保生产资源同步到UI
    this->schedule([this](float) {
        if (_uiPanel) {
            _uiPanel->updateResourceDisplay(
                Core::getInstance()->getResource(ResourceType::COIN),
                Core::getInstance()->getResource(ResourceType::DIAMOND)
            );
        }
        syncBaseBuildingLevel();
        if (_hoverInfoPanel && _hoverInfoPanel->isVisible() && _hoveredBuilding) {
            updateUpgradeUI(_hoveredBuilding);
        }
    }, 0.5f, "resource_tick");

    CCLOG("[基地场景] 初始化完成（模块化版本）");

    return true;
}

const std::vector<BaseSavedBuilding>& BaseScene::getSavedBuildings() {
    return s_savedBuildings;
}

Vec2 BaseScene::getBaseAnchorGrid() {
    return s_baseAnchor;
}

Vec2 BaseScene::getBarracksAnchorGrid() {
    return s_barracksAnchor;
}

int BaseScene::getBarracksLevel() {
    return s_barracksLevel;
}

void BaseScene::applySavedState(const std::vector<BaseSavedBuilding>& buildings,
                                const Vec2& baseAnchor,
                                const Vec2& barracksAnchor,
                                int barracksLevel) {
    s_savedBuildings = buildings;
    s_baseAnchor = baseAnchor;
    s_barracksAnchor = barracksAnchor;
    s_barracksLevel = barracksLevel;
}

void BaseScene::resetSavedState() {
    s_savedBuildings.clear();
    s_baseAnchor = Vec2(static_cast<float>(kBaseAnchorX), static_cast<float>(kBaseAnchorY));
    s_barracksAnchor = Vec2(static_cast<float>(kBarracksAnchorX), static_cast<float>(kBarracksAnchorY));
    s_barracksLevel = 0;
}

// ==================== 网格地图初始化 ====================

void BaseScene::initGridMap() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 创建网格地图（80x80格子，每格32像素 - 4倍于原来的40x40）
    _gridMap = GridMap::create(80, 80, 32.0f);
    
    // 计算初始位置：将视图中心对准地图中心（建筑区域）
    // 建筑放置在(36,36)附近，将视图移动使其可见
    // 1280x720分辨率下，需要确保建筑区域在屏幕中心
    float cellSize = 32.0f;
    float mapCenterX = 36.0f * cellSize;  // 建筑区域中心X
    float mapCenterY = 36.0f * cellSize;  // 建筑区域中心Y
    
    // 计算偏移量：屏幕中心 - 地图上的目标点
    // 为左侧UI预留空间，地图中心略向右偏
    float targetCenterX = origin.x + visibleSize.width * 0.60f;
    float targetCenterY = origin.y + visibleSize.height * 0.52f;
    float offsetX = targetCenterX - mapCenterX;
    float offsetY = targetCenterY - mapCenterY;

    _gridMap->setPosition(Vec2(offsetX, offsetY));
    this->addChild(_gridMap, 0);
    _gridMap->showGrid(GameSettings::getShowGrid());

    // 创建建筑层（作为GridMap的子节点）
    _buildingLayer = Node::create();
    _gridMap->addChild(_buildingLayer, 10);

    // 创建网格背景组件
    _gridBackground = GridBackground::create(80, 80, 32.0f);
    if (_gridBackground) {
        _gridMap->addChild(_gridBackground, -1);
    }

    CCLOG("[基地场景] 网格地图初始化完成（80x80格子），视图已居中到建筑区域");
}

// ==================== UI组件初始化 ====================

void BaseScene::initUIComponents() {
    // 创建UI面板组件，传入回调函数
    BaseUICallbacks callbacks;
    callbacks.onAttack = [this]() { this->onAttackClicked(); };
    callbacks.onBuild = [this]() { this->onBuildClicked(); };
    callbacks.onExit = [this]() { this->onExitClicked(); };
    callbacks.onAsync = [this]() { this->onAsyncClicked(); };

    _uiPanel = BaseUIPanel::create(callbacks);
    if (_uiPanel) {
        this->addChild(_uiPanel, 100);
    }

    setupAsyncPanel();

    CCLOG("[基地场景] UI组件初始化完成");
}

// ==================== 建造系统初始化 ====================

void BaseScene::initBuildingSystem() {
    // 创建建筑商店面板
    _buildShopPanel = BuildShopPanel::create(
        // 建筑选择回调
        [this](const BuildingOption& option) {
            this->onBuildingSelected(option);
        },
        // 关闭回调
        [this]() {
            CCLOG("[基地场景] 建筑商店关闭");
            if (_uiPanel && (!_placementManager || !_placementManager->isPlacing())) {
                _uiPanel->setButtonsEnabled(true);
            }
        }
    );
    if (_buildShopPanel) {
        this->addChild(_buildShopPanel, 101);
    }

    // 创建放置管理器
    _placementManager = PlacementManager::create(
        _gridMap,
        // 放置确认回调
        [this](const BuildingOption& option, int gridX, int gridY) {
            this->onPlacementConfirmed(option, gridX, gridY);
        },
        // 放置取消回调
        [this]() {
            CCLOG("[基地场景] 建筑放置取消");
            AudioManager::playButtonCancel();
            if (_uiPanel) {
                _uiPanel->setButtonsEnabled(true);
            }
        }
    );
    if (_placementManager) {
        _gridMap->addChild(_placementManager, 50);
    }

    CCLOG("[基地场景] 建造系统初始化完成");
}

// ==================== 训练面板创建 ====================

void BaseScene::createTrainPanel() {
    _trainPanel = TrainPanel::create(
        // 训练完成回调
        [this](int unitId) {
            this->onUnitTrainComplete(unitId);
        },
        // 关闭回调
        [this]() {
            CCLOG("[基地场景] 训练面板关闭");
            if (_uiPanel) {
                _uiPanel->updateResourceDisplay(
                    Core::getInstance()->getResource(ResourceType::COIN),
                    Core::getInstance()->getResource(ResourceType::DIAMOND)
                );
                _uiPanel->setButtonsEnabled(true);
            }
        }
    );

    if (_trainPanel) {
        this->addChild(_trainPanel, 102);
        CCLOG("[基地场景] 训练面板创建成功");
    }
}

void BaseScene::showTrainPanel() {
    if (_trainPanel) {
        if (_uiPanel) {
            _uiPanel->setButtonsEnabled(false);
        }
        AudioManager::playButtonClick();
        _trainPanel->show();
        CCLOG("[基地场景] 打开训练面板");
    }
}

void BaseScene::onUnitTrainComplete(int unitId) {
    CCLOG("[基地场景] 兵种训练完成: ID=%d", unitId);
    if (_uiPanel) {
        _uiPanel->updateResourceDisplay(
            Core::getInstance()->getResource(ResourceType::COIN),
            Core::getInstance()->getResource(ResourceType::DIAMOND)
        );
    }
}

void BaseScene::onAsyncClicked() {
    AudioManager::playButtonClick();
    toggleAsyncPanel();
}

void BaseScene::toggleAsyncPanel() {
    if (_asyncPanelVisible) {
        hideAsyncPanel();
    }
    else {
        showAsyncPanel();
    }
}

void BaseScene::showAsyncPanel() {
    if (!_asyncPanel) {
        setupAsyncPanel();
    }
    if (!_asyncPanel) {
        return;
    }
    _asyncPanel->setVisible(true);
    _asyncPanelVisible = true;
    updateAsyncStatus("Drop opponent target_base_snapshot.json / target_replay.json into the share folder.");
}

void BaseScene::hideAsyncPanel() {
    if (_asyncPanel) {
        _asyncPanel->setVisible(false);
    }
    _asyncPanelVisible = false;
}

void BaseScene::setupAsyncPanel() {
    if (_asyncPanel) {
        return;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto overlay = Layout::create();
    if (!overlay) {
        return;
    }
    overlay->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    overlay->setBackGroundColor(Color3B::BLACK);
    overlay->setBackGroundColorOpacity(160);
    overlay->setContentSize(visibleSize);
    overlay->setAnchorPoint(Vec2::ZERO);
    overlay->setPosition(origin);
    overlay->setTouchEnabled(true);
    overlay->setSwallowTouches(true);
    overlay->setPropagateTouchEvents(true);
    overlay->setVisible(false);
    this->addChild(overlay, 400);
    _asyncPanel = overlay;

    float cardWidth = std::min(780.0f, visibleSize.width * 0.78f);
    float cardHeight = std::min(560.0f, visibleSize.height * 0.85f);
    auto card = LayerColor::create(Color4B(24, 24, 24, 235), cardWidth, cardHeight);
    card->setIgnoreAnchorPointForPosition(false);
    card->setAnchorPoint(Vec2(0.5f, 0.5f));
    card->setPosition(Vec2(origin.x + visibleSize.width * 0.5f,
        origin.y + visibleSize.height * 0.5f));
    card->setName("asyncCard");
    _asyncPanel->addChild(card, 1);

    overlay->addTouchEventListener([this, card](Ref* sender, Widget::TouchEventType type) {
        if (!_asyncPanelVisible || !card) {
            return;
        }
        if (type == Widget::TouchEventType::ENDED) {
            auto widget = dynamic_cast<Widget*>(sender);
            if (!widget) {
                return;
            }
            Vec2 worldPos = widget->getTouchEndPosition();
            Vec2 local = card->convertToNodeSpace(worldPos);
            Rect bounds(0.0f, 0.0f, card->getContentSize().width, card->getContentSize().height);
            if (!bounds.containsPoint(local)) {
                hideAsyncPanel();
            }
        }
    });

    auto cardBorder = DrawNode::create();
    cardBorder->drawRect(Vec2::ZERO, Vec2(cardWidth, cardHeight), Color4F(1, 1, 1, 0.15f));
    card->addChild(cardBorder, 2);

    auto makeLabel = [](const std::string& text,
        float fontSize,
        const Color3B& color = Color3B::WHITE,
        TextHAlignment alignment = TextHAlignment::LEFT,
        TextVAlignment vAlignment = TextVAlignment::CENTER) -> Label* {
            auto label = createBaseLabel(text, fontSize);
            if (!label) {
                return nullptr;
            }
            label->setColor(color);
            label->setAlignment(alignment);
            label->setVerticalAlignment(vAlignment);
            label->setOverflow(Label::Overflow::RESIZE_HEIGHT);
            return label;
    };

    auto title = makeLabel("ASYNC OPERATIONS", 30, Color3B(255, 255, 255), TextHAlignment::CENTER);
    if (title) {
        title->setPosition(Vec2(cardWidth * 0.5f, cardHeight - 32.0f));
        card->addChild(title, 2);
    }

    auto subTitle = makeLabel("Share base snapshots and battle replays for asynchronous attacks.", 18,
        Color3B(195, 195, 195), TextHAlignment::CENTER);
    if (subTitle) {
        subTitle->setPosition(Vec2(cardWidth * 0.5f, cardHeight - 64.0f));
        card->addChild(subTitle, 2);
    }

    auto* shareMgr = BattleShareManager::getInstance();
    std::string shareDir = shareMgr ? shareMgr->getShareDirectory() : "";
    auto pathBg = LayerColor::create(Color4B(45, 55, 80, 230), cardWidth - 64.0f, 48.0f);
    pathBg->setPosition(Vec2(32.0f, cardHeight - 120.0f));
    card->addChild(pathBg);
    auto pathLabel = makeLabel("Share directory: " + shareDir, 16, Color3B::WHITE);
    if (pathLabel) {
        pathLabel->setAnchorPoint(Vec2(0, 0.5f));
        pathLabel->setPosition(Vec2(38.0f, cardHeight - 96.0f));
        pathLabel->setDimensions(cardWidth - 80.0f, 32.0f);
        card->addChild(pathLabel, 2);
    }

    auto note = makeLabel("Place opponent target_base_snapshot.json / target_replay.json into the directory above.", 15,
        Color3B(185, 185, 185));
    if (note) {
        note->setAnchorPoint(Vec2(0, 0.5f));
        note->setPosition(Vec2(38.0f, cardHeight - 140.0f));
        note->setDimensions(cardWidth - 76.0f, 48.0f);
        card->addChild(note, 2);
    }

    float sectionWidth = cardWidth - 60.0f;
    float sectionHeight = 170.0f;
    float sectionLeft = (cardWidth - sectionWidth) * 0.5f;
    auto createSection = [&](const Color4B& bgColor, float topY) -> LayerColor* {
        auto bg = LayerColor::create(bgColor, sectionWidth, sectionHeight);
        if (!bg) {
            return nullptr;
        }
        bg->setAnchorPoint(Vec2::ZERO);
        bg->setPosition(Vec2(sectionLeft, topY - sectionHeight));
        card->addChild(bg, 1);

        auto border = DrawNode::create();
        border->drawRect(Vec2::ZERO, Vec2(sectionWidth, sectionHeight), Color4F(1, 1, 1, 0.08f));
        bg->addChild(border, 1);

        return bg;
    };

    auto baseBox = createSection(Color4B(36, 52, 42, 230), cardHeight - 180.0f);
    if (baseBox) {
        float h = baseBox->getContentSize().height;
        auto baseTitle = makeLabel("BASE SNAPSHOT", 20);
        if (baseTitle) {
            baseTitle->setAnchorPoint(Vec2(0, 0.5f));
            baseTitle->setPosition(Vec2(16.0f, h - 24.0f));
            baseBox->addChild(baseTitle, 2);
        }

        auto baseDesc = makeLabel("Export my_base_snapshot.json to share your layout; load target_base_snapshot.json to attack imported bases.",
            15, Color3B(215, 215, 215));
        if (baseDesc) {
            baseDesc->setAnchorPoint(Vec2(0, 1.0f));
            baseDesc->setDimensions(sectionWidth - 32.0f, 78.0f);
            baseDesc->setPosition(Vec2(16.0f, h - 48.0f));
            baseBox->addChild(baseDesc, 2);
        }

        auto exportBaseBtn = createAsyncButton("EXPORT MY BASE", [this]() {
            handleExportBase();
            }, Size(200.0f, 44.0f), Color4B(70, 116, 80, 255), Color4B(56, 96, 64, 255));
        auto importBaseBtn = createAsyncButton("LOAD TARGET & ATTACK", [this]() {
            handleImportBaseAttack();
            }, Size(200.0f, 44.0f), Color4B(70, 92, 135, 255), Color4B(58, 76, 112, 255));
        if (exportBaseBtn && importBaseBtn) {
            float btnY = 36.0f;
            exportBaseBtn->setPosition(Vec2(sectionWidth * 0.5f - 115.0f, btnY));
            importBaseBtn->setPosition(Vec2(sectionWidth * 0.5f + 115.0f, btnY));
            baseBox->addChild(exportBaseBtn);
            baseBox->addChild(importBaseBtn);
        }
    }

    auto replayBox = createSection(Color4B(44, 40, 48, 230), cardHeight - 370.0f);
    if (replayBox) {
        float h = replayBox->getContentSize().height;
        auto replayTitle = makeLabel("BATTLE REPLAY", 20);
        if (replayTitle) {
            replayTitle->setAnchorPoint(Vec2(0, 0.5f));
            replayTitle->setPosition(Vec2(16.0f, h - 24.0f));
            replayBox->addChild(replayTitle, 2);
        }

        auto replayDesc = makeLabel("Export last_replay.json to share the latest battle; drop target_replay.json into the directory to watch.",
            15, Color3B(215, 215, 215));
        if (replayDesc) {
            replayDesc->setAnchorPoint(Vec2(0, 1.0f));
            replayDesc->setDimensions(sectionWidth - 32.0f, 78.0f);
            replayDesc->setPosition(Vec2(16.0f, h - 48.0f));
            replayBox->addChild(replayDesc, 2);
        }

        auto exportReplayBtn = createAsyncButton("EXPORT LAST REPLAY", [this]() {
            handleExportReplay();
            }, Size(200.0f, 44.0f), Color4B(120, 92, 50, 255), Color4B(102, 78, 40, 255));
        auto importReplayBtn = createAsyncButton("PLAY IMPORTED REPLAY", [this]() {
            handleImportReplay();
            }, Size(200.0f, 44.0f), Color4B(83, 70, 120, 255), Color4B(68, 56, 98, 255));
        if (exportReplayBtn && importReplayBtn) {
            float btnY = 36.0f;
            exportReplayBtn->setPosition(Vec2(sectionWidth * 0.5f - 115.0f, btnY));
            importReplayBtn->setPosition(Vec2(sectionWidth * 0.5f + 115.0f, btnY));
            replayBox->addChild(exportReplayBtn);
            replayBox->addChild(importReplayBtn);
        }
    }

    auto statusBg = LayerColor::create(Color4B(18, 18, 18, 230), cardWidth - 60.0f, 72.0f);
    statusBg->setPosition(Vec2(30.0f, 40.0f));
    statusBg->setName("asyncStatusBg");
    card->addChild(statusBg, 1);

    _asyncStatusLabel = createBaseLabel("", 17);
    if (_asyncStatusLabel) {
        _asyncStatusLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
        _asyncStatusLabel->setAlignment(TextHAlignment::CENTER);
        _asyncStatusLabel->setTextColor(Color4B(240, 240, 240, 255));
        _asyncStatusLabel->setDimensions(statusBg->getContentSize().width - 40.0f, 66.0f);
        _asyncStatusLabel->setPosition(Vec2(statusBg->getContentSize().width * 0.5f,
            statusBg->getContentSize().height * 0.5f));
        statusBg->addChild(_asyncStatusLabel, 2);
    }

    auto closeBtn = createAsyncButton("CLOSE", [this]() {
        hideAsyncPanel();
        }, Size(110.0f, 40.0f), Color4B(140, 70, 70, 255), Color4B(166, 82, 82, 255));
    if (closeBtn) {
        closeBtn->setPosition(Vec2(cardWidth - 70.0f, cardHeight - 34.0f));
        card->addChild(closeBtn, 2);
    }
    updateAsyncStatus("Results of export/import operations will be shown here.");
}

void BaseScene::updateAsyncStatus(const std::string& text) {
    if (_asyncStatusLabel) {
        _asyncStatusLabel->setString(text);
        _asyncStatusLabel->stopAllActions();
        _asyncStatusLabel->setOpacity(255);
        _asyncStatusLabel->runAction(Sequence::create(
            DelayTime::create(4.0f),
            FadeTo::create(0.25f, 210),
            nullptr));
    }
}

Button* BaseScene::createAsyncButton(const std::string& text,
    const std::function<void()>& handler,
    const Size& size,
    const Color4B& normalColor,
    const Color4B& pressedColor) {
    auto button = Button::create();
    if (!button) {
        return nullptr;
    }
    button->setScale9Enabled(true);
    button->setContentSize(size);
    button->setAnchorPoint(Vec2(0.5f, 0.5f));
    button->setZoomScale(0.04f);
    button->setPressedActionEnabled(true);
    button->setTitleText("");

    auto normalBg = LayerColor::create(normalColor, size.width, size.height);
    normalBg->setIgnoreAnchorPointForPosition(false);
    normalBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    normalBg->setPosition(Vec2(size.width * 0.5f, size.height * 0.5f));
    normalBg->setName("btnBg");
    button->addChild(normalBg, -1);

    auto titleLabel = createBaseLabel(text, 18);
    if (titleLabel) {
        titleLabel->setPosition(Vec2(size.width * 0.5f, size.height * 0.5f));
        titleLabel->setTextColor(Color4B::WHITE);
        button->addChild(titleLabel, 1);
    }

    auto applyColor = [normalBg](const Color4B& color) {
        if (!normalBg) {
            return;
        }
        normalBg->setColor(Color3B(color.r, color.g, color.b));
        normalBg->setOpacity(color.a);
    };

    button->addTouchEventListener([applyColor, normalColor, pressedColor](Ref*, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::BEGAN) {
            applyColor(pressedColor);
        }
        else if (type == Widget::TouchEventType::ENDED || type == Widget::TouchEventType::CANCELED) {
            applyColor(normalColor);
        }
        });

    button->addClickEventListener([handler](Ref*) {
        if (handler) {
            handler();
        }
        });
    return button;
}

void BaseScene::handleExportBase() {
    auto* shareMgr = BattleShareManager::getInstance();
    if (!shareMgr) {
        updateAsyncStatus("Share manager unavailable.");
        return;
    }
    std::string path;
    if (shareMgr->exportPlayerBaseSnapshot(&path)) {
        updateAsyncStatus("Base snapshot exported to: " + path);
    }
    else {
        updateAsyncStatus("Export failed. Please confirm the base is loaded.");
    }
}

void BaseScene::handleImportBaseAttack() {
    auto* shareMgr = BattleShareManager::getInstance();
    if (!shareMgr) {
        updateAsyncStatus("Share manager unavailable.");
        return;
    }
    BaseSnapshot snapshot;
    if (!shareMgr->loadIncomingSnapshot(&snapshot)) {
        updateAsyncStatus("Failed to load target_base_snapshot.json.");
        return;
    }
    shareMgr->setActiveTargetSnapshot(snapshot);
    auto deployUnits = UnitManager::getInstance()->getTrainedUnits();
    bool allowDefault = deployUnits.empty();
    auto scene = BattleScene::createSnapshotScene(snapshot, deployUnits, allowDefault);
    if (!scene) {
        updateAsyncStatus("Failed to create battle scene from the snapshot.");
        return;
    }
    updateAsyncStatus("Target base loaded. Ready to attack.");
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void BaseScene::handleExportReplay() {
    auto* shareMgr = BattleShareManager::getInstance();
    if (!shareMgr) {
        updateAsyncStatus("Share manager unavailable.");
        return;
    }
    std::string path;
    if (shareMgr->exportLastReplay(&path)) {
        updateAsyncStatus("Replay exported to: " + path);
    }
    else {
        updateAsyncStatus("Export failed: finish a battle first.");
    }
}

void BaseScene::handleImportReplay() {
    auto* shareMgr = BattleShareManager::getInstance();
    if (!shareMgr) {
        updateAsyncStatus("Share manager unavailable.");
        return;
    }
    BattleReplay replay;
    if (!shareMgr->loadIncomingReplay(&replay)) {
        updateAsyncStatus("Failed to load target_replay.json.");
        return;
    }
    auto scene = BattleScene::createReplayScene(replay);
    if (!scene) {
        updateAsyncStatus("Unable to create replay scene.");
        return;
    }
    updateAsyncStatus("Replay file loaded. Ready to play.");
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// ==================== 基地建筑初始化 ====================

void BaseScene::initBaseBuilding() {
    auto* manager = BuildingManager::getInstance();
    manager->setGridMap(_gridMap);
    manager->loadConfigs();
    float cellSize = _gridMap->getCellSize();

    int baseId = manager->getMainBaseId();
    if (baseId <= 0) {
        baseId = 3001;
    }
    const auto* baseConfig = manager->getProductionConfig(baseId);
    int baseWidth = baseConfig ? baseConfig->width : 4;
    int baseHeight = baseConfig ? baseConfig->length : 4;

    const int baseGridX = static_cast<int>(std::round(s_baseAnchor.x));
    const int baseGridY = static_cast<int>(std::round(s_baseAnchor.y));
    int baseLevel = Core::getInstance()->getBaseLevel() - 1;
    if (baseLevel < 0) {
        baseLevel = 0;
    }
    if (baseConfig && baseLevel > baseConfig->MAXLEVEL) {
        baseLevel = baseConfig->MAXLEVEL;
    }
    auto base = manager->createProductionBuilding(baseId, baseLevel);
    if (base) {
        _buildingLayer->addChild(base);
        BuildingManager::getInstance()->placeBuilding(base, baseGridX, baseGridY, baseWidth, baseHeight);
        s_baseAnchor = Vec2(static_cast<float>(baseGridX), static_cast<float>(baseGridY));

        scaleBuildingToFit(base, baseWidth, baseHeight, cellSize);
        setupProductionCollect(base);

        CCLOG("[基地场景] 基地建筑放置完成 (%dx%d格子，位置%d,%d)", baseWidth, baseHeight, baseGridX, baseGridY);
    }

    int barracksId = manager->getBarracksId();
    if (barracksId <= 0) {
        barracksId = 3002;
    }
    const auto* barracksConfig = manager->getProductionConfig(barracksId);
    int barracksWidth = barracksConfig ? barracksConfig->width : 5;
    int barracksHeight = barracksConfig ? barracksConfig->length : 5;

    const int barracksGridX = static_cast<int>(std::round(s_barracksAnchor.x));
    const int barracksGridY = static_cast<int>(std::round(s_barracksAnchor.y));
    int barracksLevel = s_barracksLevel;
    if (barracksLevel < 0) {
        barracksLevel = 0;
    }
    if (barracksConfig && barracksLevel > barracksConfig->MAXLEVEL) {
        barracksLevel = barracksConfig->MAXLEVEL;
    }
    s_barracksLevel = barracksLevel;
    auto barracks = manager->createProductionBuilding(barracksId, barracksLevel);
    if (barracks) {
        _buildingLayer->addChild(barracks);
        BuildingManager::getInstance()->placeBuilding(barracks, barracksGridX, barracksGridY, barracksWidth, barracksHeight);
        s_barracksAnchor = Vec2(static_cast<float>(barracksGridX), static_cast<float>(barracksGridY));

        scaleBuildingToFit(barracks, barracksWidth, barracksHeight, cellSize);
        setupProductionCollect(barracks);

        CCLOG("[基地场景] 兵营建筑放置完成 (%dx%d格子，位置%d,%d)", barracksWidth, barracksHeight, barracksGridX, barracksGridY);
    }

    restoreSavedBuildings();
}

// ==================== 建筑缩放调整常量 ====================
namespace BuildingScaleConfig {
    constexpr float PADDING_FACTOR = 0.85f;  // 建筑与格子边缘的间距系数
}

// ==================== 建筑缩放调整 ====================

void BaseScene::scaleBuildingToFit(Node* building, int gridWidth, int gridHeight, float cellSize) {
    if (!building) return;

    auto sprite = NodeUtils::findBodySprite(building);
    if (!sprite) return;

    // 计算目标尺寸（占据的格子空间，留一点边距）
    float targetWidth = gridWidth * cellSize * BuildingScaleConfig::PADDING_FACTOR;
    float targetHeight = gridHeight * cellSize * BuildingScaleConfig::PADDING_FACTOR;

    // 获取精灵原始尺寸
    Size originalSize = sprite->getContentSize();
    if (originalSize.width <= 0 || originalSize.height <= 0) return;

    // 计算缩放比例
    float scaleX = targetWidth / originalSize.width;
    float scaleY = targetHeight / originalSize.height;
    float scale = std::min(scaleX, scaleY);

    if (scale <= 0.0f) {
        return;
    }

    sprite->setScale(scale);
    if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
        defence->refreshHealthBarPosition();
    }
    else if (auto* production = dynamic_cast<ProductionBuilding*>(building)) {
        production->refreshHealthBarPosition();
        production->refreshCollectIconPosition();
    }
    else if (auto* storage = dynamic_cast<StorageBuilding*>(building)) {
        storage->refreshHealthBarPosition();
    }
    CCLOG("[基地场景] 建筑缩放调整: 原尺寸(%.1f, %.1f) -> 目标(%.1f, %.1f), scale=%.2f",
        originalSize.width, originalSize.height, targetWidth, targetHeight, scale);
}

// ==================== 触摸事件初始化 ====================

void BaseScene::initTouchListener() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BaseScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BaseScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BaseScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    CCLOG("[基地场景] 触摸事件监听初始化完成");
}

// ==================== 悬浮信息初始化 ====================

void BaseScene::initHoverInfo() {
    _hoverInfoPanel = Node::create();
    _hoverInfoPanel->setAnchorPoint(Vec2(0, 0.5f));
    _hoverInfoPanel->setIgnoreAnchorPointForPosition(false);
    _hoverInfoPanel->setVisible(false);
    this->addChild(_hoverInfoPanel, 200);

    _hoverInfoBg = LayerColor::create(Color4B(12, 12, 12, 128), 260, 140);
    _hoverInfoBg->setAnchorPoint(Vec2(0, 0));
    _hoverInfoBg->setIgnoreAnchorPointForPosition(false);
    _hoverInfoPanel->addChild(_hoverInfoBg);

    auto border = DrawNode::create();
    border->setName("hoverBorder");
    _hoverInfoPanel->addChild(border, 1);

    _hoverInfoLabel = createBaseLabel("", 18);
    _hoverInfoLabel->setAnchorPoint(Vec2(0, 1));
    _hoverInfoLabel->setAlignment(TextHAlignment::LEFT);
    _hoverInfoLabel->setTextColor(Color4B(255, 255, 255, 255));
    _hoverInfoLabel->enableShadow(Color4B(0, 0, 0, 200), Size(1, -1), 2);
    _hoverInfoLabel->setWidth(300);
    _hoverInfoPanel->addChild(_hoverInfoLabel);

    setupHoverUpgradeUI();
    setupHoverSellUI();

    if (_gridMap) {
        _hoverRangeNode = DrawNode::create();
        _hoverFootprintNode = DrawNode::create();
        _hoverRangeNode->setVisible(false);
        _hoverFootprintNode->setVisible(false);
        _gridMap->addChild(_hoverRangeNode, 25);
        _gridMap->addChild(_hoverFootprintNode, 26);
    }

    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = [this](EventMouse* event) {
        updateHoverInfo(Vec2(event->getCursorX(), event->getCursorY()));
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

void BaseScene::setupHoverUpgradeUI() {
    if (!_hoverInfoPanel) {
        return;
    }

    _hoverUpgradeNode = Node::create();
    _hoverUpgradeNode->setName("hoverUpgradeNode");
    _hoverUpgradeNode->setVisible(false);
    _hoverInfoPanel->addChild(_hoverUpgradeNode, 2);

    _hoverUpgradeBg = LayerColor::create(Color4B(60, 60, 60, 255), kHoverUpgradeWidth, kHoverUpgradeHeight);
    _hoverUpgradeBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    _hoverUpgradeBg->setIgnoreAnchorPointForPosition(false);
    _hoverUpgradeBg->setName("hoverUpgradeBg");
    _hoverUpgradeNode->addChild(_hoverUpgradeBg);

    _hoverUpgradeBorder = DrawNode::create();
    _hoverUpgradeBorder->setName("hoverUpgradeBorder");
    _hoverUpgradeNode->addChild(_hoverUpgradeBorder, 1);

    _hoverUpgradeLabel = createBaseLabel("UP", 10);
    _hoverUpgradeLabel->setName("hoverUpgradeLabel");
    _hoverUpgradeLabel->setColor(Color3B::WHITE);
    _hoverUpgradeNode->addChild(_hoverUpgradeLabel, 2);

    _hoverUpgradeButton = Button::create();
    _hoverUpgradeButton->setScale9Enabled(true);
    _hoverUpgradeButton->setContentSize(Size(kHoverUpgradeWidth, kHoverUpgradeHeight));
    _hoverUpgradeButton->setPosition(Vec2::ZERO);
    _hoverUpgradeButton->setSwallowTouches(true);
    _hoverUpgradeButton->setName("hoverUpgradeButton");
    _hoverUpgradeNode->addChild(_hoverUpgradeButton, 5);

    auto showLevelUpToast = [this]() {
        if (!_hoverInfoPanel || !_hoverUpgradeNode) {
            return;
        }

        const float toastWidth = 90.0f;
        const float toastHeight = 20.0f;
        auto toastRoot = Node::create();
        toastRoot->setCascadeOpacityEnabled(true);

        auto toastBg = LayerColor::create(Color4B(10, 10, 10, 220), toastWidth, toastHeight);
        toastBg->setAnchorPoint(Vec2(0.5f, 0.5f));
        toastBg->setIgnoreAnchorPointForPosition(false);
        toastRoot->addChild(toastBg);

        auto border = DrawNode::create();
        border->drawRect(
            Vec2(-toastWidth / 2, -toastHeight / 2),
            Vec2(toastWidth / 2, toastHeight / 2),
            Color4F(1.0f, 1.0f, 1.0f, 0.8f)
        );
        toastRoot->addChild(border, 1);

        auto toastLabel = createBaseLabel("Level up!", 12);
        toastLabel->setPosition(Vec2(0.0f, 0.0f));
        toastLabel->setColor(Color3B::WHITE);
        toastRoot->addChild(toastLabel, 2);

        Vec2 basePos = _hoverUpgradeNode->getPosition();
        toastRoot->setPosition(basePos + Vec2(0.0f, 22.0f));
        toastRoot->setOpacity(230);
        _hoverInfoPanel->addChild(toastRoot, 6);

        auto moveUp = MoveBy::create(1.2f, Vec2(0.0f, 14.0f));
        auto fadeOut = FadeOut::create(1.2f);
        toastRoot->runAction(Sequence::create(
            Spawn::create(moveUp, fadeOut, nullptr),
            RemoveSelf::create(),
            nullptr
        ));
    };

    _hoverUpgradeButton->addClickEventListener([this, showLevelUpToast](Ref*) {
        if (!_hoveredBuilding) {
            return;
        }
        bool upgraded = tryUpgradeBuilding(_hoveredBuilding);
        if (upgraded) {
            AudioManager::playButtonClick();
            showLevelUpToast();
        }
        else {
            AudioManager::playButtonCancel();
        }
        if (_uiPanel) {
            _uiPanel->updateResourceDisplay(
                Core::getInstance()->getResource(ResourceType::COIN),
                Core::getInstance()->getResource(ResourceType::DIAMOND)
            );
        }
        if (_hoveredBuilding) {
            showBuildingInfo(_hoveredBuilding);
        }
    });
}

void BaseScene::setupHoverSellUI() {
    if (!_hoverInfoPanel) {
        return;
    }

    _hoverSellNode = Node::create();
    _hoverSellNode->setName("hoverSellNode");
    _hoverSellNode->setVisible(false);
    _hoverInfoPanel->addChild(_hoverSellNode, 2);

    _hoverSellBg = LayerColor::create(Color4B(70, 45, 45, 255), kHoverSellWidth, kHoverSellHeight);
    _hoverSellBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    _hoverSellBg->setIgnoreAnchorPointForPosition(false);
    _hoverSellBg->setName("hoverSellBg");
    _hoverSellNode->addChild(_hoverSellBg);

    _hoverSellBorder = DrawNode::create();
    _hoverSellBorder->setName("hoverSellBorder");
    _hoverSellNode->addChild(_hoverSellBorder, 1);

    _hoverSellLabel = createBaseLabel("SELL", 10);
    _hoverSellLabel->setName("hoverSellLabel");
    _hoverSellLabel->setColor(Color3B::WHITE);
    _hoverSellNode->addChild(_hoverSellLabel, 2);

    _hoverSellButton = Button::create();
    _hoverSellButton->setScale9Enabled(true);
    _hoverSellButton->setContentSize(Size(kHoverSellWidth, kHoverSellHeight));
    _hoverSellButton->setPosition(Vec2::ZERO);
    _hoverSellButton->setSwallowTouches(true);
    _hoverSellButton->setName("hoverSellButton");
    _hoverSellNode->addChild(_hoverSellButton, 5);

    _hoverSellButton->addClickEventListener([this](Ref*) {
        if (!_hoveredBuilding) {
            return;
        }
        bool removed = tryDemolishBuilding(_hoveredBuilding);
        if (removed) {
            AudioManager::playButtonClick();
        }
        else {
            AudioManager::playButtonCancel();
        }
    });
}

bool BaseScene::isHoverPanelHit(const Vec2& worldPos) const {
    if (!_hoverInfoPanel || !_hoverInfoBg || !_hoverInfoPanel->isVisible()) {
        return false;
    }
    Size panelSize = _hoverInfoBg->getContentSize();
    Vec2 panelPos = _hoverInfoPanel->getPosition();
    Rect worldRect(panelPos.x, panelPos.y - panelSize.height * 0.5f, panelSize.width, panelSize.height);
    return worldRect.containsPoint(worldPos);
}

// ==================== 按钮回调 ====================

void BaseScene::onAttackClicked() {
    CCLOG("[基地场景] 点击进攻按钮，跳转到关卡选择");

    // 获取已训练的兵种
    std::map<int, int> trainedUnits;
    if (_trainPanel) {
        trainedUnits = _trainPanel->getTrainedUnits();
    }

    SaveManager::getInstance()->saveActiveSlot();

    // 跳转到关卡选择场景
    auto scene = LevelSelectScene::createScene(trainedUnits);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void BaseScene::onBuildClicked() {
    if (_buildShopPanel) {
        if (_uiPanel) {
            _uiPanel->setButtonsEnabled(false);
        }
        _buildShopPanel->show();
    }
}

void BaseScene::onExitClicked() {
    SaveManager::getInstance()->saveActiveSlot();
    auto scene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// ==================== 建筑选择与放置 ====================

void BaseScene::onBuildingSelected(const BuildingOption& option) {
    CCLOG("[基地场景] 选择建筑: %s (尺寸: %dx%d)",
        option.name.c_str(), option.gridWidth, option.gridHeight);

    // 通过放置管理器开始放置
    if (_placementManager) {
        _placementManager->startPlacement(option);
    }
}

void BaseScene::onPlacementConfirmed(const BuildingOption& option, int gridX, int gridY) {
    CCLOG("[基地场景] 确认放置建筑: %s 在位置 (%d, %d)",
        option.name.c_str(), gridX, gridY);

    // 先扣除资源，避免放置成功但资源不足
    int cost = option.cost;
    bool isSpikePlacing = (_placementManager && _placementManager->isPlacing()
        && option.type == kSpikeTrapType);
    if (!Core::getInstance()->consumeResource(ResourceType::COIN, cost)) {
        CCLOG("[基地场景] 金币不足，无法建造: %s", option.name.c_str());
        AudioManager::playButtonCancel();
        if (_uiPanel) {
            _uiPanel->updateResourceDisplay(
                Core::getInstance()->getResource(ResourceType::COIN),
                Core::getInstance()->getResource(ResourceType::DIAMOND)
            );
            if (!isSpikePlacing) {
                _uiPanel->setButtonsEnabled(true);
            }
        }
        if (isSpikePlacing && _placementManager) {
            _placementManager->cancelPlacement();
        }
        return;
    }

    // 创建实际建筑
    Node* newBuilding = createBuildingFromOption(option);

    if (newBuilding) {
        _buildingLayer->addChild(newBuilding);

        // 计算建筑位置
        Vec2 buildingPos = calculateBuildingPosition(gridX, gridY, option.gridWidth, option.gridHeight);
        newBuilding->setPosition(buildingPos);

        // 调整建筑缩放以适应格子大小
        float cellSize = _gridMap->getCellSize();
        scaleBuildingToFit(newBuilding, option.gridWidth, option.gridHeight, cellSize);

        // 标记网格为已占用
        _gridMap->occupyCell(gridX, gridY, option.gridWidth, option.gridHeight, newBuilding);
        if (auto* trap = dynamic_cast<TrapBase*>(newBuilding)) {
            trap->setGridContext(_gridMap, gridX, gridY, option.gridWidth, option.gridHeight);
        }

        // 放置完成反馈
        newBuilding->runAction(Sequence::create(
            EaseBackOut::create(ScaleTo::create(0.12f, 1.05f)),
            EaseBackOut::create(ScaleTo::create(0.12f, 1.0f)),
            nullptr));

        savePlacedBuilding(option, gridX, gridY, newBuilding);
        AudioManager::playButtonClick();

        CCLOG("[基地场景] 建筑创建成功，剩余金币: %d",
            Core::getInstance()->getResource(ResourceType::COIN));
    }
    else {
        // 建筑创建失败时返还资源
        Core::getInstance()->addResource(ResourceType::COIN, cost);
    }

    if (_uiPanel) {
        _uiPanel->updateResourceDisplay(
            Core::getInstance()->getResource(ResourceType::COIN),
            Core::getInstance()->getResource(ResourceType::DIAMOND)
        );
        if (!isSpikePlacing) {
            _uiPanel->setButtonsEnabled(true);
        }
    }
}

// ==================== 建筑创建 ====================

Node* BaseScene::createBuildingFromOption(const BuildingOption& option) {
    return buildBuildingFromOption(option, this, 0);
}

Node* BaseScene::createBuildingFromOption(const BuildingOption& option, int level) {
    return buildBuildingFromOption(option, this, level);
}

Node* BaseScene::createBuildingFromOptionForDefense(const BuildingOption& option, int level) {
    return buildBuildingFromOption(option, nullptr, level);
}

Node* BaseScene::buildBuildingFromOption(const BuildingOption& option, BaseScene* owner, int level) {
    Node* newBuilding = nullptr;
    auto* manager = BuildingManager::getInstance();

    switch (option.category) {
    case BuildingCategory::Defence:
        newBuilding = manager->createDefenceBuilding(option.configId, level);
        break;
    case BuildingCategory::Production:
        newBuilding = manager->createProductionBuilding(option.configId, level);
        break;
    case BuildingCategory::Storage:
        newBuilding = manager->createStorageBuilding(option.configId, level);
        break;
    case BuildingCategory::Trap:
        if (option.type == kSpikeTrapType) {
            newBuilding = SpikeTrap::create();
        } else if (option.type == kSnapTrapType) {
            newBuilding = SnapTrap::create();
        }
        break;
    default:
        break;
    }

    if (owner) {
        if (auto* production = dynamic_cast<ProductionBuilding*>(newBuilding)) {
            owner->setupProductionCollect(production);
        }
    }

    return newBuilding;
}

// ==================== 位置计算 ====================

Vec2 BaseScene::calculateBuildingPosition(int gridX, int gridY, int width, int height) {
    // 计算建筑中心位置：左下角格子位置 + 建筑尺寸的一半
    float cellSize = _gridMap->getCellSize();
    float centerX = (gridX + width * 0.5f) * cellSize;
    float centerY = (gridY + height * 0.5f) * cellSize;
    return Vec2(centerX, centerY);
}

// ==================== 建筑状态保存/恢复 ====================

void BaseScene::savePlacedBuilding(const BuildingOption& option, int gridX, int gridY, Node* building) {
    for (size_t i = 0; i < s_savedBuildings.size(); ++i) {
        const auto& saved = s_savedBuildings[i];
        if (saved.gridX == gridX && saved.gridY == gridY && saved.option.type == option.type) {
            if (building) {
                building->setTag(static_cast<int>(kSavedBuildingTagBase + i));
            }
            return;
        }
    }

    BaseSavedBuilding saved;
    saved.option = option;
    saved.option.canBuild = true;
    saved.gridX = gridX;
    saved.gridY = gridY;
    saved.level = 0;
    s_savedBuildings.push_back(saved);
    if (building) {
        building->setTag(static_cast<int>(kSavedBuildingTagBase + s_savedBuildings.size() - 1));
    }
}

void BaseScene::restoreSavedBuildings() {
    if (!_gridMap || !_buildingLayer) {
        return;
    }

    float cellSize = _gridMap->getCellSize();
    for (size_t i = 0; i < s_savedBuildings.size(); ++i) {
        const auto& saved = s_savedBuildings[i];
        const auto& option = saved.option;
        if (!_gridMap->canPlaceBuilding(saved.gridX, saved.gridY, option.gridWidth, option.gridHeight)) {
            continue;
        }

        Node* building = createBuildingFromOption(option, saved.level);
        if (!building) {
            continue;
        }

        _buildingLayer->addChild(building);
        building->setTag(static_cast<int>(kSavedBuildingTagBase + i));
        Vec2 buildingPos = calculateBuildingPosition(saved.gridX, saved.gridY, option.gridWidth, option.gridHeight);
        building->setPosition(buildingPos);
        scaleBuildingToFit(building, option.gridWidth, option.gridHeight, cellSize);
        _gridMap->occupyCell(saved.gridX, saved.gridY, option.gridWidth, option.gridHeight, building);
        if (auto* trap = dynamic_cast<TrapBase*>(building)) {
            trap->setGridContext(_gridMap, saved.gridX, saved.gridY, option.gridWidth, option.gridHeight);
        }
    }
}

void BaseScene::setupProductionCollect(ProductionBuilding* building) {
    if (!building) {
        return;
    }
    int id = building->getId();
    const std::string& name = building->getName();
    bool isCollector = (id == 3003 || id == 3004 || name == "GoldMaker" || name == "DiamondMaker");
    if (!isCollector) {
        return;
    }

    building->setCollectCallback([this](ProductionBuilding*, ResourceType type, int amount, const Vec2& worldPos) {
        Core::getInstance()->addResource(type, amount);
        if (_uiPanel) {
            _uiPanel->updateResourceDisplay(
                Core::getInstance()->getResource(ResourceType::COIN),
                Core::getInstance()->getResource(ResourceType::DIAMOND)
            );
        }
        playCollectEffect(type, amount, worldPos);
    });
}

void BaseScene::playCollectEffect(ResourceType type, int amount, const Vec2& worldPos) {
    if (!_effectLayer) {
        return;
    }

    auto sprite = Core::getInstance()->createResourceSprite(type);
    if (sprite) {
        sprite->setPosition(worldPos);
        sprite->setScale(0.75f);
        _effectLayer->addChild(sprite, 5);

        Vec2 targetPos = _uiPanel ? _uiPanel->getResourceIconWorldPosition(type) : Vec2::ZERO;
        if (targetPos == Vec2::ZERO) {
            targetPos = worldPos + Vec2(0.0f, 120.0f);
        }
        auto move = EaseSineIn::create(MoveTo::create(0.6f, targetPos));
        auto fade = FadeOut::create(0.6f);
        sprite->runAction(Sequence::create(Spawn::create(move, fade, nullptr), RemoveSelf::create(), nullptr));
    }

    std::string text = (type == ResourceType::COIN)
        ? StringUtils::format("Gold +%d", amount)
        : StringUtils::format("Diamond +%d", amount);

    auto label = createBaseLabel(text, 16);
    label->setPosition(worldPos + Vec2(0.0f, 12.0f));
    label->setColor(type == ResourceType::COIN ? Color3B(255, 220, 90) : Color3B(130, 210, 255));
    _effectLayer->addChild(label, 6);

    auto rise = MoveBy::create(0.7f, Vec2(0.0f, 26.0f));
    auto fade = FadeOut::create(0.7f);
    label->runAction(Sequence::create(Spawn::create(rise, fade, nullptr), RemoveSelf::create(), nullptr));
}

// ==================== 触摸事件处理 ====================

bool BaseScene::onTouchBegan(Touch* touch, Event* event) {
    // 如果训练面板正在显示，不处理触摸
    if (_trainPanel && _trainPanel->isShowing()) {
        return false;
    }

    // 如果建筑商店正在显示，不处理触摸
    if (_buildShopPanel && _buildShopPanel->isShowing()) {
        return false;
    }

    // 如果处于放置模式，由放置管理器处理
    if (_placementManager && _placementManager->isPlacing()) {
        return true;
    }

    // 检查是否点击了兵营建筑
    Vec2 touchPos = touch->getLocation();
    Vec2 localInLayer = _buildingLayer->convertToNodeSpace(touchPos);

    auto& buildings = _buildingLayer->getChildren();
    for (auto& child : buildings) {
        auto building = dynamic_cast<ProductionBuilding*>(child);
        if (!building) continue;

        // 使用建筑子精灵包围盒做点击检测，避免Node尺寸为0导致失效
        bool isHit = false;
        Sprite* bodySprite = nullptr;
        for (auto& bChild : building->getChildren()) {
            bodySprite = dynamic_cast<Sprite*>(bChild);
            if (bodySprite) break;
        }

        if (bodySprite) {
            // 使用世界坐标转到建筑本地坐标，避免坐标系不一致导致点击失效
            Vec2 localInBuilding = building->convertToNodeSpace(touchPos);
            Rect spriteBox = bodySprite->getBoundingBox();
            isHit = spriteBox.containsPoint(localInBuilding);
        }
        else {
            isHit = building->getBoundingBox().containsPoint(localInLayer);
        }

        if (isHit) {
            // 只有兵营（SoldierBuilder, ID=3002）才打开训练面板
            if (building->getId() == 3002 || building->getName() == "SoldierBuilder") {
                CCLOG("[基地场景] 点击兵营，打开训练面板");
                showTrainPanel();
                return true;
            }
            else {
                CCLOG("[基地场景] 点击建筑: %s (ID: %d)",
                    building->getName().c_str(), building->getId());
            }
        }
    }

    return false;
}

void BaseScene::onTouchMoved(Touch* touch, Event* event) {
    // 如果处于放置模式，更新预览位置
    if (_placementManager && _placementManager->isPlacing()) {
        Vec2 touchPos = touch->getLocation();
        Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
        _placementManager->updatePreviewPosition(localPos);
        if (_placementManager->getState().buildingType == kSpikeTrapType) {
            _placementManager->tryPaintPlacement(localPos);
        }
    }
}

void BaseScene::onTouchEnded(Touch* touch, Event* event) {
    // 如果处于放置模式，尝试确认放置
    if (_placementManager && _placementManager->isPlacing()) {
        Vec2 touchPos = touch->getLocation();
        Vec2 localPos = _gridMap->convertToNodeSpace(touchPos);
        if (_placementManager->getState().buildingType == kSpikeTrapType) {
            _placementManager->tryPaintPlacement(localPos);
            _placementManager->cancelPlacement();
            return;
        }

        // 尝试确认放置，如果失败则取消
        if (!_placementManager->tryConfirmPlacement(localPos)) {
            _placementManager->cancelPlacement();
        }
    }
}

// ==================== 悬浮信息处理 ====================

void BaseScene::updateHoverInfo(const Vec2& worldPos) {
    if (!_gridMap || !_buildingLayer) {
        return;
    }
    if (_trainPanel && _trainPanel->isShowing()) {
        clearBuildingInfo();
        return;
    }
    if (_buildShopPanel && _buildShopPanel->isShowing()) {
        clearBuildingInfo();
        return;
    }
    if (_placementManager && _placementManager->isPlacing()) {
        clearBuildingInfo();
        return;
    }

    if (_hoverInfoPanel && _hoverInfoPanel->isVisible() && isHoverPanelHit(worldPos)) {
        return;
    }

    Node* building = pickBuildingAt(worldPos);
    if (!building) {
        if (_hoverInfoPanel && _hoverInfoBg && _hoverInfoPanel->isVisible() && _hoveredBuilding) {
            Rect keepRect = getHoverKeepRect(_hoveredBuilding, _hoverInfoPanel, _hoverInfoBg);
            if (keepRect.containsPoint(worldPos)) {
                return;
            }
        }
        clearBuildingInfo();
        return;
    }

    if (building != _hoveredBuilding) {
        _hoveredBuilding = building;
    }
    showBuildingInfo(building);
    Vec2 anchorPos = getHoverAnchorWorldPos(building);
    if (anchorPos == Vec2::ZERO) {
        anchorPos = worldPos;
    }
    updateHoverPanelPosition(anchorPos);
}

Node* BaseScene::pickBuildingAt(const Vec2& worldPos) const {
    if (!_buildingLayer) {
        return nullptr;
    }

    for (auto* child : _buildingLayer->getChildren()) {
        if (!child) {
            continue;
        }
        if (dynamic_cast<DefenceBuilding*>(child)
            || dynamic_cast<ProductionBuilding*>(child)
            || dynamic_cast<StorageBuilding*>(child)
            || dynamic_cast<TrapBase*>(child)) {
            if (NodeUtils::hitTestBuilding(child, worldPos)) {
                return child;
            }
        }
    }
    return nullptr;
}

void BaseScene::showBuildingInfo(Node* building) {
    if (!_hoverInfoPanel || !_hoverInfoLabel) {
        return;
    }

    std::string name = "Unknown";
    int level = 0;
    float hp = 0.0f;
    float maxHp = 0.0f;
    float atk = 0.0f;
    float range = 0.0f;
    int sizeW = 0;
    int sizeH = 0;
    int produceGold = 0;
    int produceElixir = 0;
    bool hasAttack = false;
    std::string upgradeText = "-";

    if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
        name = defence->getName();
        level = defence->getLevel() + 1;
        hp = defence->getCurrentHP();
        maxHp = defence->getCurrentMaxHP();
        atk = defence->getCurrentATK();
        range = defence->getCurrentATK_RANGE();
        sizeW = defence->getWidth();
        sizeH = defence->getLength();
        hasAttack = true;
    }
    else if (auto* production = dynamic_cast<ProductionBuilding*>(building)) {
        name = production->getName();
        level = production->getLevel() + 1;
        hp = production->getCurrentHP();
        maxHp = production->getCurrentMaxHP();
        sizeW = production->getWidth();
        sizeH = production->getLength();
        produceGold = static_cast<int>(production->getCurrentPRODUCE_GOLD());
        produceElixir = static_cast<int>(production->getCurrentPRODUCE_ELIXIR());
    }
    else if (auto* storage = dynamic_cast<StorageBuilding*>(building)) {
        name = storage->getName();
        level = storage->getLevel() + 1;
        hp = storage->getCurrentHP();
        maxHp = storage->getCurrentMaxHP();
        sizeW = storage->getWidth();
        sizeH = storage->getLength();
    }
    else if (dynamic_cast<TrapBase*>(building)) {
        int savedIndex = getSavedBuildingIndex(building);
        if (savedIndex >= 0 && savedIndex < static_cast<int>(s_savedBuildings.size())) {
            const auto& saved = s_savedBuildings[static_cast<size_t>(savedIndex)];
            name = saved.option.name;
            level = 1;
            sizeW = saved.option.gridWidth;
            sizeH = saved.option.gridHeight;
        }
    }

    std::string attackText = hasAttack ? StringUtils::format("%.0f", atk) : "-";
    std::string rangeText = hasAttack ? StringUtils::format("%.0f", range) : "-";
    std::string produceText = "-";
    if (produceGold > 0 || produceElixir > 0) {
        produceText = StringUtils::format("Gold+%d Elixir+%d", produceGold, produceElixir);
    }

    int upgradeLevel = 0;
    int upgradeMax = 0;
    int upgradeCost = 0;
    ResourceType upgradeResource = ResourceType::COIN;
    bool upgradeIsBase = false;
    if (getUpgradeInfo(building, upgradeLevel, upgradeMax, upgradeCost, upgradeResource, upgradeIsBase)) {
        if (upgradeLevel >= upgradeMax || upgradeCost <= 0) {
            upgradeText = "MAX";
        }
        else {
            const char* suffix = (upgradeResource == ResourceType::DIAMOND) ? "D" : "G";
            upgradeText = StringUtils::format("%d%s", upgradeCost, suffix);
        }
    }

    char infoText[320];
    snprintf(infoText, sizeof(infoText),
        "Name: %s\nLevel: %d\nHP: %.0f / %.0f\nATK: %s\nRange: %s\nProduction: %s\nUpgrade: %s\nFootprint: %dx%d",
        name.c_str(),
        level,
        hp,
        maxHp,
        attackText.c_str(),
        rangeText.c_str(),
        produceText.c_str(),
        upgradeText.c_str(),
        sizeW,
        sizeH
    );
    _hoverInfoLabel->setString(infoText);

    Size textSize = _hoverInfoLabel->getContentSize();
    float panelWidth = std::max(kHoverPanelMinWidth, textSize.width + kHoverPanelPaddingX * 2);
    float panelHeight = std::max(kHoverPanelMinHeight, textSize.height + kHoverPanelPaddingY * 2);
    _hoverInfoBg->setContentSize(Size(panelWidth, panelHeight));
    _hoverInfoLabel->setPosition(Vec2(kHoverPanelPaddingX, panelHeight - kHoverPanelPaddingY));
    _hoverInfoPanel->setContentSize(Size(panelWidth, panelHeight));

    float upgradeX = panelWidth - kHoverUpgradeWidth * 0.5f - 8.0f;
    float upgradeY = kHoverUpgradeHeight * 0.5f + 8.0f;
    if (_hoverUpgradeNode) {
        _hoverUpgradeNode->setPosition(Vec2(upgradeX, upgradeY));
    }
    if (_hoverSellNode) {
        float sellX = upgradeX - (kHoverUpgradeWidth * 0.5f + kHoverSellWidth * 0.5f + kHoverSellGap);
        _hoverSellNode->setPosition(Vec2(sellX, upgradeY));
    }

    auto border = dynamic_cast<DrawNode*>(_hoverInfoPanel->getChildByName("hoverBorder"));
    if (border) {
        border->clear();
        border->drawRect(Vec2(0, 0), Vec2(panelWidth, panelHeight), Color4F(0.8f, 0.8f, 0.8f, 1.0f));
    }

    _hoverInfoPanel->setVisible(true);
    updateHoverOverlays(building);
    updateUpgradeUI(building);
    updateSellUI(building);
}

void BaseScene::updateHoverOverlays(Node* building) {
    if (!_hoverFootprintNode || !_hoverRangeNode || !_gridMap || !building) {
        return;
    }

    _hoverFootprintNode->clear();
    _hoverRangeNode->clear();

    float cellSize = _gridMap->getCellSize();
    int sizeW = 0;
    int sizeH = 0;
    float range = 0.0f;

    if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
        sizeW = defence->getWidth();
        sizeH = defence->getLength();
        range = defence->getCurrentATK_RANGE();
    }
    else if (auto* production = dynamic_cast<ProductionBuilding*>(building)) {
        sizeW = production->getWidth();
        sizeH = production->getLength();
    }
    else if (auto* storage = dynamic_cast<StorageBuilding*>(building)) {
        sizeW = storage->getWidth();
        sizeH = storage->getLength();
    }

    Vec2 center = building->getPosition();
    int gridX = static_cast<int>(std::floor(center.x / cellSize - sizeW * 0.5f + 0.001f));
    int gridY = static_cast<int>(std::floor(center.y / cellSize - sizeH * 0.5f + 0.001f));

    Vec2 bottomLeft(gridX * cellSize, gridY * cellSize);
    Vec2 topRight((gridX + sizeW) * cellSize, (gridY + sizeH) * cellSize);
    _hoverFootprintNode->drawRect(bottomLeft, topRight, Color4F(0.9f, 0.9f, 0.9f, 1.0f));
    _hoverFootprintNode->setVisible(true);

    if (range > 0.0f) {
        NodeUtils::drawDashedCircle(_hoverRangeNode, center, range, Color4F(0.85f, 0.85f, 0.85f, 0.8f));
        _hoverRangeNode->setVisible(true);
    }
    else {
        _hoverRangeNode->setVisible(false);
    }
}

void BaseScene::updateHoverPanelPosition(const Vec2& worldPos) {
    if (!_hoverInfoPanel || !_hoverInfoBg) {
        return;
    }
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    Size panelSize = _hoverInfoBg->getContentSize();
    Vec2 desiredPos(
        worldPos.x + kHoverPanelOffsetX,
        worldPos.y
    );

    float maxX = origin.x + visibleSize.width - panelSize.width - 6.0f;
    float minX = origin.x + 6.0f;
    float maxY = origin.y + visibleSize.height - panelSize.height * 0.5f - 6.0f;
    float minY = origin.y + panelSize.height * 0.5f + 6.0f;

    float clampedX = std::min(std::max(desiredPos.x, minX), maxX);
    float clampedY = std::min(std::max(desiredPos.y, minY), maxY);

    _hoverInfoPanel->setPosition(Vec2(clampedX, clampedY));
}

void BaseScene::clearBuildingInfo() {
    _hoveredBuilding = nullptr;
    if (_hoverInfoPanel) {
        _hoverInfoPanel->setVisible(false);
    }
    if (_hoverUpgradeNode) {
        _hoverUpgradeNode->setVisible(false);
    }
    if (_hoverSellNode) {
        _hoverSellNode->setVisible(false);
    }
    if (_hoverRangeNode) {
        _hoverRangeNode->clear();
        _hoverRangeNode->setVisible(false);
    }
    if (_hoverFootprintNode) {
        _hoverFootprintNode->clear();
        _hoverFootprintNode->setVisible(false);
    }
}

int BaseScene::getSavedBuildingIndex(Node* building) const {
    if (!building) {
        return -1;
    }
    int tag = building->getTag();
    int index = tag - kSavedBuildingTagBase;
    if (index < 0 || index >= static_cast<int>(s_savedBuildings.size())) {
        return -1;
    }
    return index;
}

bool BaseScene::canDemolishBuilding(Node* building, int& savedIndex) const {
    savedIndex = getSavedBuildingIndex(building);
    if (savedIndex < 0) {
        return false;
    }
    if (savedIndex >= static_cast<int>(s_savedBuildings.size())) {
        return false;
    }
    return true;
}

void BaseScene::removeSavedBuildingAt(int index) {
    if (index < 0 || index >= static_cast<int>(s_savedBuildings.size())) {
        return;
    }
    s_savedBuildings.erase(s_savedBuildings.begin() + index);
    if (!_buildingLayer) {
        return;
    }

    int tagThreshold = kSavedBuildingTagBase + index;
    for (auto* child : _buildingLayer->getChildren()) {
        if (!child) {
            continue;
        }
        int tag = child->getTag();
        if (tag > tagThreshold) {
            child->setTag(tag - 1);
        }
    }
}

bool BaseScene::getUpgradeInfo(Node* building,
                               int& currentLevel,
                               int& maxLevel,
                               int& cost,
                               ResourceType& resource,
                               bool& isBase) const {
    currentLevel = 0;
    maxLevel = 0;
    cost = 0;
    resource = ResourceType::COIN;
    isBase = false;

    if (!building) {
        return false;
    }

    if (auto* production = dynamic_cast<ProductionBuilding*>(building)) {
        int id = production->getId();
        const std::string& name = production->getName();
        if (id == 3001 || name == "Base") {
            isBase = true;
            int baseLevel = Core::getInstance()->getBaseLevel() - 1;
            if (baseLevel < 0) {
                baseLevel = 0;
            }
            int baseMax = Core::getInstance()->getBaseMaxLevel() - 1;
            if (baseMax < 0) {
                baseMax = 0;
            }
            maxLevel = std::min(baseMax, production->getMaxLevel());
            if (baseLevel > maxLevel) {
                baseLevel = maxLevel;
            }
            currentLevel = baseLevel;
            cost = Core::getInstance()->getBaseUpgradeCost();
            resource = ResourceType::DIAMOND;
            return true;
        }
    }

    int width = 0;
    int height = 0;
    if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
        currentLevel = defence->getLevel();
        maxLevel = defence->getMaxLevel();
        width = defence->getWidth();
        height = defence->getLength();
    }
    else if (auto* production = dynamic_cast<ProductionBuilding*>(building)) {
        currentLevel = production->getLevel();
        maxLevel = production->getMaxLevel();
        width = production->getWidth();
        height = production->getLength();
    }
    else if (auto* storage = dynamic_cast<StorageBuilding*>(building)) {
        currentLevel = storage->getLevel();
        maxLevel = storage->getMaxLevel();
        width = storage->getWidth();
        height = storage->getLength();
    }
    else {
        return false;
    }

    int baseCost = 0;
    int savedIndex = getSavedBuildingIndex(building);
    if (savedIndex >= 0) {
        baseCost = s_savedBuildings[static_cast<size_t>(savedIndex)].option.cost;
    }
    if (baseCost <= 0) {
        int area = std::max(1, width * height);
        baseCost = area * 20;
        if (baseCost < 50) {
            baseCost = 50;
        }
    }
    cost = baseCost * (currentLevel + 1);
    cost = static_cast<int>(std::round(cost * kUpgradeCostFactor));
    if (cost < 1) {
        cost = 1;
    }
    resource = ResourceType::COIN;
    return true;
}

void BaseScene::updateUpgradeUI(Node* building) {
    if (!_hoverUpgradeNode || !_hoverUpgradeLabel || !_hoverUpgradeBg || !_hoverUpgradeBorder || !_hoverUpgradeButton) {
        return;
    }

    int currentLevel = 0;
    int maxLevel = 0;
    int cost = 0;
    ResourceType resource = ResourceType::COIN;
    bool isBase = false;
    if (!getUpgradeInfo(building, currentLevel, maxLevel, cost, resource, isBase)) {
        _hoverUpgradeNode->setVisible(false);
        return;
    }

    bool isMax = (currentLevel >= maxLevel) || (cost <= 0);
    int available = Core::getInstance()->getResource(resource);
    bool canUpgrade = (!isMax && available >= cost);

    _hoverUpgradeNode->setVisible(true);
    if (_hoverUpgradeLabel) {
        if (isMax) {
            _hoverUpgradeLabel->setString("MAX");
        }
        else {
            char buffer[32];
            const char* suffix = (resource == ResourceType::DIAMOND) ? "D" : "G";
            snprintf(buffer, sizeof(buffer), "UP %d%s", cost, suffix);
            _hoverUpgradeLabel->setString(buffer);
        }
        _hoverUpgradeLabel->setColor(canUpgrade ? Color3B::WHITE : Color3B(170, 170, 170));
    }

    if (_hoverUpgradeBg) {
        _hoverUpgradeBg->setColor(canUpgrade ? Color3B(80, 70, 40) : Color3B(70, 70, 70));
    }
    if (_hoverUpgradeBorder) {
        _hoverUpgradeBorder->clear();
        Color4F borderColor = canUpgrade ? Color4F::WHITE : Color4F(0.4f, 0.4f, 0.4f, 1.0f);
        _hoverUpgradeBorder->drawRect(
            Vec2(-kHoverUpgradeWidth / 2, -kHoverUpgradeHeight / 2),
            Vec2(kHoverUpgradeWidth / 2, kHoverUpgradeHeight / 2),
            borderColor
        );
    }
    if (_hoverUpgradeButton) {
        _hoverUpgradeButton->setEnabled(canUpgrade);
        _hoverUpgradeButton->setBright(canUpgrade);
    }
}

void BaseScene::updateSellUI(Node* building) {
    if (!_hoverSellNode || !_hoverSellLabel || !_hoverSellBg || !_hoverSellBorder || !_hoverSellButton) {
        return;
    }

    int savedIndex = -1;
    if (!canDemolishBuilding(building, savedIndex)) {
        _hoverSellNode->setVisible(false);
        return;
    }

    int refund = 0;
    if (savedIndex >= 0 && savedIndex < static_cast<int>(s_savedBuildings.size())) {
        refund = static_cast<int>(std::round(s_savedBuildings[static_cast<size_t>(savedIndex)].option.cost * kSellRefundRate));
    }
    if (refund < 0) {
        refund = 0;
    }

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "SELL %dG", refund);
    _hoverSellLabel->setString(buffer);
    _hoverSellLabel->setColor(Color3B::WHITE);

    _hoverSellBg->setColor(Color3B(70, 45, 45));
    _hoverSellBorder->clear();
    _hoverSellBorder->drawRect(
        Vec2(-kHoverSellWidth / 2, -kHoverSellHeight / 2),
        Vec2(kHoverSellWidth / 2, kHoverSellHeight / 2),
        Color4F(0.9f, 0.9f, 0.9f, 1.0f)
    );
    _hoverSellButton->setEnabled(true);
    _hoverSellButton->setBright(true);
    _hoverSellNode->setVisible(true);
}

bool BaseScene::tryUpgradeBuilding(Node* building) {
    if (!building) {
        return false;
    }

    int currentLevel = 0;
    int maxLevel = 0;
    int cost = 0;
    ResourceType resource = ResourceType::COIN;
    bool isBase = false;
    if (!getUpgradeInfo(building, currentLevel, maxLevel, cost, resource, isBase)) {
        return false;
    }

    if (currentLevel >= maxLevel || cost <= 0) {
        return false;
    }

    if (isBase) {
        if (!Core::getInstance()->upgradeBase()) {
            return false;
        }
        syncBaseBuildingLevel();
        return true;
    }

    if (!Core::getInstance()->consumeResource(resource, cost)) {
        return false;
    }

    int nextLevel = currentLevel + 1;
    if (auto* defence = dynamic_cast<DefenceBuilding*>(building)) {
        defence->setLevel(nextLevel);
    }
    else if (auto* production = dynamic_cast<ProductionBuilding*>(building)) {
        production->setLevel(nextLevel);
        int id = production->getId();
        const std::string& name = production->getName();
        if (id == 3002 || name == "SoldierBuilder") {
            s_barracksLevel = nextLevel;
        }
    }
    else if (auto* storage = dynamic_cast<StorageBuilding*>(building)) {
        storage->setLevel(nextLevel);
    }
    else {
        return false;
    }

    int savedIndex = getSavedBuildingIndex(building);
    if (savedIndex >= 0) {
        s_savedBuildings[static_cast<size_t>(savedIndex)].level = nextLevel;
    }

    auto* bodySprite = NodeUtils::findBodySprite(building);
    if (bodySprite) {
        constexpr int kUpgradeEffectTag = 21001;
        bodySprite->stopActionByTag(kUpgradeEffectTag);
        float baseScaleX = bodySprite->getScaleX();
        float baseScaleY = bodySprite->getScaleY();
        auto scaleUp = ScaleTo::create(0.08f, baseScaleX * 1.05f, baseScaleY * 1.05f);
        auto scaleDown = ScaleTo::create(0.12f, baseScaleX, baseScaleY);
        auto seq = Sequence::create(scaleUp, scaleDown, nullptr);
        seq->setTag(kUpgradeEffectTag);
        bodySprite->runAction(seq);
    }

    return true;
}

bool BaseScene::tryDemolishBuilding(Node* building) {
    if (!building || !_gridMap || !_buildingLayer) {
        return false;
    }

    int savedIndex = -1;
    if (!canDemolishBuilding(building, savedIndex)) {
        return false;
    }

    const auto& saved = s_savedBuildings[static_cast<size_t>(savedIndex)];
    const auto& option = saved.option;
    int refund = static_cast<int>(std::round(option.cost * kSellRefundRate));
    if (refund < 0) {
        refund = 0;
    }

    _gridMap->freeCell(saved.gridX, saved.gridY, option.gridWidth, option.gridHeight);
    building->removeFromParent();
    removeSavedBuildingAt(savedIndex);

    Core::getInstance()->addResource(ResourceType::COIN, refund);
    if (_uiPanel) {
        _uiPanel->updateResourceDisplay(
            Core::getInstance()->getResource(ResourceType::COIN),
            Core::getInstance()->getResource(ResourceType::DIAMOND)
        );
    }

    _hoveredBuilding = nullptr;
    clearBuildingInfo();
    return true;
}

void BaseScene::syncBaseBuildingLevel() {
    if (!_buildingLayer) {
        return;
    }

    int baseLevel = Core::getInstance()->getBaseLevel() - 1;
    if (baseLevel < 0) {
        baseLevel = 0;
    }

    for (auto* child : _buildingLayer->getChildren()) {
        auto* production = dynamic_cast<ProductionBuilding*>(child);
        if (!production) {
            continue;
        }
        int id = production->getId();
        const std::string& name = production->getName();
        if (id != 3001 && name != "Base") {
            continue;
        }

        int maxLevel = production->getMaxLevel();
        if (baseLevel > maxLevel) {
            baseLevel = maxLevel;
        }
        if (production->getLevel() != baseLevel) {
            production->setLevel(baseLevel);
        }
        break;
    }
}
