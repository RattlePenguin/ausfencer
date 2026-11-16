#pragma once

#include "db/db_manager.hpp"
#include "interface.hpp"
#include <string>

class BaseRepo : public IRepo {
protected:
  // all db operations go through db mgr
  // I'm using a shared ptr here to indicate
  // shared ownership of a singular db mgr
  std::shared_ptr<DbManager> db_mgr_;
  std::string name_;

public:
  BaseRepo(std::shared_ptr<DbManager> db_mgr);
  std::string get_name() const;
};
