#include "Save/SaveManager.h"
#include "Buildings/BuildingCatalog.h"
#include "Core/Core.h"
#include "Scenes/BaseScene.h"
#include "Soldier/UnitManager.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include "storage/local-storage/LocalStorage.h"
#include "cocos2d.h"
#include <cmath>
#include <cstdint>
#include <ctime>
#include <map>

using namespace cocos2d;

namespace {
constexpr int kSlotCount = 6;
constexpr int kSaveVersion = 1;
const char* kSlotKeyPrefix = "vk_save_slot_";

std::string categoryToString(BuildingCategory category) {
    switch (category) {
        case BuildingCategory::Defence:
            return "defence";
        case BuildingCategory::Production:
            return "production";
        case BuildingCategory::Storage:
            return "storage";
        case BuildingCategory::Trap:
            return "trap";
        default:
            return "unknown";
    }
}

BuildingCategory stringToCategory(const std::string& raw) {
    if (raw == "defence" || raw == "defense") {
        return BuildingCategory::Defence;
    }
    if (raw == "production") {
        return BuildingCategory::Production;
    }
    if (raw == "storage") {
        return BuildingCategory::Storage;
    }
    if (raw == "trap") {
        return BuildingCategory::Trap;
    }
    return BuildingCategory::Unknown;
}

int readInt(const rapidjson::Value& obj, const char* key, int fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) {
        return fallback;
    }
    if (it->value.IsInt()) {
        return it->value.GetInt();
    }
    if (it->value.IsNumber()) {
        return static_cast<int>(it->value.GetDouble());
    }
    return fallback;
}

long long readInt64(const rapidjson::Value& obj, const char* key, long long fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) {
        return fallback;
    }
    if (it->value.IsInt64()) {
        return it->value.GetInt64();
    }
    if (it->value.IsInt()) {
        return it->value.GetInt();
    }
    if (it->value.IsNumber()) {
        return static_cast<long long>(it->value.GetDouble());
    }
    return fallback;
}

bool readBool(const rapidjson::Value& obj, const char* key, bool fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) {
        return fallback;
    }
    if (it->value.IsBool()) {
        return it->value.GetBool();
    }
    return fallback;
}

std::string readString(const rapidjson::Value& obj, const char* key, const std::string& fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) {
        return fallback;
    }
    if (it->value.IsString()) {
        return it->value.GetString();
    }
    return fallback;
}

std::string makeSlotKey(int slot) {
    return StringUtils::format("%s%d", kSlotKeyPrefix, slot);
}
} // namespace

SaveManager* SaveManager::s_instance = nullptr;

SaveManager* SaveManager::getInstance() {
    if (!s_instance) {
        s_instance = new SaveManager();
        s_instance->init();
    }
    return s_instance;
}

void SaveManager::init() {
    if (_initialized) {
        return;
    }
    std::string dbPath = FileUtils::getInstance()->getWritablePath() + "voidkings_saves.db";
    localStorageInit(dbPath);
    _initialized = true;
}

bool SaveManager::isValidSlot(int slot) const {
    return slot >= 1 && slot <= kSlotCount;
}

std::string SaveManager::getSlotKey(int slot) const {
    return makeSlotKey(slot);
}

void SaveManager::resetGameState() const {
    Core::getInstance()->resetState();
    UnitManager::getInstance()->resetState();
    BaseScene::resetSavedState();
}

std::string SaveManager::buildSummary() const {
    int gold = Core::getInstance()->getResource(ResourceType::COIN);
    int diamond = Core::getInstance()->getResource(ResourceType::DIAMOND);
    int baseLevel = Core::getInstance()->getBaseLevel();
    int buildingCount = static_cast<int>(BaseScene::getSavedBuildings().size());
    return StringUtils::format("Lv%d G%d D%d B%d", baseLevel, gold, diamond, buildingCount);
}

bool SaveManager::saveSlot(int slot) {
    if (!isValidSlot(slot)) {
        return false;
    }
    init();

    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    doc.AddMember("version", kSaveVersion, alloc);

    rapidjson::Value meta(rapidjson::kObjectType);
    meta.AddMember("timestamp", static_cast<int64_t>(std::time(nullptr)), alloc);
    std::string summary = buildSummary();
    meta.AddMember("summary", rapidjson::Value(summary.c_str(), alloc), alloc);
    doc.AddMember("meta", meta, alloc);

    rapidjson::Value core(rapidjson::kObjectType);
    core.AddMember("coin", Core::getInstance()->getResource(ResourceType::COIN), alloc);
    core.AddMember("diamond", Core::getInstance()->getResource(ResourceType::DIAMOND), alloc);
    core.AddMember("totalCoin", static_cast<int64_t>(Core::getInstance()->getTotalEarned(ResourceType::COIN)), alloc);
    core.AddMember("totalDiamond", static_cast<int64_t>(Core::getInstance()->getTotalEarned(ResourceType::DIAMOND)), alloc);
    core.AddMember("baseLevel", Core::getInstance()->getBaseLevel(), alloc);
    rapidjson::Value stars(rapidjson::kArrayType);
    for (const auto& pair : Core::getInstance()->getLevelStarsData()) {
        rapidjson::Value item(rapidjson::kObjectType);
        item.AddMember("id", pair.first, alloc);
        item.AddMember("stars", pair.second, alloc);
        stars.PushBack(item, alloc);
    }
    core.AddMember("levelStars", stars, alloc);
    doc.AddMember("core", core, alloc);

    rapidjson::Value units(rapidjson::kObjectType);
    rapidjson::Value trained(rapidjson::kArrayType);
    const auto& trainedUnits = UnitManager::getInstance()->getTrainedUnits();
    for (const auto& pair : trainedUnits) {
        rapidjson::Value item(rapidjson::kObjectType);
        item.AddMember("id", pair.first, alloc);
        item.AddMember("count", pair.second, alloc);
        trained.PushBack(item, alloc);
    }
    units.AddMember("trained", trained, alloc);

    rapidjson::Value levels(rapidjson::kArrayType);
    const auto& unitLevels = UnitManager::getInstance()->getUnitLevels();
    for (const auto& pair : unitLevels) {
        rapidjson::Value item(rapidjson::kObjectType);
        item.AddMember("id", pair.first, alloc);
        item.AddMember("level", pair.second, alloc);
        levels.PushBack(item, alloc);
    }
    units.AddMember("levels", levels, alloc);
    doc.AddMember("units", units, alloc);

    rapidjson::Value base(rapidjson::kObjectType);
    Vec2 baseAnchor = BaseScene::getBaseAnchorGrid();
    Vec2 barracksAnchor = BaseScene::getBarracksAnchorGrid();
    base.AddMember("baseAnchorX", static_cast<int>(std::round(baseAnchor.x)), alloc);
    base.AddMember("baseAnchorY", static_cast<int>(std::round(baseAnchor.y)), alloc);
    base.AddMember("barracksAnchorX", static_cast<int>(std::round(barracksAnchor.x)), alloc);
    base.AddMember("barracksAnchorY", static_cast<int>(std::round(barracksAnchor.y)), alloc);
    base.AddMember("barracksLevel", BaseScene::getBarracksLevel(), alloc);

    rapidjson::Value buildings(rapidjson::kArrayType);
    const auto& savedBuildings = BaseScene::getSavedBuildings();
    for (const auto& saved : savedBuildings) {
        rapidjson::Value item(rapidjson::kObjectType);
        item.AddMember("gridX", saved.gridX, alloc);
        item.AddMember("gridY", saved.gridY, alloc);
        item.AddMember("level", saved.level, alloc);

        rapidjson::Value option(rapidjson::kObjectType);
        option.AddMember("type", saved.option.type, alloc);
        option.AddMember("configId", saved.option.configId, alloc);
        std::string category = categoryToString(saved.option.category);
        option.AddMember("category", rapidjson::Value(category.c_str(), alloc), alloc);
        option.AddMember("name", rapidjson::Value(saved.option.name.c_str(), alloc), alloc);
        option.AddMember("cost", saved.option.cost, alloc);
        option.AddMember("gridWidth", saved.option.gridWidth, alloc);
        option.AddMember("gridHeight", saved.option.gridHeight, alloc);
        option.AddMember("spritePath", rapidjson::Value(saved.option.spritePath.c_str(), alloc), alloc);
        option.AddMember("canBuild", saved.option.canBuild, alloc);
        item.AddMember("option", option, alloc);

        buildings.PushBack(item, alloc);
    }
    base.AddMember("buildings", buildings, alloc);
    doc.AddMember("base", base, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    localStorageSetItem(getSlotKey(slot), buffer.GetString());
    return true;
}

bool SaveManager::saveActiveSlot() {
    if (!isValidSlot(_activeSlot)) {
        return false;
    }
    return saveSlot(_activeSlot);
}

bool SaveManager::loadSlot(int slot) {
    if (!isValidSlot(slot)) {
        return false;
    }
    init();

    std::string jsonData;
    if (!localStorageGetItem(getSlotKey(slot), &jsonData) || jsonData.empty()) {
        resetGameState();
        return false;
    }

    rapidjson::Document doc;
    doc.Parse(jsonData.c_str());
    if (doc.HasParseError() || !doc.IsObject()) {
        resetGameState();
        return false;
    }

    resetGameState();

    if (doc.HasMember("core") && doc["core"].IsObject()) {
        const auto& core = doc["core"];
        Core::getInstance()->setResource(ResourceType::COIN, readInt(core, "coin", 0));
        Core::getInstance()->setResource(ResourceType::DIAMOND, readInt(core, "diamond", 0));
        Core::getInstance()->setTotalEarned(ResourceType::COIN, readInt64(core, "totalCoin", 0));
        Core::getInstance()->setTotalEarned(ResourceType::DIAMOND, readInt64(core, "totalDiamond", 0));
        Core::getInstance()->setBaseLevel(readInt(core, "baseLevel", 1));

        if (core.HasMember("levelStars") && core["levelStars"].IsArray()) {
            std::vector<std::pair<int, int>> stars;
            for (const auto& item : core["levelStars"].GetArray()) {
                if (!item.IsObject()) {
                    continue;
                }
                int id = readInt(item, "id", 0);
                int star = readInt(item, "stars", 0);
                if (id > 0) {
                    stars.emplace_back(id, star);
                }
            }
            Core::getInstance()->setLevelStarsData(stars);
        }
    }

    if (doc.HasMember("units") && doc["units"].IsObject()) {
        const auto& units = doc["units"];
        std::map<int, int> trainedUnits;
        if (units.HasMember("trained") && units["trained"].IsArray()) {
            for (const auto& item : units["trained"].GetArray()) {
                if (!item.IsObject()) {
                    continue;
                }
                int id = readInt(item, "id", 0);
                int count = readInt(item, "count", 0);
                if (id > 0 && count > 0) {
                    trainedUnits[id] = count;
                }
            }
        }
        UnitManager::getInstance()->setTrainedUnits(trainedUnits);

        std::map<int, int> unitLevels;
        if (units.HasMember("levels") && units["levels"].IsArray()) {
            for (const auto& item : units["levels"].GetArray()) {
                if (!item.IsObject()) {
                    continue;
                }
                int id = readInt(item, "id", 0);
                int level = readInt(item, "level", 0);
                if (id > 0) {
                    unitLevels[id] = level;
                }
            }
        }
        UnitManager::getInstance()->setUnitLevels(unitLevels);
    }

    if (doc.HasMember("base") && doc["base"].IsObject()) {
        const auto& base = doc["base"];
        Vec2 defaultBaseAnchor = BaseScene::getBaseAnchorGrid();
        Vec2 defaultBarracksAnchor = BaseScene::getBarracksAnchorGrid();
        int defaultBarracksLevel = BaseScene::getBarracksLevel();
        Vec2 baseAnchor(static_cast<float>(readInt(base, "baseAnchorX", static_cast<int>(std::round(defaultBaseAnchor.x)))),
                        static_cast<float>(readInt(base, "baseAnchorY", static_cast<int>(std::round(defaultBaseAnchor.y)))));
        Vec2 barracksAnchor(static_cast<float>(readInt(base, "barracksAnchorX", static_cast<int>(std::round(defaultBarracksAnchor.x)))),
                            static_cast<float>(readInt(base, "barracksAnchorY", static_cast<int>(std::round(defaultBarracksAnchor.y)))));
        int barracksLevel = readInt(base, "barracksLevel", defaultBarracksLevel);

        std::vector<BaseSavedBuilding> savedBuildings;
        if (base.HasMember("buildings") && base["buildings"].IsArray()) {
            for (const auto& item : base["buildings"].GetArray()) {
                if (!item.IsObject()) {
                    continue;
                }
                BaseSavedBuilding saved;
                saved.gridX = readInt(item, "gridX", 0);
                saved.gridY = readInt(item, "gridY", 0);
                saved.level = readInt(item, "level", 0);

                if (item.HasMember("option") && item["option"].IsObject()) {
                    const auto& option = item["option"];
                    saved.option.type = readInt(option, "type", 0);
                    saved.option.configId = readInt(option, "configId", saved.option.type);
                    saved.option.category = stringToCategory(readString(option, "category", ""));
                    saved.option.name = readString(option, "name", "");
                    saved.option.cost = readInt(option, "cost", 0);
                    saved.option.gridWidth = readInt(option, "gridWidth", 0);
                    saved.option.gridHeight = readInt(option, "gridHeight", 0);
                    saved.option.spritePath = readString(option, "spritePath", "");
                    saved.option.canBuild = readBool(option, "canBuild", true);
                }
                savedBuildings.push_back(saved);
            }
        }

        BaseScene::applySavedState(savedBuildings, baseAnchor, barracksAnchor, barracksLevel);
    }

    return true;
}

bool SaveManager::deleteSlot(int slot) {
    if (!isValidSlot(slot)) {
        return false;
    }
    init();
    localStorageRemoveItem(getSlotKey(slot));
    if (_activeSlot == slot) {
        _activeSlot = 0;
    }
    return true;
}

bool SaveManager::hasSlot(int slot) const {
    if (!isValidSlot(slot)) {
        return false;
    }
    const_cast<SaveManager*>(this)->init();
    std::string jsonData;
    return localStorageGetItem(getSlotKey(slot), &jsonData) && !jsonData.empty();
}

void SaveManager::setActiveSlot(int slot) {
    if (!isValidSlot(slot)) {
        _activeSlot = 0;
        return;
    }
    _activeSlot = slot;
}

int SaveManager::getActiveSlot() const {
    return _activeSlot;
}

SaveSlotInfo SaveManager::getSlotInfo(int slot) const {
    SaveSlotInfo info;
    info.slot = slot;
    if (!isValidSlot(slot)) {
        return info;
    }
    const_cast<SaveManager*>(this)->init();

    std::string jsonData;
    if (!localStorageGetItem(getSlotKey(slot), &jsonData) || jsonData.empty()) {
        info.exists = false;
        info.summary = "Empty";
        return info;
    }

    rapidjson::Document doc;
    doc.Parse(jsonData.c_str());
    if (doc.HasParseError() || !doc.IsObject()) {
        info.exists = false;
        info.summary = "Empty";
        return info;
    }

    info.exists = true;
    if (doc.HasMember("meta") && doc["meta"].IsObject()) {
        info.summary = readString(doc["meta"], "summary", "");
    }
    if (info.summary.empty()) {
        info.summary = "Saved";
    }
    return info;
}

std::vector<SaveSlotInfo> SaveManager::listSlots() const {
    std::vector<SaveSlotInfo> slots;
    slots.reserve(kSlotCount);
    for (int i = 1; i <= kSlotCount; ++i) {
        slots.push_back(getSlotInfo(i));
    }
    return slots;
}
