#include "db/repos/base.hpp"
#include "db/db_manager.hpp"

BaseRepo::BaseRepo(DbManager &db_mgr) : db_mgr_{db_mgr}, name_{"BaseRepo"} {}
