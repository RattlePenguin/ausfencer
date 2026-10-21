#include "handlers/tracker.hpp"
#include "db/repos/fixtures.hpp"
#include "services/fixtures.hpp"
#include "db/repos/profiles.hpp"
#include "handlers/workflows.hpp"
#include "handlers/fencers.hpp"
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

TEST_F(TrackerHandlerTest, ClaimAndOptOutJourneyRedactsEveryPublicRouteAfterReimport) {
  auto db=std::make_shared<DbManager>(":memory:");
  FixtureRepo fixtures(db); fixtures.import_fixture(load_fixture(FIXTURE_DIR),true);
  auto member=AccountRepo(db).create_demo_session("Fencer","member");
  auto moderator=AccountRepo(db).create_demo_session("Reviewer","moderator");
  App app; TrackerHandler tracker(std::make_shared<TrackerRepo>(db)); WorkflowHandler workflows(db);
  FencerHandler legacy("/api/fencers",std::make_shared<FencerRepo>(db));
  tracker.register_routes(app); workflows.register_routes(app); legacy.register_routes(app); app.validate();
  auto request = [&](const std::string &url, crow::HTTPMethod method=crow::HTTPMethod::GET,
                     const std::string &body="", const std::string &token="") {
    crow::request req; req.url=url; req.method=method; req.body=body;
    if (!token.empty()) req.add_header("Authorization","Bearer "+token);
    crow::response response; app.handle_full(req,response); return response;
  };
  auto claim=request("/api/actions/claim",crow::HTTPMethod::POST,
    R"({"fencer":101,"kind":"self","evidence":"Fictional identity evidence","consent":""})",member);
  ASSERT_EQ(claim.code,200);
  auto id=crow::json::load(claim.body)["id"].i();
  EXPECT_EQ(request("/api/actions/review-claim",crow::HTTPMethod::POST,
    "{\"id\":"+std::to_string(id)+",\"approved\":true}",moderator).code,200);
  EXPECT_EQ(request("/api/actions/visibility",crow::HTTPMethod::POST,
    R"({"fencer":101,"visible":false})",member).code,200);
  EXPECT_EQ(request("/api/actions/visibility",crow::HTTPMethod::POST,
    R"({"fencer":112,"visible":true})",member).code,403);
  for (const auto *path : {"/api/tracker/fencers/101","/api/fencers/101"}) EXPECT_EQ(request(path).code,404);
  auto event=request("/api/tracker/events/epee-2025"); ASSERT_EQ(event.code,200);
  EXPECT_EQ(event.body.find("M. Vale"),std::string::npos);
  EXPECT_EQ(event.body.find("Mira Vale"),std::string::npos);
  EXPECT_NE(event.body.find("Private fencer"),std::string::npos);
  auto history=crow::json::load(request("/api/tracker/fencers/107").body);
  for (const auto &bout : history["bouts"]) if (bout["event_id"].s()=="epee-2025") {
    EXPECT_FALSE(bout.has("left_id")); EXPECT_EQ(bout["left_name"].s(),"Private fencer");
  }
  fixtures.import_fixture(load_fixture(FIXTURE_DIR),true);
  EXPECT_EQ(request("/api/tracker/fencers/101").code,404);
}
