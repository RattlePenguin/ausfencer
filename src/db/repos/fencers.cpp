#include "db/repos/fencers.hpp"
#include "db/db_manager.hpp"
#include "db/repos/base.hpp"
#include "models/Fencer.hpp"

FencersRepo::FencersRepo(DbManager &db_mgr) : BaseRepo(db_mgr) {
  this->name_ = "FencersRepo";
}

void FencersRepo::create() {
  // use db_mgr to add Fencer entry
}
Fencer FencersRepo::get();
Fencer FencersRepo::update();
void FencersRepo::remove();
std::vector<Fencer> FencersRepo::get_all();
