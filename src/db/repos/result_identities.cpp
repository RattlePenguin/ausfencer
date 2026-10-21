#include "db/repos/results.hpp"
#include <crow/json.h>

void ResultRepo::correct_identity(const std::string &token, int bout, int revision,
                                  const std::string &side, int fencer, const std::string &reason) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty() || actor.at("role") != "moderator") throw std::domain_error("Reviewer required");
  if ((side != "left" && side != "right") || reason.empty() || reason.size() > 4000)
    throw std::invalid_argument("Identity correction requires a side and reason");
  auto lock = db_->acquire_lock();
  db_->query("BEGIN IMMEDIATE");
  try {
    auto rows = db_->query("SELECT * FROM bouts WHERE rowid=?", {std::to_string(bout)});
    if (rows.empty()) throw std::invalid_argument("Unknown bout");
    auto before = rows[0];
    if (std::stoi(before.at("revision")) != revision) throw std::domain_error("Stale bout revision");
    auto target = std::to_string(fencer), old = before.at(side + "_id");
    if (target == before.at("left_id") || target == before.at("right_id"))
      throw std::invalid_argument("Choose a different participant");
    if (db_->query("SELECT 1 FROM entries WHERE event_id=? AND fencer_id=? AND (?<>'pool' OR pool=?)",
      {before.at("event_id"),target,before.at("round"),before.at("pool")}).empty())
      throw std::invalid_argument("Corrected participant must be an event entrant");
    auto left = side == "left" ? target : before.at("left_id");
    auto right = side == "right" ? target : before.at("right_id");
    auto ls = before.count("left_score") ? before.at("left_score") : "";
    auto rs = before.count("right_score") ? before.at("right_score") : "";
    auto winner = before.count("winner_id") ? before.at("winner_id") : "";
    if (winner == old) winner = target;
    if (std::stoi(left) > std::stoi(right)) { std::swap(left,right); std::swap(ls,rs); }
    if (!db_->query("SELECT 1 FROM bouts WHERE event_id=? AND round=? AND pool=? AND left_id=? AND right_id=? AND rowid<>?",
      {before.at("event_id"),before.at("round"),before.at("pool"),left,right,std::to_string(bout)}).empty())
      throw std::domain_error("Identity correction would duplicate another bout");
    db_->query(R"(UPDATE bouts SET left_id=?,right_id=?,left_score=NULLIF(?,''),right_score=NULLIF(?,''),
      winner_id=NULLIF(?,''),revision=revision+1,updated_at=CURRENT_TIMESTAMP WHERE rowid=?)",
      {left,right,ls,rs,winner,std::to_string(bout)});
    db_->query("UPDATE reports SET state='resolved',reviewer=? WHERE bout_id=? AND kind='identity' AND state='pending'", {actor.at("id"),before.at("id")});
    db_->query("UPDATE bouts SET disputed=EXISTS(SELECT 1 FROM reports WHERE bout_id=? AND state='pending') WHERE rowid=?", {before.at("id"),std::to_string(bout)});
    db_->query("UPDATE events SET coverage='partial' WHERE id=?", {before.at("event_id")});
    crow::json::wvalue prior, after;
    for (const auto &field : before) prior[field.first] = field.second;
    auto current = db_->query("SELECT * FROM bouts WHERE rowid=?", {std::to_string(bout)});
    for (const auto &field : current[0]) after[field.first] = field.second;
    db_->query("INSERT INTO revisions(bout_id,actor,reason,before_json,after_json) VALUES(?,?,?,?,?)",
               {before.at("id"),actor.at("id"),reason,prior.dump(),after.dump()});
    db_->query("COMMIT");
  } catch (...) { db_->query("ROLLBACK"); throw; }
}
