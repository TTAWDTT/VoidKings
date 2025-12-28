#ifndef __REPLAY_MANAGER_H__
#define __REPLAY_MANAGER_H__

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct ReplayDeployEvent {
    float time = 0.0f;
    int unitId = 0;
    int gridX = 0;
    int gridY = 0;
    int level = 0;
};

struct BattleReplay {
    int version = 1;
    int levelId = 1;
    bool defenseMode = false;
    bool allowDefaultUnits = true;
    float battleSpeed = 1.0f;
    int64_t timestamp = 0;
    bool resultWin = false;
    int resultStars = 0;
    float duration = 0.0f;
    std::map<int, int> deployableUnits;
    std::vector<ReplayDeployEvent> events;
};

class ReplayManager {
public:
    static ReplayManager* getInstance();

    void setLastReplay(const BattleReplay& replay);
    bool hasLastReplay();
    const BattleReplay* getLastReplay();
    bool saveLastReplay();
    bool loadLastReplay();
    std::string getLastReplayPath() const;
    bool exportReplayTo(const std::string& path, const BattleReplay& replay);
    bool importReplayFrom(const std::string& path, BattleReplay* outReplay) const;

private:
    ReplayManager() = default;
    ~ReplayManager() = default;

    std::string buildReplayPath() const;

    BattleReplay _lastReplay;
    bool _hasLastReplay = false;

    static ReplayManager* s_instance;
};

#endif // __REPLAY_MANAGER_H__
