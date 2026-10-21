#include "db/repos/results.hpp"
#include <crow/json.h>
#include <set>

void ResultRepo::correct(const std::string &token, int bout, int revision, std::optional<int> left,
                         std::optional<int> right, const std::string &winner_side,
                         const std::string &status, const std::string &reason) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty() || actor.at("role") != "moderator") throw std::domain_error("Reviewer required");
  if (reason.empty() || reason.size() > 4000 || left.has_value() != right.has_value() ||
      (left && (*left < 0 || *left > 99 || *right < 0 || *right > 99)) ||
      !std::set<std::string>{"completed","unfinished","administrative","withdrawal"}.count(status) ||
      !std::set<std::string>{"left","right","none"}.count(winner_side) ||
      (status == "completed" && winner_side == "none"))
    throw std::invalid_argument("Invalid correction");
  auto lock = db_->acquire_lock();
  db_->query("BEGIN IMMEDIATE");
  try {
    auto rows = db_->query("SELECT * FROM bouts WHERE rowid=?", {std::to_string(bout)});
    if (rows.empty()) throw std::invalid_argument("Unknown bout");
    auto before = rows[0];
    if (std::stoi(before.at("revision")) != revision) throw std::domain_error("Stale bout revision; reload before correcting");
    auto number = [](std::optional<int> value) { return value ? std::to_string(*value) : ""; };
    auto winner = winner_side == "none" ? "" : before.at(winner_side == "left" ? "left_id" : "right_id");
    db_->query(R"(UPDATE bouts SET left_score=NULLIF(?,''),right_score=NULLIF(?,''),winner_id=NULLIF(?,''),
      status=?,revision=revision+1,updated_at=CURRENT_TIMESTAMP WHERE rowid=?)",
      {number(left),number(right),winner,status,std::to_string(bout)});
    db_->query("UPDATE reports SET state='resolved',reviewer=? WHERE bout_id=? AND kind='score' AND state='pending'",
               {actor.at("id"),before.at("id")});
    db_->query("UPDATE bouts SET disputed=EXISTS(SELECT 1 FROM reports WHERE bout_id=? AND state='pending') WHERE rowid=?",
               {before.at("id"),std::to_string(bout)});
    auto after = db_->query("SELECT * FROM bouts WHERE rowid=?", {std::to_string(bout)})[0];
    auto json = [](const DbRow &row) {
      crow::json::wvalue result;
      for (const auto &field : row) result[field.first] = field.second;
      return result.dump();
    };
    db_->query("INSERT INTO revisions(bout_id,actor,reason,before_json,after_json) VALUES(?,?,?,?,?)",
               {before.at("id"),actor.at("id"),reason,json(before),json(after)});
    db_->query("COMMIT");
  } catch (...) { db_->query("ROLLBACK"); throw; }
}
