#pragma once

#include "base.hpp"
#include "db/db_manager.hpp"
#include "models/Bout.hpp"
#include <memory>
#include <vector>

class BoutRepo : public BaseRepo {
public:
  explicit BoutRepo(std::shared_ptr<DbManager> db_mgr);

  // Creates a new data entry with the given Bout, returns its primary id
  // Primary id is autoincremented with each entry
  int create(const Bout &fencer);

  // Getter for Bout with id
  std::unique_ptr<Bout> get(int id);

  // Updates Bout, id is taken from the Bout object itself
  void update(const Bout &fencer);

  // Removes Bout with given id
  void remove(int id);

  // Returns a list of all fencers
  // Accepts q, page, and limit for limiting results
  std::vector<Bout> get_all(const std::string &q, const int page,
                            const int limit);
};
