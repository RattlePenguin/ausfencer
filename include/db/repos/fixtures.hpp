#pragma once

#include "db/db_manager.hpp"
#include "models/Tracker.hpp"
#include <memory>

class FixtureRepo {
  std::shared_ptr<DbManager> db_;
  void write_context(const FixtureData &data);
  void write_bouts(const FixtureData &data);
public:
  explicit FixtureRepo(std::shared_ptr<DbManager> db) : db_(std::move(db)) {}
  // Preview executes the same writes, then rolls the transaction back.
  void import_fixture(const FixtureData &data, bool approved = false);
};
