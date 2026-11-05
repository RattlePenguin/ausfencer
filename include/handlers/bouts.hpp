#pragma once

#include "base.hpp"
#include "db/repos/bouts.hpp"
#include "interface.hpp"
#include <memory>

class BoutHandler : public BaseHandler {
  std::shared_ptr<BoutRepo> repo_;

  // Returns all bouts, capped by pagination and limits
  // req url params: fencer_id, page, limit
  // fencer_id optionally matches either participant
  // page: number to offset by based on limit
  // limit: max number of rows/entries returned at once
  crow::response get_all(const crow::request &req);

  // Returns the bout with the given id, else returns not found
  crow::response get(int id);

  // Creates the bout with the given id
  // Returns the id of the bout
  // Both participant keys are required; null denotes an anonymous guest.
  crow::response create(const crow::request &req);

  // Updates the bout with the given id
  crow::response update(int id, const crow::request &req);

  // Removes the bout with the given id
  crow::response remove(int id);

public:
  BoutHandler(const std::string &basePath, std::shared_ptr<BoutRepo>);
  void register_routes(App &app) override;
};
