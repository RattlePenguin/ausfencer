#include <iostream>
#include <sqlite3.h>

class DbManager {
  DbManager(std::string dbName) {
    sqlite3 *db;
    sqlite3_open(dbName, &db);
  }
  ~DbManager() {}
};
