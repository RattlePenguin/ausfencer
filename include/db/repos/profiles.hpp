#pragma once

#include "db/repos/accounts.hpp"

class ProfileRepo {
  std::shared_ptr<DbManager> db_;
  void change_visibility(const std::string &token, int fencer, bool visible, bool withdraw);
public:
  explicit ProfileRepo(std::shared_ptr<DbManager> db) : db_(std::move(db)) {}
  int request_claim(const std::string &token, int fencer, const std::string &kind,
                    const std::string &evidence, const std::string &consent);
  void review_claim(const std::string &token, int claim, bool approved);
  void set_visibility(const std::string &token, int id, bool visible) { change_visibility(token,id,visible,false); }
  void withdraw(const std::string &token, int id) { change_visibility(token,id,false,true); }
  DbRows claims(const std::string &token);
  int request_privacy(int fencer, const std::string &contact, const std::string &evidence);
  void review_privacy(const std::string &token, int request, bool hide);
  DbRows review_queue(const std::string &token);
};
