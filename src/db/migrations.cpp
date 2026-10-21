#include "db/db_manager.hpp"
#include "db/migrations.hpp"
#include <stdexcept>

void migrate_tracker(DbManager &db) {
  const int version = std::stoi(db.query("PRAGMA user_version")[0].at("user_version"));
  if (version > 1) throw std::runtime_error("Database schema is newer than this application");
  if (version >= 1) return;
  db.query("BEGIN IMMEDIATE");
  try {
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
    db.query("COMMIT");
  } catch (...) {
    db.query("ROLLBACK");
    throw;
  }
}
