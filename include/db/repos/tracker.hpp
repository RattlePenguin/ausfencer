#pragma once

#include "db/db_manager.hpp"
#include "models/Tracker.hpp"
#include <memory>

class TrackerRepo {
  std::shared_ptr<DbManager> db_;
public:
  explicit TrackerRepo(std::shared_ptr<DbManager> db) : db_(std::move(db)) {}
  DbRows search(const std::string &query, int page = 1, int limit = 20);
  DbRows profile(int id);
  DbRows events();
  DbRows event_entries(const std::string &event);
};
