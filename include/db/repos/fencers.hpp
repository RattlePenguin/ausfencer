#include "base.hpp"
#include "db/db_manager.hpp"
#include "models/Fencer.hpp"
#include <vector>

class FencersRepo : public BaseRepo {
  void create();
  Fencer get();
  Fencer update();
  void remove();
  std::vector<Fencer> get_all();

public:
  FencersRepo(DbManager &db_mgr);
};
