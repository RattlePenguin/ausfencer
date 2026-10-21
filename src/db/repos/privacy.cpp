#include "db/repos/profiles.hpp"

int ProfileRepo::request_privacy(int fencer, const std::string &contact, const std::string &evidence) {
  if (contact.empty() || contact.size() > 300 || evidence.empty() || evidence.size() > 4000)
    throw std::invalid_argument("Contact and supporting context required");
  auto lock = db_->acquire_lock();
  auto pending = db_->query("SELECT count(*) AS total FROM privacy_requests WHERE fencer_id=? AND contact=? AND state='pending'",
                            {std::to_string(fencer),contact});
  if (std::stoi(pending[0].at("total")) >= 3) throw std::domain_error("A privacy request is already awaiting review");
  db_->query("INSERT INTO privacy_requests(fencer_id,contact,evidence) VALUES(?,?,?)",
             {std::to_string(fencer),contact,evidence});
  return std::stoi(db_->query("SELECT last_insert_rowid() AS id")[0].at("id"));
}

void ProfileRepo::review_privacy(const std::string &token, int request, bool hide) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty() || actor.at("role") != "moderator") throw std::domain_error("Reviewer required");
  auto lock = db_->acquire_lock();
  db_->query("BEGIN IMMEDIATE");
  try {
    auto rows = db_->query("SELECT fencer_id FROM privacy_requests WHERE id=? AND state='pending'", {std::to_string(request)});
    if (rows.empty()) throw std::invalid_argument("Unknown pending privacy request");
    auto id = rows[0].at("fencer_id");
    if (hide) {
      db_->query(R"(INSERT INTO profile_audit(fencer_id,actor,reason,before_visible,after_visible)
        SELECT fencer_id,?,'privacy request',visible,0 FROM profiles WHERE fencer_id=?)", {actor.at("id"),id});
      db_->query("UPDATE profiles SET visible=0 WHERE fencer_id=?", {id});
    }
    db_->query("UPDATE privacy_requests SET state=? WHERE id=?", {hide ? "hidden" : "rejected",std::to_string(request)});
    db_->query("COMMIT");
  } catch (...) { db_->query("ROLLBACK"); throw; }
}

DbRows ProfileRepo::review_queue(const std::string &token) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty() || actor.at("role") != "moderator") throw std::domain_error("Reviewer required");
  auto lock = db_->acquire_lock();
  return db_->query(R"(SELECT id,'claim' AS type,fencer_id,kind,evidence,'' AS contact FROM claims WHERE state='pending'
    UNION ALL SELECT id,'privacy',fencer_id,'privacy',evidence,contact FROM privacy_requests WHERE state='pending')");
}
