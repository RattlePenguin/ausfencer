#include "handlers/tracker.hpp"
#include "services/statistics.hpp"
#include "services/fixtures.hpp"
#include <charconv>
#include <set>

TrackerFilter tracker_filter(const crow::request &req) {
  auto get = [&](const char *key) -> std::string { return req.url_params.get(key) ? req.url_params.get(key) : ""; };
  TrackerFilter filter{get("weapon"),get("year"),get("stage"),get("trust"),get("category"),get("from"),get("to")};
  filter.coverage = get("coverage");
  if (!std::set<std::string>{"","foil","epee","sabre"}.count(filter.weapon) ||
      !std::set<std::string>{"","pool","de","R16","QF","SF","F"}.count(filter.stage) ||
      !std::set<std::string>{"","synthetic","community","organiser"}.count(filter.trust) || filter.category.size()>100 ||
      !std::set<std::string>{"","complete","partial","results-only","unresolved"}.count(filter.coverage))
    throw std::invalid_argument("Invalid tracker filter");
  for (const auto &date : {filter.from,filter.to})
    if (!date.empty() && !valid_competition_date(date))
      throw std::invalid_argument("Dates must use YYYY-MM-DD");
  if (!filter.from.empty() && !filter.to.empty() && filter.from>filter.to) throw std::invalid_argument("Reversed date range");
  if (!get("opponent").empty()) {
    auto value = get("opponent");
    auto parsed = std::from_chars(value.data(),value.data()+value.size(),filter.opponent);
    if (parsed.ec != std::errc{} || parsed.ptr != value.data()+value.size() || filter.opponent < 1)
      throw std::invalid_argument("Invalid opponent");
  }
  return filter;
}

namespace {
crow::json::wvalue statistics(const DbRows &rows) {
  auto stats = calculate_statistics(rows);
  crow::json::wvalue result;
  result["wins"]=stats.wins; result["losses"]=stats.losses;
  result["scored"]=stats.scored; result["received"]=stats.received;
  result["differential"]=stats.scored-stats.received; result["score_known"]=stats.score_known;
  result["disputed"]=stats.disputed; result["administrative"]=stats.administrative;
  result["unfinished"]=stats.unfinished; result["undated"]=stats.undated;
  result["win_rate"]=stats.win_rate() ? crow::json::wvalue(*stats.win_rate()) : crow::json::wvalue(nullptr);
  result["touch_share"]=stats.touch_share() ? crow::json::wvalue(*stats.touch_share()) : crow::json::wvalue(nullptr);
  return result;
}
}

crow::json::wvalue TrackerHandler::read_profile(int id, const crow::request &request) {
  auto profile = repo_->profile(id);
  if (profile.empty()) throw TrackerNotFound("Profile not found");
  auto filter = tracker_filter(request);
  auto history = repo_->history(id,filter);
  crow::json::wvalue result;
  result["profile"]=public_rows(profile); result["bouts"]=public_rows(history);
  result["statistics"]=statistics(history); result["demo"]=true;
  result["events"]=public_rows(repo_->profile_events(id,filter));
  std::map<std::string,DbRows> years;
  for (const auto &bout : history) if (!bout.at("date").empty()) years[bout.at("date").substr(0,4)].push_back(bout);
  for (const auto &year : years) result["annual"][year.first]=statistics(year.second);
  return result;
}
