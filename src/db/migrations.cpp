#include "db/db_manager.hpp"
#include "db/migrations.hpp"
#include <stdexcept>

void migrate_tracker(DbManager &db) {
  const int version = std::stoi(db.query("PRAGMA user_version")[0].at("user_version"));
  if (version > 2) throw std::runtime_error("Database schema is newer than this application");
  if (version >= 2) return;
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
    db.query("COMMIT");
  } catch (...) {
    db.query("ROLLBACK");
    throw;
  }
}
