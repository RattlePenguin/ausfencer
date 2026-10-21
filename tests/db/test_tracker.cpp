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
