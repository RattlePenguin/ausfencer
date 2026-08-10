#include <iostream>
#include <sqlite3.h>

class DbManager {
  DbManager(char *dbName) {
    sqlite3 *db;
    sqlite3_open(dbName, &db);
  }
  ~DbManager() {}

  // adds bout to db
  void addBout(Bout bout);

  Bout getBout();
};
