#pragma once

#include "db/repos/accounts.hpp"
#include "models/Tracker.hpp"

class ResultRepo {
  std::shared_ptr<DbManager> db_;
public:
  explicit ResultRepo(std::shared_ptr<DbManager> db) : db_(std::move(db)) {}
  int report(const std::string &token, int bout, const std::string &kind, const std::string &detail);
  DbRows reports(const std::string &token, bool queue = false);
};
