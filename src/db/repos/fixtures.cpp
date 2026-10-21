#include "db/repos/fixtures.hpp"
#include "services/fixtures.hpp"

void FixtureRepo::write_context(const FixtureData &data) {
  for (const auto &p : data.fencers) {
    auto id = std::to_string(p.id);
    db_->query("INSERT INTO fencers(id,first_name,last_name,birth_year) VALUES(?,?,'',0) ON CONFLICT(id) DO NOTHING",
               {id,p.name});
    db_->query("INSERT INTO profiles VALUES(?,?,?,?,?,?,?) ON CONFLICT(fencer_id) DO NOTHING",
               {id,p.name,p.club,p.state,p.country,p.guardian_required ? "0" : "1",p.guardian_required ? "1" : "0"});
    if (!p.alias.empty()) db_->query("INSERT INTO aliases VALUES(?,?) ON CONFLICT DO NOTHING", {id,p.alias});
    db_->query("INSERT INTO external_ids VALUES('ausfencer-demo-v1',?,?) ON CONFLICT DO NOTHING", {p.external_id,id});
  }
  for (const auto &e : data.events) {
    db_->query("INSERT INTO tournaments VALUES(?,?,'AU','Fictional demo venue',?) ON CONFLICT DO NOTHING",
               {e.tournament,e.tournament,e.timezone});
    db_->query("INSERT INTO events VALUES(?,?,?,?,?,?,?) ON CONFLICT DO NOTHING",
               {e.id,e.tournament,e.name,e.date,e.weapon,e.category,e.coverage});
  }
  for (const auto &e : data.entries)
    db_->query(R"(INSERT INTO entries SELECT ?,fencer_id,?,club,state,country,?,?,?,?
      FROM profiles WHERE fencer_id=? ON CONFLICT DO NOTHING)",
      {e.event,e.source_name,e.pool,std::to_string(e.seed),std::to_string(e.place),
       e.bye ? "1" : "0",std::to_string(e.fencer)});
}

void FixtureRepo::import_fixture(const FixtureData &data, bool approved, bool reset) {
  validate_fixture(data);
  auto lock = db_->acquire_lock();
  db_->query("BEGIN IMMEDIATE");
  try {
    auto marker = db_->query("SELECT value FROM tracker_meta WHERE key='dataset'");
    if (marker.empty() && !db_->query("SELECT id FROM fencers LIMIT 1").empty())
      throw std::invalid_argument("Refusing to mix demo data with existing records");
    if (!marker.empty() && marker[0].at("value") != "ausfencer-demo-v1")
      throw std::invalid_argument("This database is not the demo dataset");
    if (reset) {
      if (marker.empty()) throw std::invalid_argument("Reset requires an existing marked demo database");
      for (const auto *table : {"confirmations","contributions","revisions","observations","bouts",
           "entries","events","tournaments","sources","profile_audit","privacy_requests",
           "claims","sessions","signins","accounts","external_ids","aliases","profiles","fencers","tracker_meta"})
        db_->query(std::string("DELETE FROM ") + table);
    }
    db_->query("INSERT INTO tracker_meta VALUES('dataset','ausfencer-demo-v1') ON CONFLICT DO NOTHING");
    write_context(data);
    write_bouts(data);
    if (approved) db_->query("INSERT INTO imports(checksum,status,detail) VALUES(?,'applied',?)",
                             {data.checksum,reset ? "demo reset" : "fixture import"});
    db_->query(approved ? "COMMIT" : "ROLLBACK");
    if (!approved) db_->query("INSERT INTO imports(checksum,status,detail) VALUES(?,'preview','fixture import')",
                              {data.checksum});
  } catch (const std::exception &e) {
    if (!db_->get_storage().get_autocommit()) db_->query("ROLLBACK");
    db_->query("INSERT INTO imports(checksum,status,detail) VALUES(?,'failed',?)", {data.checksum,e.what()});
    throw;
  }
}
