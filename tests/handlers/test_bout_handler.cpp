#include "handlers/bouts.hpp"
#include "handlers/fencers.hpp"
#include <gtest/gtest.h>

class BoutHandlerTest : public ::testing::Test {
protected:
  std::shared_ptr<DbManager> db = std::make_shared<DbManager>(":memory:");
  std::shared_ptr<BoutRepo> repo = std::make_shared<BoutRepo>(db);
  std::shared_ptr<FencerRepo> fencers = std::make_shared<FencerRepo>(db);
  BoutHandler handler{"/api/bouts", repo};
  FencerHandler fencer_handler{"/api/fencers", fencers};
  App app;
  const std::string payload = R"({"left_fencer_id":1,"right_fencer_id":2,
    "timestamp":1700000000,"weapon":"foil","time":180,"left_score":15,
    "right_score":12,"left_yellow":1,"right_yellow":0,"left_red":0,"right_red":1})";

  void SetUp() override {
    fencers->create({0, "Left", "Fencer", 2000});
    fencers->create({0, "Right", "Fencer", 2001});
    handler.register_routes(app);
    fencer_handler.register_routes(app);
    app.validate();
  }

  crow::response request(const std::string &url, crow::HTTPMethod method,
                         const std::string &body = "") {
    crow::request req;
    req.raw_url = url;
    req.url = url.substr(0, url.find('?'));
    req.url_params = crow::query_string(url);
    req.method = method;
    req.body = body;
    crow::response response;
    app.handle_full(req, response);
    return response;
  }
};

TEST_F(BoutHandlerTest, CrudReturnsCompletePersistedObjects) {
  using M = crow::HTTPMethod;
  auto empty = request("/api/bouts", M::GET);
  ASSERT_EQ(empty.code, 200);
  EXPECT_EQ(crow::json::load(empty.body)["bouts"].size(), 0u);
  auto created = request("/api/bouts", M::POST, payload);
  ASSERT_EQ(created.code, 200);
  auto json = crow::json::load(created.body);
  const int id = json["id"].i();
  ASSERT_GT(id, 0);
  EXPECT_EQ(json["left_fencer_id"].i(), 1);
  EXPECT_EQ(json["right_fencer_id"].i(), 2);
  EXPECT_EQ(json["timestamp"].i(), 1700000000);
  EXPECT_EQ(json["weapon"].s(), "foil");
  EXPECT_EQ(json["time"].i(), 180);
  EXPECT_EQ(json["left_score"].i(), 15);
  EXPECT_EQ(json["right_score"].i(), 12);
  EXPECT_EQ(json["left_yellow"].i(), 1);
  EXPECT_EQ(json["right_yellow"].i(), 0);
  EXPECT_EQ(json["left_red"].i(), 0);
  EXPECT_EQ(json["right_red"].i(), 1);
  const auto url = "/api/bouts/" + std::to_string(id);
  EXPECT_EQ(request(url, M::GET).code, 200);
  EXPECT_EQ(request(url, M::PUT, payload).code, 200);
  EXPECT_EQ(request(url, M::DELETE).code, 200);
  EXPECT_EQ(request(url, M::GET).code, 404);
  EXPECT_EQ(request(url, M::PUT, payload).code, 404);
  EXPECT_EQ(request(url, M::DELETE).code, 404);
}
