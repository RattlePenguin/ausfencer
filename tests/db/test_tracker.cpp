#include "db/db_manager.hpp"
#include "services/fixtures.hpp"
#include "db/repos/fixtures.hpp"
#include "db/repos/tracker.hpp"
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

TEST_F(TrackerTest, SearchesUnicodeAliasesDuplicateNamesAndBoundedPages) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  TrackerRepo repo(db);
  EXPECT_EQ(repo.search("VALE mira").size(), 2u);
  EXPECT_EQ(repo.search("M. Vale").size(), 1u);
  EXPECT_EQ(repo.search("ESMÉ").size(), 1u);
  EXPECT_TRUE(repo.search("%").empty());
  EXPECT_EQ(repo.search("",2,10).size(), 1u);
  EXPECT_THROW(repo.search("",0,10), std::invalid_argument);
  EXPECT_THROW(repo.search("",1,101), std::invalid_argument);
  EXPECT_TRUE(repo.profile(112).empty());
}

TEST_F(TrackerTest, EventEntriesRedactSourceNamesAndHistoricalAffiliations) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo fixtures(db);
  fixtures.import_fixture(load_fixture(FIXTURE_DIR), true);
  TrackerRepo repo(db);
  EXPECT_EQ(repo.events().size(), 4u);
  db->query("UPDATE profiles SET visible=0 WHERE fencer_id=101");
  auto entries = repo.event_entries("epee-2025");
  ASSERT_EQ(entries.size(), 2u);
  EXPECT_EQ(entries[0].at("name"), "Private fencer");
  EXPECT_EQ(entries[0].count("id"), 0u);
  EXPECT_TRUE(entries[0].at("source_name").empty());
  EXPECT_TRUE(entries[0].at("club").empty());
  fixtures.import_fixture(load_fixture(FIXTURE_DIR), true);
  EXPECT_TRUE(repo.search("M. Vale").empty());
  EXPECT_TRUE(repo.profile(101).empty());
}
