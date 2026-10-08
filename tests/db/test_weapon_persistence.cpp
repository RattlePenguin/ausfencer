#include "db/db_manager.hpp"
#include <algorithm>
#include <gtest/gtest.h>

class WeaponPersistenceTest : public ::testing::TestWithParam<Weapon> {
protected:
  DbManager db_mgr_{":memory:"};
  int left_id_;
  int right_id_;

  void SetUp() override {
    auto lock = db_mgr_.acquire_lock();
    auto &storage = db_mgr_.get_storage();
    left_id_ = storage.insert(Fencer{0, "Left", "Fencer", 2000});
    right_id_ = storage.insert(Fencer{0, "Right", "Fencer", 2001});
  }

  Bout make_bout(Weapon weapon) const {
    return Bout{0, left_id_, right_id_, 1700000000, weapon, 180,
                15, 12, 1, 0, 0, 1};
  }
};

TEST_P(WeaponPersistenceTest, StoresAndRetrievesIntegerWeapon) {
  auto lock = db_mgr_.acquire_lock();
  auto &storage = db_mgr_.get_storage();
  const int id = storage.insert(make_bout(GetParam()));

  auto bout = storage.get_pointer<Bout>(id);
  ASSERT_NE(bout, nullptr);
  EXPECT_EQ(bout->weapon, GetParam());

  const auto columns = storage.pragma.table_info("bouts");
  const auto weapon_column = std::find_if(
      columns.begin(), columns.end(),
      [](const auto &column) { return column.name == "weapon"; });
  ASSERT_NE(weapon_column, columns.end());
  EXPECT_EQ(weapon_column->type, "INTEGER");

  const auto integers = storage.select(sql::cast<int>(&Bout::weapon));
  ASSERT_EQ(integers.size(), 1u);
  EXPECT_EQ(integers[0], static_cast<int>(GetParam()));

  // Also exercise enum binding in a predicate and extraction in a scalar query.
  const auto weapons = storage.select(
      &Bout::weapon, sql::where(sql::c(&Bout::weapon) == GetParam()));
  ASSERT_EQ(weapons.size(), 1u);
  EXPECT_EQ(weapons[0], GetParam());
}

TEST_P(WeaponPersistenceTest, UpdatesWeapon) {
  auto lock = db_mgr_.acquire_lock();
  auto &storage = db_mgr_.get_storage();
  const Weapon original = GetParam() == Foil ? Epee : Foil;
  Bout bout = make_bout(original);
  bout.id = storage.insert(bout);
  bout.weapon = GetParam();
  storage.update(bout);

  auto retrieved = storage.get_pointer<Bout>(bout.id);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->weapon, GetParam());
}

INSTANTIATE_TEST_SUITE_P(AllWeapons, WeaponPersistenceTest,
                        ::testing::Values(Foil, Epee, Sabre));

TEST(WeaponPersistence, RejectsUnknownStoredInteger) {
  DbManager db_mgr(":memory:");
  auto lock = db_mgr.acquire_lock();
  auto &storage = db_mgr.get_storage();
  const int left_id = storage.insert(Fencer{0, "Left", "Fencer", 2000});
  const int right_id = storage.insert(Fencer{0, "Right", "Fencer", 2001});
  const int id = storage.insert(
      Bout{0, left_id, right_id, 1700000000, Foil, 180, 15, 12, 1, 0, 0, 1});

  // Simulate an invalid stored value without constructing an invalid C++ enum.
  storage.update_all(sql::set(sql::assign(&Bout::weapon, 4294967296LL)));
  EXPECT_THROW(storage.get_pointer<Bout>(id), std::domain_error);
}
