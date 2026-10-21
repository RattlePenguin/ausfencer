#include "services/statistics.hpp"

TrackerStats calculate_statistics(const DbRows &history) {
  TrackerStats stats;
  for (const auto &bout : history) {
    if (bout.at("disputed") == "1") { ++stats.disputed; continue; }
    if (bout.at("status") == "unfinished") { ++stats.unfinished; continue; }
    if (bout.at("status") != "completed") { ++stats.administrative; continue; }
    if (bout.at("result") == "win") ++stats.wins;
    else ++stats.losses;
    if (bout.at("date").empty()) ++stats.undated;
    if (bout.count("scored") && bout.count("received")) {
      stats.scored += std::stoi(bout.at("scored"));
      stats.received += std::stoi(bout.at("received"));
      ++stats.score_known;
    }
  }
  return stats;
}
