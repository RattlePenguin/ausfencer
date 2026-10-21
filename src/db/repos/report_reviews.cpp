#include "db/repos/results.hpp"
#include <crow/json.h>

void ResultRepo::reject_report(const std::string &token, int report, const std::string &reason) {
  auto actor=AccountRepo(db_).authenticate(token);
  if (actor.empty() || actor.at("role")!="moderator") throw std::domain_error("Reviewer required");
  if (reason.empty() || reason.size()>4000) throw std::invalid_argument("A review reason is required");
  auto lock=db_->acquire_lock(); db_->query("BEGIN IMMEDIATE");
  try {
    auto reports=db_->query("SELECT * FROM reports WHERE id=? AND state='pending'", {std::to_string(report)});
    if (reports.empty()) throw std::invalid_argument("Unknown pending report");
    auto id=reports[0].at("bout_id");
    auto before=db_->query("SELECT * FROM bouts WHERE id=?", {id})[0];
    db_->query("UPDATE reports SET state='rejected',reviewer=? WHERE id=?", {actor.at("id"),std::to_string(report)});
    db_->query(R"(UPDATE contributions SET state='rejected' WHERE bout_id=? AND state='conflict'
      AND ('Conflicting community submission: '||payload)=?)", {id,reports[0].at("detail")});
    db_->query(R"(UPDATE bouts SET disputed=EXISTS(SELECT 1 FROM reports WHERE bout_id=? AND state='pending'),
      revision=revision+1,updated_at=CURRENT_TIMESTAMP WHERE id=?)", {id,id});
    auto after=db_->query("SELECT * FROM bouts WHERE id=?", {id})[0];
    crow::json::wvalue prior,current;
    for (const auto &field : before) prior[field.first]=field.second;
    for (const auto &field : after) current[field.first]=field.second;
    db_->query("INSERT INTO revisions(bout_id,actor,reason,before_json,after_json) VALUES(?,?,?,?,?)",
      {id,actor.at("id"),"Rejected report "+std::to_string(report)+": "+reason,prior.dump(),current.dump()});
    db_->query("COMMIT");
  } catch (...) { db_->query("ROLLBACK"); throw; }
}
