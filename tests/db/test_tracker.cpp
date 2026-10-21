#include "db/db_manager.hpp"
#include "services/fixtures.hpp"
#include "db/repos/fixtures.hpp"
#include "db/repos/tracker.hpp"
#include "services/statistics.hpp"
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

TEST_F(TrackerTest, HistoriesUseCanonicalScoresAndRedactHiddenOpponents) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  TrackerRepo repo(db);
  EXPECT_EQ(repo.history(101).size(), 9u);
  EXPECT_EQ(repo.history(107).size(), 9u);
  EXPECT_EQ(repo.event_bouts("epee-2026").size(), 41u);
  db->query("UPDATE profiles SET visible=0 WHERE fencer_id=101");
  EXPECT_TRUE(repo.history(101).empty());
  auto history = repo.history(107);
  EXPECT_EQ(history.back().at("left_name"), "Private fencer");
  EXPECT_EQ(history.back().count("left_id"), 0u);
  EXPECT_EQ(history.back().at("received"), "5");
  EXPECT_TRUE(repo.bout(99999).empty());
}

TEST_F(TrackerTest, StatisticsMatchHandCalculationsAndAnnualRoundFilters) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  TrackerRepo repo(db);
  TrackerFilter filter; filter.year = "2026"; filter.weapon = "epee";
  auto stats = calculate_statistics(repo.history(101,filter));
  EXPECT_EQ(stats.wins, 7); EXPECT_EQ(stats.losses, 1);
  EXPECT_EQ(stats.scored, 67); EXPECT_EQ(stats.received, 45);
  EXPECT_DOUBLE_EQ(*stats.win_rate(), 7.0/8);
  EXPECT_DOUBLE_EQ(*stats.touch_share(), 67.0/112);
  stats = calculate_statistics(repo.history(107,filter));
  EXPECT_EQ(stats.wins, 8); EXPECT_EQ(stats.losses, 0);
  EXPECT_EQ(stats.scored, 70); EXPECT_EQ(stats.received, 44);
  stats = calculate_statistics(repo.history(101));
  EXPECT_EQ(stats.wins, 8); EXPECT_EQ(stats.scored, 72); EXPECT_EQ(stats.received, 48);
  filter.stage = "de";
  stats = calculate_statistics(repo.history(101,filter));
  EXPECT_EQ(stats.wins, 2); EXPECT_EQ(stats.losses, 1); EXPECT_EQ(stats.scored, 43);
  EXPECT_FALSE(calculate_statistics({}).win_rate());
}

TEST_F(TrackerTest, StatisticsSeparateDisputesAdministrativeAndUnknownScores) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  db->query("UPDATE bouts SET disputed=1 WHERE rowid=1");
  db->query("UPDATE bouts SET status='administrative' WHERE rowid=2");
  db->query("UPDATE bouts SET status='unfinished',winner_id=NULL WHERE rowid=3");
  db->query("UPDATE bouts SET left_score=NULL,right_score=NULL WHERE rowid=4");
  auto stats = calculate_statistics(TrackerRepo(db).history(101));
  EXPECT_EQ(stats.wins, 5); EXPECT_EQ(stats.losses, 1);
  EXPECT_EQ(stats.disputed, 1); EXPECT_EQ(stats.administrative, 1); EXPECT_EQ(stats.unfinished, 1);
  EXPECT_EQ(stats.score_known, 5);
}
