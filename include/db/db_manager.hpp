#include <sqlite3.h>

#include "../bout.hpp"

class DbManager {
  DbManager(const std::string &dbName) { auto storage{createStorage(dbName)}; }
  ~DbManager() {}

  inline auto createStorage(const std::string &filename) {
    return sqlite_orm::make_storage(
        dbName,
        sqlite_orm::make_table("fencers",
                               sqlite_orm::make_column(
                                   "id", &Fencer::id, sqlite_orm::primary_key(),
                                   sqlite_orm::auto_increment()),
                               sqlite_orm::make_column("name", &Fencer::name),
                               sqlite_orm::make_column("age", &Fencer::age)));
  }
};
