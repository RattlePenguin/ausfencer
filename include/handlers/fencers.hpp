#pragma once

#include "../models/Fencer.hpp"
#include "base.hpp"
#include "interface.hpp"
#include <unordered_map>

class FencerHandler : public BaseHandler {
  std::unordered_map<int, Fencer> fencers_;
  int lastID_;
  std::mutex mutex_;

  crow::response list(const crow::request &req);
  crow::response get(int id);
  crow::response create(const crow::request &req);
  crow::response update(int id, const crow::request &req);
  crow::response remove(int id);

public:
  FencerHandler(const std::string &basePath);
  void register_routes(App &app) override;
};
