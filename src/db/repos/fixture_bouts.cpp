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
    const auto key = "ausfencer-demo-v1:" + b.event + ":" + b.id;
    auto id = key;
    crow::json::wvalue evidence;
    evidence["event"] = b.event; evidence["round"] = b.stage; evidence["pool"] = b.pool;
    evidence["left"] = b.left; evidence["right"] = b.right;
    evidence["left_score"] = number(b.left_score); evidence["right_score"] = number(b.right_score);
    evidence["winner"] = number(b.winner); evidence["status"] = b.status; evidence["trust"] = b.trust;
    auto payload = evidence.dump();
    auto previous = db_->query(R"(SELECT o.bout_id,o.payload FROM observations o
      JOIN sources s ON s.id=o.source_id WHERE o.source_key=? AND s.basis='synthetic' ORDER BY o.rowid LIMIT 1)", {key});
    bool insert = previous.empty();
    if (!previous.empty()) id = previous[0].at("bout_id");
    else {
      auto matches = db_->query("SELECT * FROM bouts WHERE event_id=? AND round=? AND pool=? AND left_id=? AND right_id=?",
        {b.event,b.stage,b.pool,std::to_string(b.left),std::to_string(b.right)});
      if (!matches.empty()) {
        const auto &old = matches[0];
        if (matches.size()!=1 || (old.count("left_score") ? old.at("left_score") : "")!=number(b.left_score) ||
            (old.count("right_score") ? old.at("right_score") : "")!=number(b.right_score) ||
            (old.count("winner_id") ? old.at("winner_id") : "")!=number(b.winner) || old.at("status")!=b.status)
          throw std::invalid_argument("Fixture conflicts with a canonical community result; review required");
        id = old.at("id"); insert = false;
      }
    }
    if (!previous.empty() && previous[0].at("payload") != payload)
      throw std::invalid_argument("Fixture conflicts with an accepted observation; review required: " + b.id);
    if (insert)
      db_->query(R"(INSERT INTO bouts(id,event_id,round,pool,left_id,right_id,left_score,
        right_score,winner_id,status,trust,source_id) VALUES(?,?,?,?,?,?,NULLIF(?,''),
        NULLIF(?,''),NULLIF(?,''),?,?,?))",
        {id,b.event,b.stage,b.pool,std::to_string(b.left),std::to_string(b.right),
         number(b.left_score),number(b.right_score),number(b.winner),b.status,b.trust,source});
    db_->query("INSERT INTO observations VALUES(?,?,?,?) ON CONFLICT DO NOTHING", {source,key,id,payload});
  }
}
