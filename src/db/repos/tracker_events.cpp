#include "db/repos/tracker.hpp"

DbRows TrackerRepo::events() {
  auto lock = db_->acquire_lock();
  return db_->query(R"(SELECT e.*,t.name AS tournament,t.location,t.country,t.timezone
    FROM events e JOIN tournaments t ON t.id=e.tournament_id ORDER BY e.date DESC,e.id)");
}

DbRows TrackerRepo::event_entries(const std::string &event) {
  auto lock = db_->acquire_lock();
  return db_->query(R"(SELECT CASE WHEN p.visible=1 THEN p.fencer_id END AS id,
    CASE WHEN p.visible=1 THEN p.name ELSE 'Private fencer' END AS name,
    CASE WHEN p.visible=1 THEN en.source_name ELSE '' END AS source_name,
    CASE WHEN p.visible=1 THEN en.club ELSE '' END AS club,
    CASE WHEN p.visible=1 THEN en.state ELSE '' END AS state,
    CASE WHEN p.visible=1 THEN en.country ELSE '' END AS country,
    en.pool,en.seed,en.place,en.bye FROM entries en
    JOIN profiles p ON p.fencer_id=en.fencer_id WHERE en.event_id=?
    ORDER BY en.seed,en.fencer_id)", {event});
}

DbRows TrackerRepo::profile_events(int id, const TrackerFilter &filter) {
  std::string statement = R"(SELECT e.id,e.name,e.date,e.weapon,e.category,e.coverage,
    en.place,en.club,en.state,en.country FROM entries en JOIN events e ON e.id=en.event_id
    JOIN profiles p ON p.fencer_id=en.fencer_id WHERE en.fencer_id=? AND p.visible=1)";
  std::vector<std::string> parameters{std::to_string(id)};
  auto clause = [&](const std::string &sql, const std::string &value) {
    if (!value.empty()) { statement += " AND " + sql; parameters.push_back(value); }
  };
  clause("e.weapon=?",filter.weapon); clause("substr(e.date,1,4)=?",filter.year);
  clause("e.category=?",filter.category); clause("e.coverage=?",filter.coverage);
  clause("e.date>=?",filter.from); clause("e.date<=?",filter.to);
  auto lock = db_->acquire_lock();
  return db_->query(statement + " ORDER BY e.date DESC,e.id",parameters);
}
