#pragma once

#include <optional>
#include <string>
#include <vector>

struct ProfileRecord {
  int id;
  std::string name, club, state, country, alias, external_id;
  bool guardian_required;
};

struct EventRecord {
  std::string id, tournament, name, date, weapon, category, coverage, timezone;
};

struct EntryRecord {
  std::string event;
  int fencer;
  std::string source_name, pool;
  int seed, place;
  bool bye;
};

struct CompetitionBout {
  std::string id, event, stage, pool;
  int left, right;
  std::optional<int> left_score, right_score, winner;
  std::string status, trust;
};

struct FixtureData {
  std::vector<ProfileRecord> fencers;
  std::vector<EventRecord> events;
  std::vector<EntryRecord> entries;
  std::vector<CompetitionBout> bouts;
  std::string checksum;
};

struct ImportSummary {
  int profiles_created, events_created, bouts_created;
  size_t repeated_observations;
};

struct TrackerFilter {
  std::string weapon, year, stage, trust, category, from, to, coverage;
  int opponent{0};
};

struct TrackerStats {
  int wins{0}, losses{0}, scored{0}, received{0}, score_known{0};
  int administrative{0}, unfinished{0}, disputed{0}, undated{0};
  std::optional<double> win_rate() const {
    if (!wins && !losses) return std::nullopt;
    return static_cast<double>(wins) / (wins + losses);
  }
  std::optional<double> touch_share() const {
    if (!scored && !received) return std::nullopt;
    return static_cast<double>(scored) / (scored + received);
  }
};
