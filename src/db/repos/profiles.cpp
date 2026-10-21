#include "db/repos/profiles.hpp"

int ProfileRepo::request_claim(const std::string &token, int fencer, const std::string &kind,
                                const std::string &evidence, const std::string &consent) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty()) throw std::domain_error("Sign in required");
  if ((kind != "self" && kind != "guardian") || evidence.empty() || evidence.size() > 4000 ||
      (kind == "guardian" && consent != "demo-v1"))
    throw std::invalid_argument("Claim needs evidence and guardian consent where applicable");
  auto lock = db_->acquire_lock();
  auto existing = db_->query("SELECT 1 FROM claims WHERE account_id=? AND fencer_id=? AND kind=?",
                             {actor.at("id"),std::to_string(fencer),kind});
  if (!existing.empty()) throw std::domain_error("Claim already exists; follow its review status");
  auto recent = db_->query("SELECT count(*) AS total FROM claims WHERE account_id=? AND created_at>=datetime('now','-1 hour')", {actor.at("id")});
  if (std::stoi(recent[0].at("total")) >= 10) throw std::domain_error("Hourly claim limit reached");
  auto profile = db_->query("SELECT guardian_required FROM profiles WHERE fencer_id=?", {std::to_string(fencer)});
  if (profile.empty()) throw std::invalid_argument("Unknown profile");
  if (kind == "self" && profile[0].at("guardian_required") == "1")
    throw std::domain_error("This demo profile requires guardian management");
  db_->query(R"(INSERT INTO claims(account_id,fencer_id,kind,state,evidence,consent_version)
    VALUES(?,?,?,'pending',?,?))", {actor.at("id"),std::to_string(fencer),kind,evidence,consent});
  return std::stoi(db_->query("SELECT last_insert_rowid() AS id")[0].at("id"));
}

void ProfileRepo::review_claim(const std::string &token, int claim, bool approved) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty() || actor.at("role") != "moderator") throw std::domain_error("Reviewer required");
  auto lock = db_->acquire_lock();
  auto rows = db_->query("SELECT * FROM claims WHERE id=? AND state='pending'", {std::to_string(claim)});
  if (rows.empty()) throw std::invalid_argument("Unknown pending claim");
  const auto &row = rows[0];
  if (actor.at("id") == row.at("account_id")) throw std::domain_error("Independent review required");
  if (approved && row.at("kind") == "self" && !db_->query(
      "SELECT 1 FROM claims WHERE fencer_id=? AND kind='self' AND state='approved'", {row.at("fencer_id")}).empty())
    throw std::domain_error("Conflicting self claim needs review");
  db_->query("UPDATE claims SET state=?,reviewer=?,updated_at=CURRENT_TIMESTAMP WHERE id=?",
             {approved ? "approved" : "rejected",actor.at("id"),std::to_string(claim)});
}

void ProfileRepo::change_visibility(const std::string &token, int fencer, bool visible, bool withdraw) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty()) throw std::domain_error("Sign in required");
  auto lock = db_->acquire_lock();
  db_->query("BEGIN IMMEDIATE");
  try {
    auto id = std::to_string(fencer);
    auto grants = db_->query(R"(SELECT 1 FROM claims WHERE account_id=? AND fencer_id=?
      AND state='approved' AND (kind='self' OR consent_version='demo-v1'))", {actor.at("id"),id});
    if (grants.empty()) throw std::domain_error("Approved profile management grant required");
    db_->query(R"(INSERT INTO profile_audit(fencer_id,actor,reason,before_visible,after_visible)
      SELECT fencer_id,?,?,visible,? FROM profiles WHERE fencer_id=?)",
      {actor.at("id"),withdraw ? "consent withdrawn" : "visibility changed",visible ? "1" : "0",id});
    db_->query("UPDATE profiles SET visible=? WHERE fencer_id=?", {visible ? "1" : "0",id});
    if (withdraw) db_->query("UPDATE claims SET state='withdrawn',updated_at=CURRENT_TIMESTAMP WHERE account_id=? AND fencer_id=?",
                            {actor.at("id"),id});
    db_->query("COMMIT");
  } catch (...) { db_->query("ROLLBACK"); throw; }
}

DbRows ProfileRepo::claims(const std::string &token) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty()) throw std::domain_error("Sign in required");
  auto lock = db_->acquire_lock();
  return db_->query(R"(SELECT c.id,c.fencer_id,c.kind,c.state,c.consent_version,c.updated_at,
    CASE WHEN c.state='approved' THEN p.name ELSE 'Pending profile' END AS name,p.visible
    FROM claims c JOIN profiles p ON p.fencer_id=c.fencer_id WHERE c.account_id=? ORDER BY c.id)", {actor.at("id")});
}
