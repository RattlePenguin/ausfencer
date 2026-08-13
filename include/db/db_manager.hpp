#pragma once

#include "models/Fencer.hpp"
#include <mutex>
#include <shared_mutex>
#include <sqlite3.h>
#include <sqlite_orm/sqlite_orm.h>
#include <string>

namespace sql = sqlite_orm;

/**
  Creates the sqlite file with db_name, then populates with the db schema
  defined here. Normally bad practice in header files, but I need the decltype
  for the Storage definition.
*/
inline auto create_db_storage(const std::string &db_name) {
  return sql::make_storage(
      db_name,
      sql::make_table("fencers",
                      sql::make_column("id", &Fencer::id,
                                       sql::primary_key().autoincrement()),
                      sql::make_column("first_name", &Fencer::first_name),
                      sql::make_column("last_name", &Fencer::last_name),
                      sql::make_column("birth_year", &Fencer::birth_year)));
}

#include "../models/Fencer.hpp"

class DbManager {
  std::string db_name_;
  sqlite3 *db_;

  // Expose shared lock to repositories that want to read
  std::shared_lock<std::shared_mutex> acquire_read_lock() const;

  // Unique lock for writes
  std::unique_lock<std::shared_mutex> acquire_write_lock() const;

public:
  explicit DbManager(const std::string &db_name);
  ~DbManager();
};
