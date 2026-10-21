#pragma once

#include "db/db_manager.hpp"
#include "models/Tracker.hpp"
#include <memory>

class TrackerRepo {
  std::shared_ptr<DbManager> db_;
  DbRows read_bouts(int person, const std::string &event, const TrackerFilter &filter, int bout = 0);
public:
  explicit TrackerRepo(std::shared_ptr<DbManager> db) : db_(std::move(db)) {}
  DbRows search(const std::string &query, int page = 1, int limit = 20);
  DbRows profile(int id);
  DbRows events();
  DbRows event_entries(const std::string &event);
  DbRows history(int id, const TrackerFilter &filter = {}) { return read_bouts(id,"",filter); }
  DbRows event_bouts(const std::string &id, const TrackerFilter &filter = {}) { return read_bouts(0,id,filter); }
  DbRows bout(int id) { return read_bouts(0,"",{},id); }
};
