#include "services/fixtures.hpp"
#include <map>
#include <set>
#include <regex>
#include <stdexcept>

namespace {
void require(bool valid, const char *message) {
  if (!valid) throw std::invalid_argument(message);
}
}

bool valid_competition_date(const std::string &date) {
  if (!std::regex_match(date, std::regex("[0-9]{4}-[0-9]{2}-[0-9]{2}"))) return false;
  int year = std::stoi(date.substr(0,4)), month = std::stoi(date.substr(5,2));
  int day = std::stoi(date.substr(8,2));
  const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if (year < 1900 || month < 1 || month > 12) return false;
  int leap = month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
  return day > 0 && day <= days[month-1] + leap;
}

void validate_fixture(const FixtureData &fixture) {
  std::set<int> people;
  std::set<std::string> external, events;
  std::map<std::pair<std::string,int>, std::string> entries;
  require(!fixture.fencers.empty() && !fixture.events.empty(), "Empty fixture");
  for (const auto &p : fixture.fencers) {
    require(p.id > 0 && !p.name.empty() && p.name.size() <= 200, "Invalid profile");
    require(people.insert(p.id).second, "Duplicate profile ID");
    require(!p.external_id.empty() && external.insert(p.external_id).second, "Duplicate external ID");
  }
  for (const auto &e : fixture.events) {
    require(!e.id.empty() && !e.tournament.empty() && !e.name.empty(), "Missing event fields");
    require(events.insert(e.id).second, "Duplicate event ID");
    require(valid_competition_date(e.date), "Invalid competition date");
    require(e.weapon == "epee" || e.weapon == "foil" || e.weapon == "sabre", "Invalid weapon");
    require(!e.timezone.empty() && !e.category.empty(), "Missing event context");
    require(std::set<std::string>{"complete","partial","results-only","unresolved"}.count(e.coverage),
            "Invalid coverage");
  }
  for (const auto &e : fixture.entries) {
    require(people.count(e.fencer) && events.count(e.event), "Entry references missing identity/event");
    require(e.seed > 0 && e.place > 0 && !e.source_name.empty(), "Invalid event entry");
    require(entries.emplace(std::make_pair(e.event,e.fencer),e.pool).second, "Duplicate entry");
  }
  for (const auto &b : fixture.bouts) {
    require(!b.id.empty() && b.left != b.right, "Invalid bout identity");
    require(entries.count({b.event,b.left}) && entries.count({b.event,b.right}), "Missing bout entry");
    require(!b.winner || *b.winner == b.left || *b.winner == b.right, "Winner is not a participant");
    require(b.left_score.has_value() == b.right_score.has_value(), "Incomplete scoreboard");
    require(!b.left_score || (*b.left_score >= 0 && *b.left_score <= 99 &&
      *b.right_score >= 0 && *b.right_score <= 99), "Invalid scoreboard");
    require(std::set<std::string>{"completed","withdrawal","administrative","unfinished"}.count(b.status),
            "Invalid bout status");
    require(b.status != "completed" || b.winner.has_value(), "Completed bout needs explicit winner");
    require(b.trust == "synthetic", "Local fixtures must be synthetic");
    require(std::set<std::string>{"pool","R16","QF","SF","F"}.count(b.stage), "Invalid round");
    require(b.stage != "pool" || (!b.pool.empty() && entries.at({b.event,b.left}) == b.pool &&
      entries.at({b.event,b.right}) == b.pool), "Invalid poule membership");
  }
}
