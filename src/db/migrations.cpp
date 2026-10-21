#include "db/db_manager.hpp"
#include "db/migrations.hpp"
#include <stdexcept>

void migrate_tracker(DbManager &db) {
  const int version = std::stoi(db.query("PRAGMA user_version")[0].at("user_version"));
  if (version > 4) throw std::runtime_error("Database schema is newer than this application");
  if (version >= 4) return;
  db.query("BEGIN IMMEDIATE");
  try {
    if (version < 1) {
    db.query("CREATE TABLE tracker_meta(key TEXT PRIMARY KEY, value TEXT NOT NULL)");
    db.query(R"(CREATE TABLE profiles(
      fencer_id INTEGER PRIMARY KEY REFERENCES fencers(id), name TEXT NOT NULL,
      club TEXT NOT NULL, state TEXT NOT NULL, country TEXT NOT NULL,
      visible INTEGER NOT NULL DEFAULT 1 CHECK(visible IN (0,1)),
      guardian_required INTEGER NOT NULL DEFAULT 0 CHECK(guardian_required IN (0,1))))");
    db.query(R"(CREATE TABLE aliases(fencer_id INTEGER REFERENCES profiles(fencer_id),
      name TEXT NOT NULL, PRIMARY KEY(fencer_id,name)))");
    db.query(R"(CREATE TABLE external_ids(issuer TEXT NOT NULL, value TEXT NOT NULL,
      fencer_id INTEGER NOT NULL REFERENCES profiles(fencer_id), PRIMARY KEY(issuer,value)))");
    db.query(R"(CREATE TABLE tournaments(id TEXT PRIMARY KEY, name TEXT NOT NULL,
      country TEXT NOT NULL, location TEXT NOT NULL, timezone TEXT NOT NULL))");
    db.query(R"(CREATE TABLE events(id TEXT PRIMARY KEY,
      tournament_id TEXT NOT NULL REFERENCES tournaments(id), name TEXT NOT NULL,
      date TEXT NOT NULL, weapon TEXT NOT NULL CHECK(weapon IN ('epee','foil','sabre')),
      category TEXT NOT NULL, coverage TEXT NOT NULL
      CHECK(coverage IN ('complete','partial','results-only','unresolved'))))");
    db.query(R"(CREATE TABLE entries(event_id TEXT REFERENCES events(id),
      fencer_id INTEGER REFERENCES profiles(fencer_id), source_name TEXT NOT NULL,
      club TEXT NOT NULL, state TEXT NOT NULL, country TEXT NOT NULL,
      pool TEXT NOT NULL, seed INTEGER CHECK(seed>0), place INTEGER CHECK(place>0),
      bye INTEGER NOT NULL CHECK(bye IN (0,1)), PRIMARY KEY(event_id,fencer_id)))");
    db.query("PRAGMA user_version = 1");
    }
    if (version < 2) {
    db.query(R"(CREATE TABLE sources(id TEXT PRIMARY KEY NOT NULL, basis TEXT NOT NULL,
      acquired_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, parser_version TEXT NOT NULL,
      checksum TEXT NOT NULL))");
    db.query(R"(CREATE TABLE bouts(id TEXT PRIMARY KEY NOT NULL,
      event_id TEXT NOT NULL REFERENCES events(id), round TEXT NOT NULL, pool TEXT NOT NULL,
      left_id INTEGER NOT NULL, right_id INTEGER NOT NULL, left_score INTEGER, right_score INTEGER,
      winner_id INTEGER REFERENCES profiles(fencer_id), status TEXT NOT NULL
      CHECK(status IN ('completed','withdrawal','administrative','unfinished')),
      trust TEXT NOT NULL CHECK(trust IN ('synthetic','community','organiser')),
      disputed INTEGER NOT NULL DEFAULT 0 CHECK(disputed IN (0,1)),
      source_id TEXT NOT NULL REFERENCES sources(id), revision INTEGER NOT NULL DEFAULT 0,
      updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
      FOREIGN KEY(event_id,left_id) REFERENCES entries(event_id,fencer_id),
      FOREIGN KEY(event_id,right_id) REFERENCES entries(event_id,fencer_id),
      CHECK(left_id<>right_id), CHECK(winner_id IS NULL OR winner_id IN (left_id,right_id)),
      CHECK((left_score IS NULL AND right_score IS NULL) OR
        (left_score IS NOT NULL AND right_score IS NOT NULL AND
         left_score BETWEEN 0 AND 99 AND right_score BETWEEN 0 AND 99)),
      CHECK(status<>'completed' OR winner_id IS NOT NULL)))");
    db.query("CREATE INDEX bouts_left ON bouts(left_id,event_id)");
    db.query("CREATE INDEX bouts_right ON bouts(right_id,event_id)");
    db.query(R"(CREATE TABLE observations(source_id TEXT REFERENCES sources(id),
      source_key TEXT NOT NULL, bout_id TEXT NOT NULL REFERENCES bouts(id), payload TEXT NOT NULL,
      PRIMARY KEY(source_id,source_key)))");
    db.query(R"(CREATE TABLE revisions(id INTEGER PRIMARY KEY, bout_id TEXT NOT NULL REFERENCES bouts(id),
      actor TEXT NOT NULL, reason TEXT NOT NULL, before_json TEXT NOT NULL, after_json TEXT NOT NULL,
      created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP))");
    db.query(R"(CREATE TABLE imports(id INTEGER PRIMARY KEY, checksum TEXT NOT NULL,
      status TEXT NOT NULL CHECK(status IN ('preview','applied','failed')),
      detail TEXT NOT NULL, created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP))");
    db.query("PRAGMA user_version = 2");
    }
    if (version < 3) {
    db.query(R"(CREATE TABLE accounts(id INTEGER PRIMARY KEY, label TEXT NOT NULL,
      role TEXT NOT NULL CHECK(role IN ('member','moderator')),
      created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP))");
    db.query(R"(CREATE TABLE signins(provider TEXT NOT NULL, subject TEXT NOT NULL,
      account_id INTEGER NOT NULL REFERENCES accounts(id), PRIMARY KEY(provider,subject)))");
    db.query(R"(CREATE TABLE sessions(token_hash TEXT PRIMARY KEY NOT NULL,
      account_id INTEGER NOT NULL REFERENCES accounts(id), expires_at INTEGER NOT NULL))");
    db.query(R"(CREATE TABLE claims(id INTEGER PRIMARY KEY,
      account_id INTEGER NOT NULL REFERENCES accounts(id), fencer_id INTEGER NOT NULL REFERENCES profiles(fencer_id),
      kind TEXT NOT NULL CHECK(kind IN ('self','guardian')),
      state TEXT NOT NULL CHECK(state IN ('pending','approved','rejected','withdrawn')),
      evidence TEXT NOT NULL, consent_version TEXT NOT NULL,
      reviewer INTEGER REFERENCES accounts(id), created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
      updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, UNIQUE(account_id,fencer_id,kind)))");
    db.query(R"(CREATE TABLE privacy_requests(id INTEGER PRIMARY KEY,
      fencer_id INTEGER NOT NULL REFERENCES profiles(fencer_id), contact TEXT NOT NULL,
      evidence TEXT NOT NULL, state TEXT NOT NULL DEFAULT 'pending'
      CHECK(state IN ('pending','hidden','rejected')), created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP))");
    db.query(R"(CREATE TABLE contributions(id INTEGER PRIMARY KEY,
      account_id INTEGER NOT NULL REFERENCES accounts(id), operation_id TEXT NOT NULL,
      bout_id TEXT REFERENCES bouts(id), payload TEXT NOT NULL,
      state TEXT NOT NULL CHECK(state IN ('published','conflict','rejected')),
      created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, UNIQUE(account_id,operation_id)))");
    db.query(R"(CREATE TABLE confirmations(contribution_id INTEGER REFERENCES contributions(id),
      account_id INTEGER REFERENCES accounts(id), fencer_id INTEGER REFERENCES profiles(fencer_id),
      created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, PRIMARY KEY(contribution_id,fencer_id)))");
    db.query("PRAGMA user_version = 3");
    }
    db.query(R"(CREATE TABLE profile_audit(id INTEGER PRIMARY KEY,
      fencer_id INTEGER NOT NULL REFERENCES profiles(fencer_id), actor INTEGER REFERENCES accounts(id),
      reason TEXT NOT NULL, before_visible INTEGER NOT NULL, after_visible INTEGER NOT NULL,
      created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP))");
    db.query("PRAGMA user_version = 4");
    db.query("COMMIT");
  } catch (...) {
    db.query("ROLLBACK");
    throw;
  }
}
