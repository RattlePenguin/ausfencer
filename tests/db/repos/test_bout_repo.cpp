#include "db/db_manager.hpp"
#include "db/repos/bouts.hpp"
#include "db/repos/fencers.hpp"
#include "models/Bout.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <future>

class BoutRepoTest : public ::testing::Test {
protected:
  std::shared_ptr<DbManager> db_mgr_;
  std::shared_ptr<BoutRepo> repo_;

  int left_, right_;
  Bout fixture() const {
    return {0, left_, right_, 1700000000, Foil, 180, 15, 12, 1, 0, 0, 1};
  }

  void SetUp() override {
    db_mgr_ = std::make_shared<DbManager>(":memory:");
    repo_ = std::make_shared<BoutRepo>(db_mgr_);
    FencerRepo fencers(db_mgr_);
    left_ = fencers.create({0, "Left", "Fencer", 2000});
    right_ = fencers.create({0, "Right", "Fencer", 2001});
  }
};

TEST_F(BoutRepoTest, CreateAndGetBout) {
  Bout bout = fixture();
  int id = repo_->create(bout);
  EXPECT_GT(id, 0);

  auto retrieved = repo_->get(id);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->id, id);
  EXPECT_EQ(retrieved->left_fencer_id, left_);
  EXPECT_EQ(retrieved->right_fencer_id, right_);
  EXPECT_EQ(retrieved->timestamp, bout.timestamp);
  EXPECT_EQ(retrieved->weapon, Foil);
  EXPECT_EQ(retrieved->time, 180);
  EXPECT_EQ(retrieved->left_score, 15);
  EXPECT_EQ(retrieved->right_score, 12);
  EXPECT_EQ(retrieved->left_yellow, 1);
  EXPECT_EQ(retrieved->right_yellow, 0);
  EXPECT_EQ(retrieved->left_red, 0);
  EXPECT_EQ(retrieved->right_red, 1);
}

TEST_F(BoutRepoTest, GetNonExistentBoutReturnsNull) {
  auto retrieved = repo_->get(999);
  EXPECT_EQ(retrieved, nullptr);
}

TEST_F(BoutRepoTest, UpdateBout) {
  Bout bout = fixture();
  int id = repo_->create(bout);

  Bout update_data{id, right_, left_, 1800000000, Sabre, 120,
                   10, 11, 0, 1, 2, 0};
  repo_->update(update_data);

  auto retrieved = repo_->get(id);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->id, id);
  EXPECT_EQ(retrieved->left_fencer_id, right_);
  EXPECT_EQ(retrieved->right_fencer_id, left_);
  EXPECT_EQ(retrieved->timestamp, update_data.timestamp);
  EXPECT_EQ(retrieved->weapon, Sabre);
  EXPECT_EQ(retrieved->time, 120);
  EXPECT_EQ(retrieved->left_score, 10);
  EXPECT_EQ(retrieved->right_score, 11);
  EXPECT_EQ(retrieved->left_yellow, 0);
  EXPECT_EQ(retrieved->right_yellow, 1);
  EXPECT_EQ(retrieved->left_red, 2);
  EXPECT_EQ(retrieved->right_red, 0);
}

TEST_F(BoutRepoTest, RemoveBout) {
  Bout bout = fixture();
  int id = repo_->create(bout);

  auto before_remove = repo_->get(id);
  EXPECT_NE(before_remove, nullptr);

  repo_->remove(id);

  auto after_remove = repo_->get(id);
  EXPECT_EQ(after_remove, nullptr);
}

TEST_F(BoutRepoTest, ListsHaveStableDistinctPages) {
  EXPECT_TRUE(repo_->get_all(1, 10).empty());
  const int a = repo_->create(fixture());
  const int b = repo_->create(fixture());
  const int c = repo_->create(fixture());
  auto first = repo_->get_all(1, 2);
  auto second = repo_->get_all(2, 2);
  ASSERT_EQ(first.size(), 2u);
  ASSERT_EQ(second.size(), 1u);
  EXPECT_EQ(first[0].id, a);
  EXPECT_EQ(first[1].id, b);
  EXPECT_EQ(second[0].id, c);
  EXPECT_TRUE(repo_->get_all(3, 2).empty());
}

TEST_F(BoutRepoTest, HistoryMatchesEitherSideOnceAndPaginates) {
  auto bout = fixture();
  const int a = repo_->create(bout);
  std::swap(bout.left_fencer_id, bout.right_fencer_id);
  const int b = repo_->create(bout);
  bout.right_fencer_id = left_;
  const int c = repo_->create(bout);
  bout.left_fencer_id = bout.right_fencer_id = right_;
  repo_->create(bout);
  auto first = repo_->get_all_by_fencer(left_, 1, 2);
  auto second = repo_->get_all_by_fencer(left_, 2, 2);
  ASSERT_EQ(first.size(), 2u);
  ASSERT_EQ(second.size(), 1u);
  EXPECT_EQ(first[0].id, a);
  EXPECT_EQ(first[1].id, b);
  EXPECT_EQ(second[0].id, c);
  EXPECT_TRUE(repo_->get_all_by_fencer(999, 1, 10).empty());
}

TEST_F(BoutRepoTest, RejectsInvalidPaginationAndHandlesLargeOffsets) {
  EXPECT_THROW(repo_->get_all(0, 10), std::invalid_argument);
  EXPECT_THROW(repo_->get_all(1, 101), std::invalid_argument);
  EXPECT_THROW(repo_->get_all_by_fencer(left_, -1, 10), std::invalid_argument);
  EXPECT_THROW(repo_->get_all_by_fencer(left_, 1, 0), std::invalid_argument);
  EXPECT_TRUE(repo_->get_all(2147483647, 100).empty());
}

TEST_F(BoutRepoTest, ForeignKeysRejectOrphansAndPreserveHistory) {
  FencerRepo fencers(db_mgr_);
  for (bool left : {false, true}) {
    auto bout = fixture();
    (left ? bout.left_fencer_id : bout.right_fencer_id) = 999;
    EXPECT_THROW(repo_->create(bout), std::system_error);
    bout.id = repo_->create(fixture());
    EXPECT_THROW(repo_->update(bout), std::system_error);
    auto saved = repo_->get(bout.id);
    ASSERT_NE(saved, nullptr);
    EXPECT_EQ(saved->left_fencer_id, left_);
    EXPECT_EQ(saved->right_fencer_id, right_);
  }
  EXPECT_THROW(fencers.remove(left_), std::system_error);
  EXPECT_THROW(fencers.remove(right_), std::system_error);
  EXPECT_NE(fencers.get(left_), nullptr);
}

TEST_F(BoutRepoTest, CoordinatedDeleteAndUpdateReportPersistedOutcomes) {
  auto bout = fixture();
  bout.id = repo_->create(bout);
  std::promise<void> deleted;
  auto done = deleted.get_future();
  auto remover = std::async(std::launch::async, [&] {
    const bool removed = repo_->remove(bout.id);
    deleted.set_value();
    return removed;
  });
  auto updater = std::async(std::launch::async, [&] {
    done.wait();
    return repo_->update(bout);
  });
  EXPECT_TRUE(remover.get());
  EXPECT_FALSE(updater.get());
  EXPECT_FALSE(repo_->remove(bout.id));
  EXPECT_EQ(repo_->get(bout.id), nullptr);
}
