#include "handlers/workflows.hpp"
#include "db/repos/fixtures.hpp"
#include "db/repos/accounts.hpp"
#include "services/fixtures.hpp"
#include <gtest/gtest.h>

class WorkflowTest : public ::testing::Test {};

TEST_F(WorkflowTest, DeniesAnonymousAndMalformedAuthenticatedActions) {
  auto db=std::make_shared<DbManager>(":memory:");
  App app; WorkflowHandler handler(db); handler.register_routes(app); app.validate();
  crow::request req; req.url="/api/actions/visibility"; req.method=crow::HTTPMethod::POST; req.body="{";
  crow::response response; app.handle_full(req,response); EXPECT_EQ(response.code,403);
  req.url="/api/actions/privacy"; response=crow::response(); app.handle_full(req,response);
  EXPECT_EQ(response.code,400);
}

TEST_F(WorkflowTest, AuthenticatedSubmissionIsRepeatableAndShowsInContributionHistory) {
  auto db=std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR),true);
  auto token=AccountRepo(db).create_demo_session("Contributor","member");
  App app; WorkflowHandler handler(db); handler.register_routes(app); app.validate();
  crow::request req; req.url="/api/actions/submit"; req.method=crow::HTTPMethod::POST;
  req.add_header("Authorization","Bearer "+token);
  req.body=R"({"event":"epee-2026","stage":"pool","pool":"A","left":101,"right":102,"left_score":5,
    "right_score":2,"winner":101,"operation":"http-retry","evidence":"Fictional record"})";
  crow::response response; app.handle_full(req,response); EXPECT_EQ(response.code,200);
  response=crow::response(); app.handle_full(req,response); EXPECT_EQ(response.code,200);
  EXPECT_EQ(db->query("SELECT * FROM contributions").size(),1u);
  req.url="/api/account"; req.method=crow::HTTPMethod::GET; response=crow::response();
  app.handle_full(req,response); EXPECT_EQ(crow::json::load(response.body)["contributions"].size(),1u);
}
