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

TEST_F(DbManagerTest, ConcurrentLockAcquisition) {
  DbManager db_mgr(":memory:");
  {
    auto lock1 = db_mgr.acquire_lock();
    EXPECT_TRUE(lock1.owns_lock());
  }
  {
    auto lock2 = db_mgr.acquire_lock();
    EXPECT_TRUE(lock2.owns_lock());
  }
}

TEST_F(DbManagerTest, PreparedQueriesPreserveQuotedValuesAndRollback) {
  DbManager db_mgr(":memory:");
  auto lock = db_mgr.acquire_lock();
  db_mgr.query("BEGIN");
  db_mgr.query("INSERT INTO fencers(first_name,last_name,birth_year) VALUES(?,?,?)",
               {"D'Arcy", "Demo", "2000"});
  EXPECT_EQ(db_mgr.query("SELECT first_name FROM fencers")[0].at("first_name"), "D'Arcy");
  db_mgr.query("ROLLBACK");
  EXPECT_TRUE(db_mgr.query("SELECT * FROM fencers").empty());
  EXPECT_THROW(db_mgr.query("SELECT ?"), std::invalid_argument);
  EXPECT_THROW(db_mgr.query("SELECT missing FROM fencers"), std::runtime_error);
}
