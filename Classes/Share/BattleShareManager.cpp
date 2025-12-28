#include "Share/BattleShareManager.h"
#include "Core/Core.h"
#include "Soldier/UnitManager.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include <functional>

using namespace cocos2d;

namespace {
constexpr int kSnapshotVersion = 1;
const char* kShareDir = "share/";
const char* kOutgoingSnapshotFile = "my_base_snapshot.json";
const char* kIncomingSnapshotFile = "target_base_snapshot.json";
const char* kOutgoingReplayFile = "last_replay.json";
const char* kIncomingReplayFile = "target_replay.json";
const char* kBuildingsConfigPath = "res/buildings_config.json";
const char* kUnitsConfigPath = "res/units_config.json";

int readInt(const rapidjson::Value& obj, const char* key, int fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) return fallback;
    if (it->value.IsInt()) return it->value.GetInt();
    if (it->value.IsNumber()) return static_cast<int>(it->value.GetDouble());
    return fallback;
}

float readFloat(const rapidjson::Value& obj, const char* key, float fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) return fallback;
    if (it->value.IsNumber()) return static_cast<float>(it->value.GetDouble());
    return fallback;
}

std::string readString(const rapidjson::Value& obj, const char* key, const std::string& fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) return fallback;
    if (it->value.IsString()) return it->value.GetString();
    return fallback;
}

bool readBool(const rapidjson::Value& obj, const char* key, bool fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) return fallback;
    if (it->value.IsBool()) return it->value.GetBool();
    return fallback;
}

Vec2 readVec2(const rapidjson::Value& obj) {
    float x = readFloat(obj, "x", 0.0f);
    float y = readFloat(obj, "y", 0.0f);
    return Vec2(x, y);
}

void writeVec2(rapidjson::Value& dst, const Vec2& value, rapidjson::Document::AllocatorType& alloc) {
    dst.SetObject();
    dst.AddMember("x", value.x, alloc);
    dst.AddMember("y", value.y, alloc);
}

std::string toHexHash(size_t value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%zx", value);
    return buffer;
}

bool writeDocumentToFile(const rapidjson::Document& doc, const std::string& path) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return FileUtils::getInstance()->writeStringToFile(buffer.GetString(), path);
}
} // namespace

BattleShareManager* BattleShareManager::s_instance = nullptr;

BattleShareManager* BattleShareManager::getInstance() {
    if (!s_instance) {
        s_instance = new BattleShareManager();
    }
    return s_instance;
}

BaseSnapshot BattleShareManager::captureCurrentBase() const {
    BaseSnapshot snapshot;
    snapshot.version = kSnapshotVersion;
    snapshot.configHash = computeConfigHash();
    snapshot.baseLevel = std::max(0, Core::getInstance()->getBaseLevel() - 1);
    snapshot.barracksLevel = std::max(0, BaseScene::getBarracksLevel());
    snapshot.baseAnchor = BaseScene::getBaseAnchorGrid();
    snapshot.barracksAnchor = BaseScene::getBarracksAnchorGrid();
    snapshot.buildings = BaseScene::getSavedBuildings();
    return snapshot;
}

bool BattleShareManager::exportPlayerBaseSnapshot(std::string* outPath) {
    BaseSnapshot snapshot = captureCurrentBase();
    std::string path = getOutgoingSnapshotPath();
    bool ok = saveSnapshotToPath(snapshot, path);
    if (ok && outPath) {
        *outPath = path;
    }
    return ok;
}

bool BattleShareManager::loadIncomingSnapshot(BaseSnapshot* outSnapshot) {
    if (!outSnapshot) {
        return false;
    }
    return loadSnapshotFromPath(getIncomingSnapshotPath(), outSnapshot);
}

bool BattleShareManager::saveSnapshotToPath(const BaseSnapshot& snapshot, const std::string& path) {
    ensureShareDirectory();
    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    doc.AddMember("version", snapshot.version, alloc);
    doc.AddMember("configHash", rapidjson::Value(snapshot.configHash.c_str(), alloc), alloc);
    doc.AddMember("baseLevel", snapshot.baseLevel, alloc);
    doc.AddMember("barracksLevel", snapshot.barracksLevel, alloc);

    rapidjson::Value baseAnchorVal;
    writeVec2(baseAnchorVal, snapshot.baseAnchor, alloc);
    doc.AddMember("baseAnchor", baseAnchorVal, alloc);

    rapidjson::Value barracksAnchorVal;
    writeVec2(barracksAnchorVal, snapshot.barracksAnchor, alloc);
    doc.AddMember("barracksAnchor", barracksAnchorVal, alloc);

    rapidjson::Value buildings(rapidjson::kArrayType);
    for (const auto& saved : snapshot.buildings) {
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
    doc.AddMember("buildings", buildings, alloc);

    return writeDocumentToFile(doc, path);
}

bool BattleShareManager::loadSnapshotFromPath(const std::string& path, BaseSnapshot* outSnapshot) {
    if (!outSnapshot) {
        return false;
    }
    auto* fileUtils = FileUtils::getInstance();
    if (!fileUtils->isFileExist(path)) {
        return false;
    }
    std::string data = fileUtils->getStringFromFile(path);
    if (data.empty()) {
        return false;
    }

    rapidjson::Document doc;
    doc.Parse(data.c_str());
    if (doc.HasParseError() || !doc.IsObject()) {
        return false;
    }

    BaseSnapshot snapshot;
    snapshot.version = readInt(doc, "version", kSnapshotVersion);
    snapshot.configHash = readString(doc, "configHash", "");
    snapshot.baseLevel = readInt(doc, "baseLevel", 0);
    snapshot.barracksLevel = readInt(doc, "barracksLevel", 0);
    if (doc.HasMember("baseAnchor") && doc["baseAnchor"].IsObject()) {
        snapshot.baseAnchor = readVec2(doc["baseAnchor"]);
    }
    if (doc.HasMember("barracksAnchor") && doc["barracksAnchor"].IsObject()) {
        snapshot.barracksAnchor = readVec2(doc["barracksAnchor"]);
    }

    if (doc.HasMember("buildings") && doc["buildings"].IsArray()) {
        for (const auto& item : doc["buildings"].GetArray()) {
            if (!item.IsObject()) {
                continue;
            }
            BaseSavedBuilding saved;
            saved.gridX = readInt(item, "gridX", 0);
            saved.gridY = readInt(item, "gridY", 0);
            saved.level = readInt(item, "level", 0);
            if (item.HasMember("option") && item["option"].IsObject()) {
                auto& optionObj = item["option"];
                saved.option.type = readInt(optionObj, "type", 0);
                saved.option.configId = readInt(optionObj, "configId", saved.option.type);
                saved.option.category = stringToCategory(readString(optionObj, "category", ""));
                saved.option.name = readString(optionObj, "name", "");
                saved.option.cost = readInt(optionObj, "cost", 0);
                saved.option.gridWidth = readInt(optionObj, "gridWidth", 0);
                saved.option.gridHeight = readInt(optionObj, "gridHeight", 0);
                saved.option.spritePath = readString(optionObj, "spritePath", "");
                saved.option.canBuild = readBool(optionObj, "canBuild", true);
            }
            snapshot.buildings.push_back(saved);
        }
    }

    *outSnapshot = snapshot;
    return true;
}

void BattleShareManager::setActiveTargetSnapshot(const BaseSnapshot& snapshot) {
    _targetSnapshot = snapshot;
    _hasTargetSnapshot = true;
}

bool BattleShareManager::hasActiveTargetSnapshot() const {
    return _hasTargetSnapshot;
}

const BaseSnapshot* BattleShareManager::getActiveTargetSnapshot() const {
    return _hasTargetSnapshot ? &_targetSnapshot : nullptr;
}

void BattleShareManager::clearActiveTargetSnapshot() {
    _hasTargetSnapshot = false;
    _targetSnapshot = BaseSnapshot();
}

bool BattleShareManager::exportLastReplay(std::string* outPath) {
    auto* replayMgr = ReplayManager::getInstance();
    const BattleReplay* replay = replayMgr->getLastReplay();
    if (!replay) {
        return false;
    }
    std::string path = getOutgoingReplayPath();
    bool ok = replayMgr->exportReplayTo(path, *replay);
    if (ok && outPath) {
        *outPath = path;
    }
    return ok;
}

bool BattleShareManager::loadIncomingReplay(BattleReplay* outReplay) const {
    if (!outReplay) {
        return false;
    }
    return ReplayManager::getInstance()->importReplayFrom(getIncomingReplayPath(), outReplay);
}

bool BattleShareManager::hasIncomingSnapshot() const {
    return FileUtils::getInstance()->isFileExist(getIncomingSnapshotPath());
}

bool BattleShareManager::hasIncomingReplay() const {
    return FileUtils::getInstance()->isFileExist(getIncomingReplayPath());
}

std::string BattleShareManager::getShareDirectory() const {
    auto* fileUtils = FileUtils::getInstance();
    return fileUtils->getWritablePath() + std::string(kShareDir);
}

std::string BattleShareManager::getOutgoingSnapshotPath() const {
    return buildSharePath(kOutgoingSnapshotFile);
}

std::string BattleShareManager::getIncomingSnapshotPath() const {
    return buildSharePath(kIncomingSnapshotFile);
}

std::string BattleShareManager::getOutgoingReplayPath() const {
    return buildSharePath(kOutgoingReplayFile);
}

std::string BattleShareManager::getIncomingReplayPath() const {
    return buildSharePath(kIncomingReplayFile);
}

std::string BattleShareManager::buildSharePath(const std::string& filename) const {
    return getShareDirectory() + filename;
}

void BattleShareManager::ensureShareDirectory() const {
    auto* fileUtils = FileUtils::getInstance();
    std::string dir = getShareDirectory();
    if (!fileUtils->isDirectoryExist(dir)) {
        fileUtils->createDirectory(dir);
    }
}

std::string BattleShareManager::computeConfigHash() const {
    auto* fileUtils = FileUtils::getInstance();
    std::string buildingFull = fileUtils->fullPathForFilename(kBuildingsConfigPath);
    std::string unitFull = fileUtils->fullPathForFilename(kUnitsConfigPath);
    std::string buildingData = buildingFull.empty() ? "" : fileUtils->getStringFromFile(buildingFull);
    std::string unitData = unitFull.empty() ? "" : fileUtils->getStringFromFile(unitFull);
    std::string merged = buildingData + "|" + unitData;
    std::hash<std::string> hasher;
    return toHexHash(hasher(merged));
}

std::string BattleShareManager::categoryToString(BuildingCategory category) const {
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

BuildingCategory BattleShareManager::stringToCategory(const std::string& str) const {
    if (str == "defence" || str == "defense") {
        return BuildingCategory::Defence;
    }
    if (str == "production") {
        return BuildingCategory::Production;
    }
    if (str == "storage") {
        return BuildingCategory::Storage;
    }
    if (str == "trap") {
        return BuildingCategory::Trap;
    }
    return BuildingCategory::Unknown;
}
