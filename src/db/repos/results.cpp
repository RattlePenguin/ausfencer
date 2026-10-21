#include "db/repos/results.hpp"

int ResultRepo::report(const std::string &token, int bout, const std::string &kind, const std::string &detail) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty()) throw std::domain_error("Sign in required");
  if ((kind != "score" && kind != "identity") || detail.empty() || detail.size() > 4000)
    throw std::invalid_argument("Report needs a kind and supporting detail");
  auto lock = db_->acquire_lock();
  auto rows = db_->query("SELECT id FROM bouts WHERE rowid=?", {std::to_string(bout)});
  if (rows.empty()) throw std::invalid_argument("Unknown bout");
  db_->query("BEGIN IMMEDIATE");
  try {
    db_->query("INSERT INTO reports(bout_id,account_id,kind,detail) VALUES(?,?,?,?)",
               {rows[0].at("id"),actor.at("id"),kind,detail});
    auto id = std::stoi(db_->query("SELECT last_insert_rowid() AS id")[0].at("id"));
    db_->query("UPDATE bouts SET disputed=1 WHERE rowid=?", {std::to_string(bout)});
    db_->query("COMMIT");
    return id;
  } catch (...) { db_->query("ROLLBACK"); throw; }
}

DbRows ResultRepo::reports(const std::string &token, bool queue) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty() || (queue && actor.at("role") != "moderator")) throw std::domain_error("Permission required");
  auto lock = db_->acquire_lock();
  return db_->query(R"(SELECT r.id,b.rowid AS bout,r.kind,r.detail,r.state,r.created_at
    FROM reports r JOIN bouts b ON b.id=r.bout_id WHERE )" +
    (queue ? std::string("r.state='pending'") : std::string("r.account_id=?")) + " ORDER BY r.id",
    queue ? std::vector<std::string>{} : std::vector<std::string>{actor.at("id")});
}

DbRows ResultRepo::contributions(const std::string &token) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty()) throw std::domain_error("Sign in required");
  auto lock = db_->acquire_lock();
  return db_->query(R"(SELECT c.id,b.rowid AS bout,c.state,c.created_at,b.trust,b.disputed
    FROM contributions c JOIN bouts b ON b.id=c.bout_id WHERE c.account_id=? ORDER BY c.id DESC)", {actor.at("id")});
}
