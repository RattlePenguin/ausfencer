#include "db/db_manager.hpp"
#include <gtest/gtest.h>

class DbManagerTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DbManagerTest, InitializeInMemoryDatabase) {
  EXPECT_NO_THROW({
    DbManager db_mgr(":memory:");
    auto lock = db_mgr.acquire_lock();
    EXPECT_TRUE(lock.owns_lock());
  });
}

TEST_F(DbManagerTest, StorageTableCreationAndSyncSchema) {
  DbManager db_mgr(":memory:");
  auto &storage = db_mgr.get_storage();
  
  // Verify storage is functional by inserting and retrieving a fencer
  Fencer fencer{0, "Test", "User", 2000};
  int id = storage.insert(fencer);
  EXPECT_GT(id, 0);

  auto retrieved = storage.get_pointer<Fencer>(id);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->first_name, "Test");
  EXPECT_EQ(retrieved->last_name, "User");
  EXPECT_EQ(retrieved->birth_year, 2000);
}

