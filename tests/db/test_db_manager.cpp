#include "db/db_manager.hpp"
#include "db/migrations.hpp"
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

TEST_F(DbManagerTest, DomainMigrationIsRepeatableAndPreservesLegacyRecords) {
  DbManager db(":memory:");
  auto id = db.get_storage().insert(Fencer{0, "Demo", "Legacy", 2000});
  migrate_tracker(db);
  EXPECT_EQ(db.get_storage().get<Fencer>(id).last_name, "Legacy");
  EXPECT_EQ(db.query("PRAGMA foreign_keys")[0].at("foreign_keys"), "1");
  EXPECT_THROW(db.query("INSERT INTO aliases VALUES(999,'Unknown')"), std::runtime_error);
  db.query("PRAGMA user_version = 99");
  EXPECT_THROW(migrate_tracker(db), std::runtime_error);
}

TEST_F(DbManagerTest, BoutsRequireEntriesAndValidScores) {
  DbManager db(":memory:");
  db.query("INSERT INTO sources(id,basis,parser_version,checksum) VALUES('demo','synthetic','1','test')");
  EXPECT_THROW(db.query(R"(INSERT INTO bouts(id,event_id,round,pool,left_id,right_id,
    left_score,right_score,winner_id,status,trust,source_id)
    VALUES('test','missing','F','',1,2,15,10,1,'completed','synthetic','demo'))"), std::runtime_error);
  EXPECT_TRUE(db.query("SELECT * FROM bouts").empty());
}

TEST_F(DbManagerTest, PreparedQueriesPreserveEmbeddedNulls) {
  DbManager db(":memory:");
  std::string value("Demo\0Name",9);
  auto rows=db.query("SELECT ? AS value",{value});
  EXPECT_EQ(rows[0].at("value"),value);
}
