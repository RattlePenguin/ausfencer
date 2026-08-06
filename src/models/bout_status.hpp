#pragma once

#include <string>
#include <string_view>
#include <nlohmann/json.hpp>

enum class BoutStatus {
    NotStarted,
    InProgress,
    Finished
};

inline std::string boutStatusToString(BoutStatus status) {
    switch (status) {
        case BoutStatus::NotStarted: return "NotStarted";
        case BoutStatus::InProgress: return "InProgress";
        case BoutStatus::Finished: return "Finished";
        default: return "NotStarted";
    }
}

inline BoutStatus stringToBoutStatus(std::string_view str) {
    if (str == "NotStarted" || str == "notstarted") return BoutStatus::NotStarted;
    if (str == "InProgress" || str == "inprogress") return BoutStatus::InProgress;
    if (str == "Finished" || str == "finished") return BoutStatus::Finished;
    return BoutStatus::NotStarted;
}

inline void to_json(nlohmann::json& j, const BoutStatus& status) {
    j = boutStatusToString(status);
}

inline void from_json(const nlohmann::json& j, BoutStatus& status) {
    if (j.is_string()) {
        status = stringToBoutStatus(j.get<std::string>());
    } else if (j.is_number_integer()) {
        status = static_cast<BoutStatus>(j.get<int>());
    } else {
        status = BoutStatus::NotStarted;
    }
}
