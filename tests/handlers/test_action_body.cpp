#include "handlers/action_body.hpp"
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
