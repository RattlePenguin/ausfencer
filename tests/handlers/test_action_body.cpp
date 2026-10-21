#include "handlers/action_body.hpp"
#include "handlers/tracker.hpp"
#include <gtest/gtest.h>

class ActionBodyTest : public ::testing::Test {};

TEST_F(ActionBodyTest, RejectsMalformedMissingAndIncorrectlyTypedFields) {
  EXPECT_THROW(ActionBody("[]"), std::invalid_argument);
  EXPECT_THROW(ActionBody("{"), std::invalid_argument);
  ActionBody body(R"({"text":"demo","decimal":1.5,"string":"1","large":4294967296,"no":null})");
  EXPECT_EQ(body.text("text"), "demo");
  EXPECT_THROW(body.integer("missing"), std::invalid_argument);
  EXPECT_THROW(body.integer("decimal"), std::invalid_argument);
  EXPECT_THROW(body.integer("string"), std::invalid_argument);
  EXPECT_THROW(body.integer("large"), std::invalid_argument);
  EXPECT_THROW(body.boolean("text"), std::invalid_argument);
  EXPECT_FALSE(body.number("no"));
  EXPECT_THROW(ActionBody(R"({"n":18446744073709551615})").integer("n"), std::invalid_argument);
  EXPECT_THROW(ActionBody(R"({"n":1e5})").integer("n"), std::invalid_argument);
}

TEST_F(ActionBodyTest, DateAndOpponentFiltersRejectImpossibleOrOverflowedValues) {
  crow::request request;
  for (const auto *query : {"?from=2026-02-30","?opponent=99999999999999999999","?from=2026-01-02&to=2026-01-01"}) {
    request.url_params=crow::query_string(query);
    EXPECT_THROW(tracker_filter(request),std::invalid_argument);
  }
  request.url_params=crow::query_string("?from=2024-02-29&opponent=101");
  EXPECT_EQ(tracker_filter(request).opponent,101);
}
