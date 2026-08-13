#include "db/db_manager.hpp"

DbManager::DbManager(const std::string &db_name)
    : db_name_{db_name}, storage_{create_db_storage(db_name)} {

  // sync_schema creates tables if they don't exist
  storage_.sync_schema();
}
