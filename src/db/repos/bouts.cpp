#include "db/repos/bouts.hpp"
#include "db/db_manager.hpp"
#include "db/repos/base.hpp"
#include "models/Bout.hpp"
#include "sqlite_orm/sqlite_orm.h"
#include <memory>
#include "db/pagination.hpp"

BoutRepo::BoutRepo(std::shared_ptr<DbManager> db_mgr) : BaseRepo(db_mgr) {
  this->name_ = "BoutRepo";
}

int BoutRepo::create(const Bout &bout) {
  if (bout.left_fencer_id && bout.left_fencer_id == bout.right_fencer_id)
    throw std::invalid_argument("Participants must be different fencers");
  // use db_mgr to add Bout entry
  // TODO confirm: storage_.insert returns autoincremented primary id, Bout.id
  auto lock = db_mgr_->acquire_lock();
  return db_mgr_->get_storage().insert(bout);
}

std::unique_ptr<Bout> BoutRepo::get(int id) {
  auto lock = db_mgr_->acquire_lock();
  return db_mgr_->get_storage().get_pointer<Bout>(id);
}

bool BoutRepo::update(const Bout &bout) {
  if (bout.left_fencer_id && bout.left_fencer_id == bout.right_fencer_id)
    throw std::invalid_argument("Participants must be different fencers");
  auto lock = db_mgr_->acquire_lock();
  auto &storage = db_mgr_->get_storage();
  if (!storage.get_pointer<Bout>(bout.id)) return false;
  storage.update(bout);
  return true;
}

bool BoutRepo::remove(int id) {
  auto lock = db_mgr_->acquire_lock();
  auto &storage = db_mgr_->get_storage();
  if (!storage.get_pointer<Bout>(id)) return false;
  storage.remove<Bout>(id);
  return true;
}

std::vector<Bout> BoutRepo::get_all(const int page, const int limit) {
  auto lock = db_mgr_->acquire_lock();

  // how many entries to skip.
  const auto offset = page_offset(page, limit);

  // Get all bouts, limited to limit after skipping offset * limit
  return db_mgr_->get_storage().get_all<Bout>(
      sql::order_by(&Bout::id), sql::limit(limit, sql::offset(offset)));
}

std::vector<Bout> BoutRepo::get_all_by_fencer(const int fencer_id,
                                              const int page, const int limit) {
  auto lock = db_mgr_->acquire_lock();

  // how many entries to skip.
  const auto offset = page_offset(page, limit);

  return db_mgr_->get_storage().get_all<Bout>(
      sql::where((sql::c(&Bout::left_fencer_id) == fencer_id) ||
                 (sql::c(&Bout::right_fencer_id) == fencer_id)),
      sql::order_by(&Bout::id), sql::limit(limit, sql::offset(offset)));
}
