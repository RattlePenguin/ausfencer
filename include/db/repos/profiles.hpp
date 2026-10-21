#pragma once

#include "db/repos/accounts.hpp"

class ProfileRepo {
  std::shared_ptr<DbManager> db_;
public:
  explicit ProfileRepo(std::shared_ptr<DbManager> db) : db_(std::move(db)) {}
  int request_claim(const std::string &token, int fencer, const std::string &kind,
                    const std::string &evidence, const std::string &consent);
  void review_claim(const std::string &token, int claim, bool approved);
};
