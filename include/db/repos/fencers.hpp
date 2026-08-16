#include "base.hpp"
#include "db/db_manager.hpp"
#include "models/Fencer.hpp"
#include <memory>
#include <vector>

class FencerRepo : public BaseRepo {
public:
  explicit FencerRepo(std::shared_ptr<DbManager> db_mgr);

  // Creates a new data entry with the given Fencer, returns its primary id
  // Primary id is autoincremented with each entry
  int create(const Fencer &fencer);

  // Getter for Fencer with id
  std::unique_ptr<Fencer> get(int id);

  // Updates Fencer, id is taken from the Fencer object itself
  void update(const Fencer &fencer);

  // Removes Fencer with given id
  void remove(int id);

  // Returns a list of all fencers
  // Accepts q, page, and limit for limiting results
  std::vector<Fencer> get_all(const std::string &q, const int page,
                              const int limit);
};
