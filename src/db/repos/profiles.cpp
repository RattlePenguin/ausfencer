#include "db/repos/profiles.hpp"

int ProfileRepo::request_claim(const std::string &token, int fencer, const std::string &kind,
                                const std::string &evidence, const std::string &consent) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty()) throw std::domain_error("Sign in required");
  if ((kind != "self" && kind != "guardian") || evidence.empty() || evidence.size() > 4000 ||
      (kind == "guardian" && consent != "demo-v1"))
    throw std::invalid_argument("Claim needs evidence and guardian consent where applicable");
  auto lock = db_->acquire_lock();
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
