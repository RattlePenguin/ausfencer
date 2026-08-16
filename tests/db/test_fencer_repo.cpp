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
  Fencer fencer{0, "Allan", "Goodman", 2002};
  int id = repo_->create(fencer);
  EXPECT_GT(id, 0);

  auto retrieved = repo_->get(id);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->id, id);
  EXPECT_EQ(retrieved->first_name, "Allan");
  EXPECT_EQ(retrieved->last_name, "Goodman");
  EXPECT_EQ(retrieved->birth_year, 2002);
}

TEST_F(FencerRepoTest, GetNonExistentFencerReturnsNull) {
  auto retrieved = repo_->get(999);
  EXPECT_EQ(retrieved, nullptr);
}

TEST_F(FencerRepoTest, UpdateFencer) {
  Fencer fencer{0, "Allan", "Le Goodman", 1982};
  int id = repo_->create(fencer);

  Fencer update_data{id, "Allan", "LeGoodman", 1982};
  repo_->update(update_data);

  auto retrieved = repo_->get(id);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->last_name, "LeGoodman");
}

TEST_F(FencerRepoTest, RemoveFencer) {
  Fencer fencer{0, "Allan", "Goodman", 1992};
  int id = repo_->create(fencer);

  auto before_remove = repo_->get(id);
  EXPECT_NE(before_remove, nullptr);

  repo_->remove(id);

  auto after_remove = repo_->get(id);
  EXPECT_EQ(after_remove, nullptr);
}

TEST_F(FencerRepoTest, GetAllWithPaginationAndSearch) {
  repo_->create(Fencer{0, "Alice", "Smith", 2001});
  repo_->create(Fencer{0, "Bob", "Smith", 1998});
  repo_->create(Fencer{0, "Charlie", "Brown", 2005});
  repo_->create(Fencer{0, "David", "Smith", 1995});

  // Test get_all default without search
  auto page1 = repo_->get_all("", 1, 2);
  EXPECT_EQ(page1.size(), 2u);

  auto page2 = repo_->get_all("", 2, 2);
  EXPECT_EQ(page2.size(), 2u);

  // Test get_all search filter for "Smith"
  auto smiths = repo_->get_all("Smith", 1, 10);
  EXPECT_EQ(smiths.size(), 3u);

  // Test get_all search filter for "Charlie"
  auto charlie = repo_->get_all("Charlie", 1, 10);
  ASSERT_EQ(charlie.size(), 1u);
  EXPECT_EQ(charlie[0].first_name, "Charlie");
}
