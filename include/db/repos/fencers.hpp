#include "base.hpp"
#include "db/db_manager.hpp"
#include "models/Fencer.hpp"
#include <memory>
#include <vector>

class FencersRepo : public BaseRepo {
  // Creates a new data entry with the given Fencer, returns its primary id
  // Primary id is autoincremented with each entry
  int create(const Fencer &fencer);
  std::unique_ptr<Fencer> get(int id);
  void update(const Fencer &fencer);
  void remove(int id);
  std::vector<Fencer> get_all();

public:
  explicit FencersRepo(DbManager &db_mgr);
};
