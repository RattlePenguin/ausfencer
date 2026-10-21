#include "db/repos/fencers.hpp"
#include "db/db_manager.hpp"
#include "db/repos/base.hpp"
#include "models/Fencer.hpp"
#include "sqlite_orm/sqlite_orm.h"
#include <memory>

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
  if (!db_mgr_->query("SELECT 1 FROM profiles WHERE fencer_id=? AND visible=0",
                       {std::to_string(id)}).empty()) return nullptr;
  return db_mgr_->get_storage().get_pointer<Fencer>(id);
}

void FencerRepo::update(const Fencer &fencer) {
  auto lock = db_mgr_->acquire_lock();
  return db_mgr_->get_storage().update<Fencer>(fencer);
}

void FencerRepo::remove(int id) {
  auto lock = db_mgr_->acquire_lock();
  return db_mgr_->get_storage().remove<Fencer>(id);
}

std::vector<Fencer> FencerRepo::get_all(const std::string &q, const int page,
                                        const int limit) {
  auto lock = db_mgr_->acquire_lock();

  if (page < 1 || page > 1000000 || limit < 1 || limit > 100)
    throw std::invalid_argument("Invalid fencer pagination");
  auto rows = db_mgr_->query(R"(SELECT f.id,f.first_name,f.last_name,f.birth_year
    FROM fencers f LEFT JOIN profiles p ON p.fencer_id=f.id
    WHERE COALESCE(p.visible,1)=1
      AND instr(lower(f.first_name||' '||f.last_name),lower(?))>0
    ORDER BY f.last_name,f.first_name,f.id LIMIT ? OFFSET ?)",
    {q,std::to_string(limit),std::to_string((page-1)*limit)});
  std::vector<Fencer> result;
  for (const auto &row : rows)
    result.push_back({std::stoi(row.at("id")),row.at("first_name"),row.at("last_name"),
                      std::stoi(row.at("birth_year"))});
  return result;
}
