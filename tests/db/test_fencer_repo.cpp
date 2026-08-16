#include "db/db_manager.hpp"
#include "db/repos/fencers.hpp"
#include "models/Fencer.hpp"
#include <gtest/gtest.h>
#include <memory>

class FencerRepoTest : public ::testing::Test {
protected:
  std::shared_ptr<DbManager> db_mgr_;
  std::shared_ptr<FencerRepo> repo_;

  void SetUp() override {
    db_mgr_ = std::make_shared<DbManager>(":memory:");
    repo_ = std::make_shared<FencerRepo>(db_mgr_);
  }
};

TEST_F(FencerRepoTest, CreateAndGetFencer) {
  Fencer fencer{0, "Alexander", "Massialas", 1994};
  int id = repo_->create(fencer);
  EXPECT_GT(id, 0);

  auto retrieved = repo_->get(id);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->id, id);
  EXPECT_EQ(retrieved->first_name, "Alexander");
  EXPECT_EQ(retrieved->last_name, "Massialas");
  EXPECT_EQ(retrieved->birth_year, 1994);
}

