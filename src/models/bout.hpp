#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>
#include <nlohmann/json.hpp>
#include "weapon.hpp"
#include "bout_status.hpp"

// Utility function to generate UUID v4
inline std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4"; // UUID version 4
    for (i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen); // Variant 10xx
    for (i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (i = 0; i < 12; i++) ss << dis(gen);
    return ss.str();
}

// Utility function to get current ISO 8601 UTC timestamp
inline std::string currentISOTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

struct Bout {
    std::string id = generateUUID();
    std::string leftFencerName = "";
    std::string rightFencerName = "";

    int leftScore = 0;
    int rightScore = 0;

    int timeRemainingSeconds = 180;
    bool timeRunning = false;

    int leftYellowCards = 0;
    int rightYellowCards = 0;
    int leftRedCards = 0;
    int rightRedCards = 0;

    Weapon weapon = Weapon::Foil;
    BoutStatus status = BoutStatus::NotStarted;

    std::string createdAt = currentISOTimestamp();
    std::optional<std::string> finishedAt = std::nullopt;
};

inline void to_json(nlohmann::json& j, const Bout& b) {
    j = nlohmann::json{
        {"id", b.id},
        {"leftFencerName", b.leftFencerName},
        {"rightFencerName", b.rightFencerName},
        {"leftScore", b.leftScore},
        {"rightScore", b.rightScore},
        {"timeRemainingSeconds", b.timeRemainingSeconds},
        {"timeRunning", b.timeRunning},
        {"leftYellowCards", b.leftYellowCards},
        {"rightYellowCards", b.rightYellowCards},
        {"leftRedCards", b.leftRedCards},
        {"rightRedCards", b.rightRedCards},
        {"weapon", b.weapon},
        {"status", b.status},
        {"createdAt", b.createdAt}
    };
    if (b.finishedAt.has_value()) {
        j["finishedAt"] = b.finishedAt.value();
    } else {
        j["finishedAt"] = nullptr;
    }
}

inline void from_json(const nlohmann::json& j, Bout& b) {
    if (j.contains("id") && !j["id"].is_null()) {
        b.id = j.value("id", b.id);
    } else if (b.id.empty()) {
        b.id = generateUUID();
    }

    b.leftFencerName = j.value("leftFencerName", "");
    b.rightFencerName = j.value("rightFencerName", "");
    b.leftScore = j.value("leftScore", 0);
    b.rightScore = j.value("rightScore", 0);
    b.timeRemainingSeconds = j.value("timeRemainingSeconds", 180);
    b.timeRunning = j.value("timeRunning", false);
    b.leftYellowCards = j.value("leftYellowCards", 0);
    b.rightYellowCards = j.value("rightYellowCards", 0);
    b.leftRedCards = j.value("leftRedCards", 0);
    b.rightRedCards = j.value("rightRedCards", 0);

    if (j.contains("weapon")) {
        b.weapon = j.at("weapon").get<Weapon>();
    }
    if (j.contains("status")) {
        b.status = j.at("status").get<BoutStatus>();
    }

    if (j.contains("createdAt") && !j["createdAt"].is_null()) {
        b.createdAt = j.at("createdAt").get<std::string>();
    }
    if (j.contains("finishedAt") && !j["finishedAt"].is_null()) {
        b.finishedAt = j.at("finishedAt").get<std::string>();
    }
}
