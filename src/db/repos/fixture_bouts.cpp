#include "db/repos/fixtures.hpp"
#include <crow/json.h>

void FixtureRepo::write_bouts(const FixtureData &data) {
  const std::string source = "demo:" + data.checksum;
  db_->query("INSERT INTO sources(id,basis,parser_version,checksum) VALUES(?,'synthetic','demo-v1',?) ON CONFLICT DO NOTHING",
             {source,data.checksum});
  auto number = [](std::optional<int> value) { return value ? std::to_string(*value) : ""; };
  for (auto b : data.bouts) {
    if (b.left > b.right) {
      std::swap(b.left,b.right);
      std::swap(b.left_score,b.right_score);
    }
    const auto id = "ausfencer-demo-v1:" + b.event + ":" + b.id;
    crow::json::wvalue evidence;
    evidence["event"] = b.event; evidence["round"] = b.stage; evidence["pool"] = b.pool;
    evidence["left"] = b.left; evidence["right"] = b.right;
    evidence["left_score"] = number(b.left_score); evidence["right_score"] = number(b.right_score);
    evidence["winner"] = number(b.winner); evidence["status"] = b.status; evidence["trust"] = b.trust;
    auto payload = evidence.dump();
    auto previous = db_->query("SELECT payload FROM observations WHERE bout_id=? ORDER BY rowid LIMIT 1", {id});
    if (!previous.empty() && previous[0].at("payload") != payload)
      throw std::invalid_argument("Fixture conflicts with an accepted observation; review required: " + b.id);
    if (previous.empty())
      db_->query(R"(INSERT INTO bouts(id,event_id,round,pool,left_id,right_id,left_score,
        right_score,winner_id,status,trust,source_id) VALUES(?,?,?,?,?,?,NULLIF(?,''),
        NULLIF(?,''),NULLIF(?,''),?,?,?))",
        {id,b.event,b.stage,b.pool,std::to_string(b.left),std::to_string(b.right),
         number(b.left_score),number(b.right_score),number(b.winner),b.status,b.trust,source});
    db_->query("INSERT INTO observations VALUES(?,?,?,?) ON CONFLICT DO NOTHING", {source,id,id,payload});
  }
}
