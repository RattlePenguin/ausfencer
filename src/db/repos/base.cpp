#include "db/repos/base.hpp"
#include "db/db_manager.hpp"
#include <memory>

BaseRepo::BaseRepo(std::shared_ptr<DbManager> db_mgr)
    : db_mgr_{db_mgr}, name_{"BaseRepo"} {}

std::string BaseRepo::get_name() const { return this->name_; }
