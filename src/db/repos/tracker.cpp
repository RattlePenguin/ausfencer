#include "db/repos/tracker.hpp"
#include "services/names.hpp"
#include <sstream>
#include <stdexcept>

DbRows TrackerRepo::search(const std::string &query, int page, int limit) {
  if (query.size() > 200 || page < 1 || page > 1000000 || limit < 1 || limit > 100)
    throw std::invalid_argument("Invalid search pagination or query length");
  std::vector<std::string> tokens;
  std::istringstream words(fold_name(query));
  for (std::string word; words >> word;) tokens.push_back(word);
  auto lock = db_->acquire_lock();
  auto rows = db_->query(R"(SELECT p.fencer_id AS id,p.name,p.club,p.state,p.country,
    COALESCE((SELECT group_concat(a.name,' ') FROM aliases a WHERE a.fencer_id=p.fencer_id),'') AS aliases
    FROM profiles p WHERE p.visible=1 ORDER BY p.name,p.fencer_id)");
  DbRows result;
  size_t skipped = 0, offset = static_cast<size_t>(page - 1) * limit;
  for (auto &row : rows) {
    auto names = fold_name(row.at("name") + " " + row.at("aliases"));
    bool matches = true;
    for (const auto &token : tokens) if (names.find(token) == std::string::npos) matches = false;
    if (!matches || skipped++ < offset) continue;
    row.erase("aliases"); result.push_back(std::move(row));
    if (result.size() == static_cast<size_t>(limit)) break;
  }
  return result;
}

DbRows TrackerRepo::profile(int id) {
  auto lock = db_->acquire_lock();
  return db_->query("SELECT fencer_id AS id,name,club,state,country FROM profiles WHERE fencer_id=? AND visible=1",
                     {std::to_string(id)});
}
