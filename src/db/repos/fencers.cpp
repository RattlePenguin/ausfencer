#include "db/repos/fencers.hpp"
#include "db/db_manager.hpp"
#include "db/repos/base.hpp"
#include "models/Fencer.hpp"
#include <memory>

FencerRepo::FencerRepo(DbManager &db_mgr) : BaseRepo(db_mgr) {
  this->name_ = "FencerRepo";
}

int FencerRepo::create(const Fencer &fencer) {
  // use db_mgr to add Fencer entry
  // TODO confirm: storage_.insert returns autoincremented primary id, Fencer.id
  auto lock = db_mgr_.acquire_lock();
  return db_mgr_.get_storage().insert(fencer);
}

std::unique_ptr<Fencer> FencerRepo::get(int id) {
  auto lock = db_mgr_.acquire_lock();
  return db_mgr_.get_storage().get_pointer<Fencer>(id);
}

void FencerRepo::update(const Fencer &fencer) {
  auto lock = db_mgr_.acquire_lock();
  return db_mgr_.get_storage().update<Fencer>(fencer);
}

void FencerRepo::remove(int id) {
  auto lock = db_mgr_.acquire_lock();
  return db_mgr_.get_storage().remove<Fencer>(id);
}

std::vector<Fencer> FencerRepo::get_all() {
  auto lock = db_mgr_.acquire_lock();
  return db_mgr_.get_storage().get_all<Fencer>();
}
