#include "db/db_manager.hpp"
#include <cstring>
#include <sqlite3.h>

DbManager::DbManager(const std::string &db_name) : db_name_{db_name} {
  char buf[1024];
  strcpy(buf, db_name.c_str());
  sqlite3_open(buf, &db_);
}

DbManager::~DbManager() {
  if (db_)
    sqlite3_close(db_);
}
