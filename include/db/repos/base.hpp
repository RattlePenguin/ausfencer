#pragma once

#include "interface.hpp"
#include <string>

class BaseRepo : public IRepo {
protected:
  DbManager &db_mgr_;
  std::string name_;

public:
  BaseRepo(DbManager &db_mgr);
};
