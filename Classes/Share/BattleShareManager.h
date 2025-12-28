#ifndef __BATTLE_SHARE_MANAGER_H__
#define __BATTLE_SHARE_MANAGER_H__

#include "cocos2d.h"
#include "Scenes/BaseScene.h"
#include "Replay/ReplayManager.h"

/**
 * @brief 异步攻防-基地快照
 *
 * 记录玩家当前基地的核心数据，便于上传给服务器或分享给其他玩家。
 */
struct BaseSnapshot {
    int version = 1;
    std::string configHash;
    int baseLevel = 0;
    int barracksLevel = 0;
    cocos2d::Vec2 baseAnchor;
    cocos2d::Vec2 barracksAnchor;
    std::vector<BaseSavedBuilding> buildings;
};

/**
 * @brief 异步攻防分享管理器
 *
 * 负责导出/导入基地快照以及回放文件，并提供默认的共享目录。
 */
class BattleShareManager {
public:
    static BattleShareManager* getInstance();

    BaseSnapshot captureCurrentBase() const;
    bool exportPlayerBaseSnapshot(std::string* outPath = nullptr);
    bool loadIncomingSnapshot(BaseSnapshot* outSnapshot);
    bool saveSnapshotToPath(const BaseSnapshot& snapshot, const std::string& path);
    bool loadSnapshotFromPath(const std::string& path, BaseSnapshot* outSnapshot);

    void setActiveTargetSnapshot(const BaseSnapshot& snapshot);
    bool hasActiveTargetSnapshot() const;
    const BaseSnapshot* getActiveTargetSnapshot() const;
    void clearActiveTargetSnapshot();

    bool exportLastReplay(std::string* outPath = nullptr);
    bool loadIncomingReplay(BattleReplay* outReplay) const;

    bool hasIncomingSnapshot() const;
    bool hasIncomingReplay() const;

    std::string getShareDirectory() const;
    std::string getOutgoingSnapshotPath() const;
    std::string getIncomingSnapshotPath() const;
    std::string getOutgoingReplayPath() const;
    std::string getIncomingReplayPath() const;

private:
    BattleShareManager() = default;
    ~BattleShareManager() = default;

    std::string buildSharePath(const std::string& filename) const;
    void ensureShareDirectory() const;
    std::string computeConfigHash() const;
    std::string categoryToString(BuildingCategory category) const;
    BuildingCategory stringToCategory(const std::string& str) const;

    BaseSnapshot _targetSnapshot;
    bool _hasTargetSnapshot = false;

    static BattleShareManager* s_instance;
};

#endif // __BATTLE_SHARE_MANAGER_H__
