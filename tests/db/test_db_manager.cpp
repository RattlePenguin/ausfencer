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
