#include "Replay/ReplayManager.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace {
constexpr int kReplayVersion = 1;

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

int64_t readInt64(const rapidjson::Value& obj, const char* key, int64_t fallback) {
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
        return static_cast<int64_t>(it->value.GetDouble());
    }
    return fallback;
}

float readFloat(const rapidjson::Value& obj, const char* key, float fallback) {
    auto it = obj.FindMember(key);
    if (it == obj.MemberEnd()) {
        return fallback;
    }
    if (it->value.IsNumber()) {
        return static_cast<float>(it->value.GetDouble());
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

std::string serializeReplay(const BattleReplay& replay) {
    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    doc.AddMember("version", replay.version, alloc);
    doc.AddMember("levelId", replay.levelId, alloc);
    doc.AddMember("defenseMode", replay.defenseMode, alloc);
    doc.AddMember("allowDefaultUnits", replay.allowDefaultUnits, alloc);
    doc.AddMember("battleSpeed", replay.battleSpeed, alloc);
    doc.AddMember("timestamp", static_cast<int64_t>(replay.timestamp), alloc);
    doc.AddMember("resultWin", replay.resultWin, alloc);
    doc.AddMember("resultStars", replay.resultStars, alloc);
    doc.AddMember("duration", replay.duration, alloc);

    rapidjson::Value units(rapidjson::kArrayType);
    for (const auto& pair : replay.deployableUnits) {
        rapidjson::Value item(rapidjson::kObjectType);
        item.AddMember("id", pair.first, alloc);
        item.AddMember("count", pair.second, alloc);
        units.PushBack(item, alloc);
    }
    doc.AddMember("units", units, alloc);

    rapidjson::Value events(rapidjson::kArrayType);
    for (const auto& event : replay.events) {
        rapidjson::Value item(rapidjson::kObjectType);
        item.AddMember("t", event.time, alloc);
        item.AddMember("id", event.unitId, alloc);
        item.AddMember("x", event.gridX, alloc);
        item.AddMember("y", event.gridY, alloc);
        item.AddMember("level", event.level, alloc);
        events.PushBack(item, alloc);
    }
    doc.AddMember("events", events, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

bool parseReplay(const std::string& json, BattleReplay* outReplay) {
    if (!outReplay) {
        return false;
    }
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    if (doc.HasParseError() || !doc.IsObject()) {
        return false;
    }

    BattleReplay replay;
    replay.version = readInt(doc, "version", kReplayVersion);
    replay.levelId = readInt(doc, "levelId", 1);
    replay.defenseMode = readBool(doc, "defenseMode", false);
    replay.allowDefaultUnits = readBool(doc, "allowDefaultUnits", true);
    replay.battleSpeed = readFloat(doc, "battleSpeed", 1.0f);
    replay.timestamp = readInt64(doc, "timestamp", 0);
    replay.resultWin = readBool(doc, "resultWin", false);
    replay.resultStars = readInt(doc, "resultStars", 0);
    replay.duration = readFloat(doc, "duration", 0.0f);

    if (doc.HasMember("units") && doc["units"].IsArray()) {
        for (const auto& item : doc["units"].GetArray()) {
            if (!item.IsObject()) {
                continue;
            }
            int id = readInt(item, "id", 0);
            int count = readInt(item, "count", 0);
            if (id > 0 && count > 0) {
                replay.deployableUnits[id] = count;
            }
        }
    }

    if (doc.HasMember("events") && doc["events"].IsArray()) {
        for (const auto& item : doc["events"].GetArray()) {
            if (!item.IsObject()) {
                continue;
            }
            ReplayDeployEvent event;
            event.time = readFloat(item, "t", 0.0f);
            event.unitId = readInt(item, "id", 0);
            event.gridX = readInt(item, "x", 0);
            event.gridY = readInt(item, "y", 0);
            event.level = readInt(item, "level", 0);
            if (event.unitId > 0) {
                replay.events.push_back(event);
            }
        }
    }

    *outReplay = replay;
    return true;
}
} // namespace

ReplayManager* ReplayManager::s_instance = nullptr;

ReplayManager* ReplayManager::getInstance() {
    if (!s_instance) {
        s_instance = new ReplayManager();
    }
    return s_instance;
}

void ReplayManager::setLastReplay(const BattleReplay& replay) {
    _lastReplay = replay;
    _hasLastReplay = true;
}

bool ReplayManager::hasLastReplay() {
    if (_hasLastReplay) {
        return true;
    }
    return loadLastReplay();
}

const BattleReplay* ReplayManager::getLastReplay() {
    if (!_hasLastReplay) {
        loadLastReplay();
    }
    return _hasLastReplay ? &_lastReplay : nullptr;
}

bool ReplayManager::saveLastReplay() {
    if (!_hasLastReplay) {
        return false;
    }
    std::string path = buildReplayPath();
    std::string data = serializeReplay(_lastReplay);
    return FileUtils::getInstance()->writeStringToFile(data, path);
}

bool ReplayManager::loadLastReplay() {
    std::string path = buildReplayPath();
    if (!FileUtils::getInstance()->isFileExist(path)) {
        return false;
    }
    std::string json = FileUtils::getInstance()->getStringFromFile(path);
    if (json.empty()) {
        return false;
    }
    BattleReplay replay;
    if (!parseReplay(json, &replay)) {
        return false;
    }
    _lastReplay = replay;
    _hasLastReplay = true;
    return true;
}

std::string ReplayManager::getLastReplayPath() const {
    return buildReplayPath();
}

std::string ReplayManager::buildReplayPath() const {
    auto* fileUtils = FileUtils::getInstance();
    std::string base = fileUtils->getWritablePath();
    std::string dir = base + "replays/";
    fileUtils->createDirectory(dir);
    return dir + "last_replay.json";
}

bool ReplayManager::exportReplayTo(const std::string& path, const BattleReplay& replay) {
    if (path.empty()) {
        return false;
    }
    auto* fileUtils = FileUtils::getInstance();
    std::string dir;
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        dir = path.substr(0, pos + 1);
    }
    if (!dir.empty() && !fileUtils->isDirectoryExist(dir)) {
        fileUtils->createDirectory(dir);
    }
    std::string data = serializeReplay(replay);
    return fileUtils->writeStringToFile(data, path);
}

bool ReplayManager::importReplayFrom(const std::string& path, BattleReplay* outReplay) const {
    if (!outReplay || path.empty()) {
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
    return parseReplay(data, outReplay);
}
