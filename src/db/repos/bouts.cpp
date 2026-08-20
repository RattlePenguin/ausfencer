#include "db/repos/bouts.hpp"
#include "db/db_manager.hpp"
#include "db/repos/base.hpp"
#include "models/Bout.hpp"
#include "sqlite_orm/sqlite_orm.h"
#include <memory>

BoutRepo::BoutRepo(std::shared_ptr<DbManager> db_mgr) : BaseRepo(db_mgr) {
  this->name_ = "BoutRepo";
}

int BoutRepo::create(const Bout &bout) {
  // use db_mgr to add Bout entry
  // TODO confirm: storage_.insert returns autoincremented primary id, Bout.id
  auto lock = db_mgr_->acquire_lock();
  return db_mgr_->get_storage().insert(bout);
}

std::unique_ptr<Bout> BoutRepo::get(int id) {
  auto lock = db_mgr_->acquire_lock();
  return db_mgr_->get_storage().get_pointer<Bout>(id);
}

void BoutRepo::update(const Bout &bout) {
  auto lock = db_mgr_->acquire_lock();
  return db_mgr_->get_storage().update<Bout>(bout);
}

void BoutRepo::remove(int id) {
  auto lock = db_mgr_->acquire_lock();
  return db_mgr_->get_storage().remove<Bout>(id);
}

std::vector<Bout> BoutRepo::get_all(const int page, const int limit) {
  auto lock = db_mgr_->acquire_lock();

  // how many entries to skip.
  int offset{(page - 1) * limit};

  // Get all bouts, limited to limit after skipping offset * limit
  return db_mgr_->get_storage().get_all<Bout>(
      sql::limit(limit, sql::offset(offset)));
}

std::vector<Bout> BoutRepo::get_all_by_fencer(const int fencer_id,
                                              const int page, const int limit) {
  auto lock = db_mgr_->acquire_lock();

  // how many entries to skip.
  int offset{(page - 1) * limit};

  return db_mgr_->get_storage().get_all<Bout>(
      sql::where(sql::like(&Bout::left_fencer_id, fencer_id) ||
                 sql::like(&Bout::right_fencer_id, fencer_id)),
      sql::limit(limit, sql::offset(offset)));
}
