#ifndef __SAVE_MANAGER_H__
#define __SAVE_MANAGER_H__

#include <string>
#include <vector>

struct SaveSlotInfo {
    int slot = 0;
    bool exists = false;
    std::string summary;
};

class SaveManager {
public:
    static SaveManager* getInstance();

    void init();
    bool saveSlot(int slot);
    bool saveActiveSlot();
    bool loadSlot(int slot);
    bool deleteSlot(int slot);
    bool hasSlot(int slot) const;

    void setActiveSlot(int slot);
    int getActiveSlot() const;

    SaveSlotInfo getSlotInfo(int slot) const;
    std::vector<SaveSlotInfo> listSlots() const;

private:
    SaveManager() = default;
    ~SaveManager() = default;

    bool isValidSlot(int slot) const;
    std::string getSlotKey(int slot) const;
    void resetGameState() const;
    std::string buildSummary() const;

    bool _initialized = false;
    int _activeSlot = 0;

    static SaveManager* s_instance;
};

#endif // __SAVE_MANAGER_H__
