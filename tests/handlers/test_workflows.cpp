#include "handlers/workflows.hpp"
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
