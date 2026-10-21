#include "db/repos/tracker.hpp"
#include <regex>
#include <stdexcept>

DbRows TrackerRepo::read_bouts(int person, const std::string &event, const TrackerFilter &filter, int bout) {
  if (person < 0 || bout < 0 || (!filter.year.empty() &&
      !std::regex_match(filter.year,std::regex("[0-9]{4}"))))
    throw std::invalid_argument("Invalid bout filter");
  auto id = std::to_string(person);
  std::vector<std::string> parameters{id,id,id,id,id,id,id};
  std::string statement = R"(SELECT b.rowid AS id,b.event_id,e.name AS event_name,
    e.date,e.weapon,e.category,e.coverage,b.round,b.pool,b.status,b.trust,b.disputed,
    b.revision,b.updated_at,s.basis AS provenance,s.acquired_at,
    CASE WHEN l.visible=1 THEN b.left_id END AS left_id,
    CASE WHEN r.visible=1 THEN b.right_id END AS right_id,
    CASE WHEN l.visible=1 THEN l.name ELSE 'Private fencer' END AS left_name,
    CASE WHEN r.visible=1 THEN r.name ELSE 'Private fencer' END AS right_name,
    b.left_score,b.right_score,
    le.seed AS left_seed,re.seed AS right_seed,
    CASE WHEN b.winner_id=b.left_id THEN 'left' WHEN b.winner_id=b.right_id THEN 'right' END AS winner_side,
    CASE WHEN b.left_id=? THEN b.left_score ELSE b.right_score END AS scored,
    CASE WHEN b.left_id=? THEN b.right_score ELSE b.left_score END AS received,
    CASE WHEN b.winner_id=? THEN 'win' ELSE 'loss' END AS result
    FROM bouts b JOIN events e ON e.id=b.event_id JOIN sources s ON s.id=b.source_id
    JOIN profiles l ON l.fencer_id=b.left_id JOIN profiles r ON r.fencer_id=b.right_id
    JOIN entries le ON le.event_id=b.event_id AND le.fencer_id=b.left_id
    JOIN entries re ON re.event_id=b.event_id AND re.fencer_id=b.right_id
    WHERE (?='0' OR ((b.left_id=? OR b.right_id=?) AND
      EXISTS(SELECT 1 FROM profiles WHERE fencer_id=? AND visible=1))))";
  auto clause = [&](const std::string &sql, const std::string &value) {
    if (!value.empty()) { statement += " AND " + sql; parameters.push_back(value); }
  };
  clause("b.event_id=?",event);
  if (bout) clause("b.rowid=?",std::to_string(bout));
  clause("e.weapon=?",filter.weapon); clause("substr(e.date,1,4)=?",filter.year);
  clause("b.trust=?",filter.trust); clause("e.category=?",filter.category);
  clause("e.coverage=?",filter.coverage);
  clause("e.date>=?",filter.from); clause("e.date<=?",filter.to);
  if (filter.stage == "de") statement += " AND b.round<>'pool'";
  else clause("b.round=?",filter.stage);
  if (filter.opponent) {
    statement += " AND ((b.left_id=? AND l.visible=1) OR (b.right_id=? AND r.visible=1))";
    parameters.push_back(std::to_string(filter.opponent)); parameters.push_back(std::to_string(filter.opponent));
  }
  statement += " ORDER BY e.date DESC,b.rowid";
  auto lock = db_->acquire_lock();
  return db_->query(statement,parameters);
}
