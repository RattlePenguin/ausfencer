#include "handlers/tracker.hpp"
#include "db/repos/fixtures.hpp"
#include "services/fixtures.hpp"
#include <gtest/gtest.h>

class TrackerHandlerTest : public ::testing::Test {};

TEST_F(TrackerHandlerTest, PublicTrackerRoutesValidatePaginationAndDisableCaching) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR),true);
  App app; TrackerHandler handler(std::make_shared<TrackerRepo>(db));
  handler.register_routes(app); app.validate();
  crow::request req; req.url="/api/tracker/fencers"; req.method=crow::HTTPMethod::GET;
  req.url_params=crow::query_string("?q=Mira%20Vale");
  crow::response response; app.handle_full(req,response);
  EXPECT_EQ(response.code,200);
  EXPECT_EQ(crow::json::load(response.body)["fencers"].size(),2u);
  EXPECT_EQ(response.get_header_value("Cache-Control"),"no-store");
  req.url_params=crow::query_string("?limit=101"); response=crow::response();
  app.handle_full(req,response); EXPECT_EQ(response.code,400);
}

TEST_F(TrackerHandlerTest, ProfileRouteReturnsKnownTotalsAndHidesPrivateRecords) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR),true);
  App app; TrackerHandler handler(std::make_shared<TrackerRepo>(db)); handler.register_routes(app); app.validate();
  crow::request req; req.url="/api/tracker/fencers/101"; req.method=crow::HTTPMethod::GET;
  req.url_params=crow::query_string("?year=2026");
  crow::response response; app.handle_full(req,response);
  EXPECT_EQ(crow::json::load(response.body)["statistics"]["scored"].i(),67);
  req.url="/api/tracker/fencers/112"; response=crow::response();
  app.handle_full(req,response); EXPECT_EQ(response.code,404);
}
