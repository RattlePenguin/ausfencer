#include "db/db_manager.hpp"
#include "services/fixtures.hpp"
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
