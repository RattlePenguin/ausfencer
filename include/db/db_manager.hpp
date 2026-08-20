#pragma once

#include "models/Bout.hpp"
#include "models/Fencer.hpp"
#include <mutex>
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
                      sql::make_column("birth_year", &Fencer::birth_year)),
      sql::make_table(
          "bouts",
          sql::make_column("id", &Bout::id, sql::primary_key().autoincrement()),
          sql::make_column("left_fencer_id", &Bout::left_fencer_id),
          sql::foreign_key(&Bout::left_fencer_id).references(&Fencer::id),
          sql::make_column("right_fencer_id", &Bout::right_fencer_id),
          sql::foreign_key(&Bout::right_fencer_id).references(&Fencer::id),
          sql::make_column("weapon", &Bout::weapon),
          sql::make_column("time", &Bout::time),
          sql::make_column("left_score", &Bout::left_score),
          sql::make_column("right_score", &Bout::right_score),
          sql::make_column("left_yellow", &Bout::left_yellow),
          sql::make_column("right_yellow", &Bout::right_yellow),
          sql::make_column("left_red", &Bout::left_red),
          sql::make_column("right_red", &Bout::right_red)));
}

using Storage = decltype(create_db_storage(""));

class DbManager {
  std::string db_name_;
  Storage storage_;
  mutable std::mutex mutex_;

public:
  explicit DbManager(const std::string &db_name);
  ~DbManager() = default;

  Storage &get_storage() { return storage_; }

  // Acquires the mutex lock on db mgr.
  // locking the mutex is a mutating operation,
  // so mutex must be made mutable if db mgr is const
  std::unique_lock<std::mutex> acquire_lock() const {
    return std::unique_lock<std::mutex>(mutex_);
  }
};
