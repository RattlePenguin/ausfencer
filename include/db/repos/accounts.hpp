#pragma once

#include "db/db_manager.hpp"
#include <memory>

class AccountRepo {
  std::shared_ptr<DbManager> db_;
public:
  explicit AccountRepo(std::shared_ptr<DbManager> db) : db_(std::move(db)) {}
  std::string create_demo_session(const std::string &label, const std::string &role);
  DbRow authenticate(const std::string &token);
  void logout(const std::string &token);
};
