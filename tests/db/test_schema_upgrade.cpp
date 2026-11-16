#include "db/repos/bouts.hpp"
#include "db/repos/fixtures.hpp"
#include "services/fixtures.hpp"
#include "db/repos/tracker.hpp"
#include "services/statistics.hpp"
#include <gtest/gtest.h>
#include <unistd.h>

TEST(SchemaUpgrade, AddsPracticeBoutsToVersionFiveWithoutChangingCompetitionResults) {
  char path[] = "/tmp/ausfencer-upgrade-XXXXXX";
  int fd = mkstemp(path);
  ASSERT_GE(fd, 0);
  close(fd);
  struct Cleanup {
    const char *path;
    ~Cleanup() { std::remove(path); }
  } cleanup{path};
  DbRows competition;
  DbRows history;
  {
    auto db = std::make_shared<DbManager>(path);
    FixtureRepo fixtures(db);
    fixtures.import_fixture(load_fixture(FIXTURE_DIR), true, false);
    competition = db->query("SELECT * FROM bouts ORDER BY id");
    ASSERT_FALSE(competition.empty());
    history = TrackerRepo(db).history(101);
    // Reproduce the pre-integration schema, retaining populated tracker tables.
    db->query("DROP TABLE practice_bouts");
    db->query("PRAGMA user_version = 5");
  }
  int practice_id;
  {
    auto db = std::make_shared<DbManager>(path);
    EXPECT_EQ(db->query("PRAGMA user_version")[0].at("user_version"), "6");
    EXPECT_EQ(db->query("SELECT * FROM bouts ORDER BY id"), competition);
    BoutRepo practice(db);
    EXPECT_TRUE(practice.get_all(1, 10).empty());
    practice_id = practice.create(Bout{0, 101, std::nullopt, 1700000000,
                                      Foil, 180, 15, 10, 0, 0, 0, 0});
    EXPECT_EQ(db->query("SELECT * FROM bouts ORDER BY id"), competition);
  }
  auto db = std::make_shared<DbManager>(path);
  BoutRepo practice(db);
  auto saved = practice.get(practice_id);
  ASSERT_NE(saved, nullptr);
  EXPECT_EQ(saved->left_fencer_id, 101);
  EXPECT_FALSE(saved->right_fencer_id);
  EXPECT_EQ(saved->left_score, 15);
  EXPECT_EQ(db->query("SELECT * FROM bouts ORDER BY id"), competition);
  EXPECT_EQ(TrackerRepo(db).history(101), history);
  const auto stats = calculate_statistics(TrackerRepo(db).history(101));
  EXPECT_EQ(stats.wins, 8);
  EXPECT_EQ(stats.scored, 72);
  EXPECT_TRUE(db->query("PRAGMA foreign_key_check").empty());
  FixtureRepo fixtures(db);
  EXPECT_NO_THROW(fixtures.import_fixture(load_fixture(FIXTURE_DIR), true, true));
  EXPECT_TRUE(practice.get_all(1, 10).empty());
  EXPECT_EQ(TrackerRepo(db).history(101), history);
  EXPECT_TRUE(db->query("PRAGMA foreign_key_check").empty());
}
