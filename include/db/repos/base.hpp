#pragma once

#include "db/db_manager.hpp"
#include "interface.hpp"
#include <string>

class BaseRepo : public IRepo {
protected:
  std::shared_ptr<DbManager> db_mgr_;
  std::string name_;

public:
  BaseRepo(std::shared_ptr<DbManager> db_mgr);
  std::string get_name() const;
};
