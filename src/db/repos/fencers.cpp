#include "db/repos/fencers.hpp"
#include "db/db_manager.hpp"
#include "db/repos/base.hpp"
#include "models/Fencer.hpp"
#include "sqlite_orm/sqlite_orm.h"
#include <memory>
#include "db/pagination.hpp"

FencerRepo::FencerRepo(std::shared_ptr<DbManager> db_mgr) : BaseRepo(db_mgr) {
  this->name_ = "FencerRepo";
}

int FencerRepo::create(const Fencer &fencer) {
  // use db_mgr to add Fencer entry
  // TODO confirm: storage_.insert returns autoincremented primary id, Fencer.id
  auto lock = db_mgr_->acquire_lock();
  return db_mgr_->get_storage().insert(fencer);
}

std::unique_ptr<Fencer> FencerRepo::get(int id) {
  auto lock = db_mgr_->acquire_lock();
  return db_mgr_->get_storage().get_pointer<Fencer>(id);
}

bool FencerRepo::update(const Fencer &fencer) {
  auto lock = db_mgr_->acquire_lock();
  auto &storage = db_mgr_->get_storage();
  if (!storage.get_pointer<Fencer>(fencer.id)) return false;
  storage.update(fencer);
  return true;
}

bool FencerRepo::remove(int id) {
  auto lock = db_mgr_->acquire_lock();
  auto &storage = db_mgr_->get_storage();
  if (!storage.get_pointer<Fencer>(id)) return false;
  storage.remove<Fencer>(id);
  return true;
}

std::vector<Fencer> FencerRepo::get_all(const std::string &q, const int page,
                                        const int limit) {
  auto lock = db_mgr_->acquire_lock();

  // how many entries to skip.
  const auto offset = page_offset(page, limit);

  if (q.empty()) {
    // Get all fencers, limited to limit after skipping offset * limit
    return db_mgr_->get_storage().get_all<Fencer>(
        sql::order_by(&Fencer::id), sql::limit(limit, sql::offset(offset)));
  } else {
    std::string search_pattern{"%" + q + "%"};

    return db_mgr_->get_storage().get_all<Fencer>(
        sql::where(sql::like(&Fencer::first_name, search_pattern) ||
                   sql::like(&Fencer::last_name, search_pattern)),
        sql::order_by(&Fencer::id), sql::limit(limit, sql::offset(offset)));
  }
}
