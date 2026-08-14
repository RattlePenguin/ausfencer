#pragma once

#include "base.hpp"
#include "db/repos/fencers.hpp"
#include "interface.hpp"

class FencerHandler : public BaseHandler {
  std::shared_ptr<FencerRepo> repo_;

  crow::response get_all(const crow::request &req);
  crow::response get(int id);
  crow::response create(const crow::request &req);
  crow::response update(int id, const crow::request &req);
  crow::response remove(int id);

public:
  FencerHandler(const std::string &basePath);
  void register_routes(App &app) override;
};
