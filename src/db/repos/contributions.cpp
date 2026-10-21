#include "db/repos/results.hpp"
#include <crow/json.h>
#include <set>

int ResultRepo::submit(const std::string &token, const std::string &operation, CompetitionBout b,
                        const std::string &evidence) {
  auto actor = AccountRepo(db_).authenticate(token);
  if (actor.empty()) throw std::domain_error("Sign in required");
  if (operation.empty() || operation.size()>100 || evidence.size()>4000 || b.left==b.right ||
      b.status!="completed" || !b.winner || (*b.winner!=b.left && *b.winner!=b.right) ||
      b.left_score.has_value()!=b.right_score.has_value() ||
      (b.left_score && (*b.left_score<0 || *b.left_score>99 || *b.right_score<0 || *b.right_score>99)) ||
      !std::set<std::string>{"pool","R16","QF","SF","F"}.count(b.stage))
    throw std::invalid_argument("Invalid community result");
  if (b.left>b.right) { std::swap(b.left,b.right); std::swap(b.left_score,b.right_score); }
  auto number = [](std::optional<int> value) { return value ? std::to_string(*value) : ""; };
  crow::json::wvalue document;
  document["event"]=b.event; document["round"]=b.stage; document["pool"]=b.pool;
  document["left"]=b.left; document["right"]=b.right; document["winner"]=*b.winner;
  document["left_score"]=number(b.left_score); document["right_score"]=number(b.right_score); document["evidence"]=evidence;
  auto payload=document.dump(), person=actor.at("id");
  auto lock=db_->acquire_lock();
  db_->query("BEGIN IMMEDIATE");
  try {
    auto repeat=db_->query("SELECT id,payload FROM contributions WHERE account_id=? AND operation_id=?", {person,operation});
    if (!repeat.empty()) {
      if (repeat[0].at("payload")!=payload) throw std::domain_error("Operation ID already used for different facts");
      db_->query("COMMIT"); return std::stoi(repeat[0].at("id"));
    }
    auto recent = db_->query("SELECT count(*) AS total FROM contributions WHERE account_id=? AND created_at>=datetime('now','-1 hour')", {person});
    if (std::stoi(recent[0].at("total")) >= 30) throw std::domain_error("Hourly contribution limit reached");
    auto entries=db_->query("SELECT pool FROM entries WHERE event_id=? AND fencer_id IN (?,?)",
      {b.event,std::to_string(b.left),std::to_string(b.right)});
    if (entries.size()!=2 || (b.stage=="pool" && (b.pool.empty() || entries[0].at("pool")!=b.pool || entries[1].at("pool")!=b.pool)))
      throw std::invalid_argument("Participants must have matching event/poule entries");
    auto found=db_->query("SELECT * FROM bouts WHERE event_id=? AND round=? AND pool=? AND left_id=? AND right_id=?",
      {b.event,b.stage,b.pool,std::to_string(b.left),std::to_string(b.right)});
    auto id="community:"+person+":"+operation, state=std::string("published"), source="community:"+person;
    db_->query("INSERT INTO sources(id,basis,parser_version,checksum) VALUES(?,'community-submitted','1','user-supplied') ON CONFLICT DO NOTHING", {source});
    if (found.empty()) {
      db_->query(R"(INSERT INTO bouts(id,event_id,round,pool,left_id,right_id,left_score,right_score,winner_id,status,trust,source_id)
        VALUES(?,?,?,?,?,?,NULLIF(?,''),NULLIF(?,''),?,'completed','community',?))",
        {id,b.event,b.stage,b.pool,std::to_string(b.left),std::to_string(b.right),number(b.left_score),number(b.right_score),number(b.winner),source});
    } else {
      const auto &old=found[0]; id=old.at("id");
      if (found.size()!=1 || (old.count("left_score") ? old.at("left_score") : "")!=number(b.left_score) ||
          (old.count("right_score") ? old.at("right_score") : "")!=number(b.right_score) ||
          !old.count("winner_id") || old.at("winner_id")!=number(b.winner) || old.at("status")!="completed") state="conflict";
    }
    db_->query("INSERT INTO contributions(account_id,operation_id,bout_id,payload,state) VALUES(?,?,?,?,?)", {person,operation,id,payload,state});
    auto contribution=std::stoi(db_->query("SELECT last_insert_rowid() AS id")[0].at("id"));
    db_->query("INSERT INTO observations VALUES(?,?,?,?)", {source,operation,id,payload});
    if (state=="conflict") {
      db_->query("INSERT INTO reports(bout_id,account_id,kind,detail) VALUES(?,?,'score',?)", {id,person,"Conflicting community submission: "+payload});
      db_->query("UPDATE bouts SET disputed=1 WHERE id=?", {id});
    }
    db_->query("COMMIT"); return contribution;
  } catch (...) { db_->query("ROLLBACK"); throw; }
}
