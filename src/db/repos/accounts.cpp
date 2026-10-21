#include "db/repos/accounts.hpp"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>

namespace {
std::string hex(const unsigned char *bytes, size_t size) {
  std::ostringstream result;
  for (size_t i = 0; i < size; ++i) result << std::hex << std::setw(2) << std::setfill('0') << int(bytes[i]);
  return result.str();
}
std::string digest(const std::string &token) {
  unsigned char bytes[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<const unsigned char *>(token.data()), token.size(), bytes);
  return hex(bytes,sizeof(bytes));
}
}

std::string AccountRepo::create_demo_session(const std::string &label, const std::string &role) {
  if (label.empty() || label.size() > 100 || (role != "member" && role != "moderator"))
    throw std::invalid_argument("Invalid demo account");
  unsigned char bytes[32];
  if (RAND_bytes(bytes,sizeof(bytes)) != 1) throw std::runtime_error("Secure randomness unavailable");
  auto token = hex(bytes,sizeof(bytes));
  auto lock = db_->acquire_lock();
  if (db_->query("SELECT 1 FROM tracker_meta WHERE key='dataset' AND value='ausfencer-demo-v1'").empty())
    throw std::invalid_argument("Demo sessions require a seeded demo database");
  db_->query("BEGIN IMMEDIATE");
  try {
    db_->query("INSERT INTO accounts(label,role) VALUES(?,?)", {label,role});
    auto id = db_->query("SELECT last_insert_rowid() AS id")[0].at("id");
    db_->query("INSERT INTO signins VALUES('demo',?,?)", {id,id});
    db_->query("INSERT INTO sessions VALUES(?,?,unixepoch()+86400)", {digest(token),id});
    db_->query("COMMIT");
  } catch (...) { db_->query("ROLLBACK"); throw; }
  return token;
}

DbRow AccountRepo::authenticate(const std::string &token) {
  if (token.size() != 64) return {};
  auto lock = db_->acquire_lock();
  auto rows = db_->query(R"(SELECT a.id,a.label,a.role FROM accounts a JOIN sessions s ON s.account_id=a.id
    WHERE s.token_hash=? AND s.expires_at>unixepoch())", {digest(token)});
  return rows.empty() ? DbRow{} : rows[0];
}

void AccountRepo::logout(const std::string &token) {
  auto lock = db_->acquire_lock();
  db_->query("DELETE FROM sessions WHERE token_hash=?", {digest(token)});
}
