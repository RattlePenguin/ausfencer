#pragma once

#include "handlers/input.hpp"
#include "models/Bout.hpp"
#include <algorithm>

namespace bout_json {
inline std::optional<int> participant(const crow::json::rvalue &json,
                                       const std::string &key) {
  if (input::field(json, key).t() == crow::json::type::Null) return std::nullopt;
  return static_cast<int>(input::integer(json, key, 1, std::numeric_limits<int>::max()));
}

inline Bout parse(const std::string &body) {
  auto json = input::object(body);
  Bout bout{};
  bout.left_fencer_id = participant(json, "left_fencer_id");
  bout.right_fencer_id = participant(json, "right_fencer_id");
  const auto max_time = std::min<std::uintmax_t>(std::numeric_limits<time_t>::max(),
                                               INT64_MAX);
  bout.timestamp = static_cast<time_t>(input::integer(json, "timestamp", 0, max_time));
  const auto &weapon = input::field(json, "weapon");
  if (weapon.t() != crow::json::type::String)
    throw std::invalid_argument("Expected weapon string");
  const std::string w = weapon.s();
  if (w == "foil") bout.weapon = Foil;
  else if (w == "epee") bout.weapon = Epee;
  else if (w == "sabre") bout.weapon = Sabre;
  else throw std::invalid_argument("Unknown weapon");
  bout.time = input::integer(json, "time", 0, 86400);
  bout.left_score = input::integer(json, "left_score", 0, 99);
  bout.right_score = input::integer(json, "right_score", 0, 99);
  bout.left_yellow = input::integer(json, "left_yellow", 0, 1);
  bout.right_yellow = input::integer(json, "right_yellow", 0, 1);
  bout.left_red = input::integer(json, "left_red", 0, 99);
  bout.right_red = input::integer(json, "right_red", 0, 99);
  return bout;
}

inline crow::json::wvalue serialize(const Bout &bout) {
  crow::json::wvalue json;
  json["id"] = bout.id;
  json["left_fencer_id"] = bout.left_fencer_id ? crow::json::wvalue(*bout.left_fencer_id)
                                             : crow::json::wvalue(nullptr);
  json["right_fencer_id"] = bout.right_fencer_id ? crow::json::wvalue(*bout.right_fencer_id)
                                               : crow::json::wvalue(nullptr);
  json["timestamp"] = static_cast<std::int64_t>(bout.timestamp);
  switch (bout.weapon) {
    case Foil: json["weapon"] = "foil"; break;
    case Epee: json["weapon"] = "epee"; break;
    case Sabre: json["weapon"] = "sabre"; break;
    default: throw std::domain_error("Invalid stored weapon");
  }
  json["time"] = bout.time;
  json["left_score"] = bout.left_score;
  json["right_score"] = bout.right_score;
  json["left_yellow"] = bout.left_yellow;
  json["right_yellow"] = bout.right_yellow;
  json["left_red"] = bout.left_red;
  json["right_red"] = bout.right_red;
  return json;
}
} // namespace bout_json
