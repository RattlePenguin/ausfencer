#pragma once

#include "base.hpp"
#include "db/repos/fencers.hpp"
#include "interface.hpp"

class FencerHandler : public BaseHandler {
  std::shared_ptr<FencerRepo> repo_;

  // Returns all fencers, capped by pagination and limits
  // req url params: q, page, limit
  // q: search pattern on fencer first/last name
  // page: number to offset by based on limit
  // limit: max number of rows/entries returned at once
  crow::response get_all(const crow::request &req);

  // Returns the fencer with the given id, else returns not found
  crow::response get(int id);

  // Creates the fencer with the given id
  // TODO Currently accepts duplicate fencers without distinguishing
  // Returns the id of the fencer
  crow::response create(const crow::request &req);

  // Updates the fencer with the given id
  crow::response update(int id, const crow::request &req);

  // Removes the fencer with the given id
  crow::response remove(int id);

public:
  FencerHandler(const std::string &basePath);
  void register_routes(App &app) override;
};
