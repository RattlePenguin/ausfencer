#pragma once

#include "interface.hpp"

class BaseRepo {
  DbManager &db_mgr_;

public:
  BaseRepo(DbManager &db_mgr);
};
