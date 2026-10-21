#include "db/db_manager.hpp"
#include "services/fixtures.hpp"
#include "db/repos/fixtures.hpp"
#include <gtest/gtest.h>

class TrackerTest : public ::testing::Test {};

TEST_F(TrackerTest, LoadsVersionedSyntheticFixture) {
  auto data = load_fixture(FIXTURE_DIR);
  EXPECT_EQ(data.fencers.size(), 12u);
  EXPECT_EQ(data.events.size(), 4u);
  EXPECT_EQ(data.entries.size(), 18u);
  EXPECT_EQ(data.bouts.size(), 44u);
  EXPECT_EQ(data.checksum.size(), 64u);
  EXPECT_EQ(load_fixture(FIXTURE_DIR).checksum, data.checksum);
  EXPECT_THROW(load_fixture("missing-fixture-directory"), std::invalid_argument);
}

TEST_F(TrackerTest, RejectsInvalidReferencesScoresAndDates) {
  auto data = load_fixture(FIXTURE_DIR);
  EXPECT_NO_THROW(validate_fixture(data));
  data.bouts[0].right = 999;
  EXPECT_THROW(validate_fixture(data), std::invalid_argument);
  data = load_fixture(FIXTURE_DIR); data.bouts[0].left_score = 100;
  EXPECT_THROW(validate_fixture(data), std::invalid_argument);
  data = load_fixture(FIXTURE_DIR); data.events[0].date = "2026-02-30";
  EXPECT_THROW(validate_fixture(data), std::invalid_argument);
  data = load_fixture(FIXTURE_DIR); data.fencers[1].id = data.fencers[0].id;
  EXPECT_THROW(validate_fixture(data), std::invalid_argument);
}

TEST_F(TrackerTest, FixtureContextPreviewIsReversibleAndRepeatable) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo repo(db);
  auto data = load_fixture(FIXTURE_DIR);
  repo.import_fixture(data);
  EXPECT_TRUE(db->query("SELECT * FROM profiles").empty());
  repo.import_fixture(data, true);
  repo.import_fixture(data, true);
  EXPECT_EQ(db->query("SELECT * FROM profiles").size(), 12u);
  EXPECT_EQ(db->query("SELECT * FROM entries").size(), 18u);
  EXPECT_EQ(db->query("SELECT visible FROM profiles WHERE fencer_id=112")[0].at("visible"), "0");
}

TEST_F(TrackerTest, ImportsBothPerspectivesOnceAndRollsBackConflicts) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo repo(db);
  auto data = load_fixture(FIXTURE_DIR);
  auto mirror = data.bouts[0];
  std::swap(mirror.left,mirror.right); std::swap(mirror.left_score,mirror.right_score);
  data.bouts.push_back(mirror);
  repo.import_fixture(data, true); repo.import_fixture(data, true);
  EXPECT_EQ(db->query("SELECT * FROM bouts").size(), 44u);
  EXPECT_EQ(db->query("SELECT * FROM observations").size(), 44u);
  auto changed = load_fixture(FIXTURE_DIR);
  changed.bouts[0].left_score = 4;
  EXPECT_THROW(repo.import_fixture(changed, true), std::invalid_argument);
  EXPECT_EQ(db->query("SELECT left_score FROM bouts WHERE id=?",
    {"ausfencer-demo-v1:epee-2026:A-101-102"})[0].at("left_score"), "5");
  EXPECT_EQ(db->query("SELECT * FROM bouts").size(), 44u);
}
