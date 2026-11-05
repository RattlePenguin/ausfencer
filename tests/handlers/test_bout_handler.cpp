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

TEST_F(BoutHandlerTest, RejectsBadBodiesWithoutChangingSavedBout) {
  using M = crow::HTTPMethod;
  ASSERT_EQ(request("/api/bouts", M::POST, payload).code, 200);
  for (const std::string body : {"{", "null", "[]", "{}", "false"}) {
    EXPECT_EQ(request("/api/bouts", M::POST, body).code, 400);
    EXPECT_EQ(request("/api/bouts/1", M::PUT, body).code, 400);
  }
  for (const auto &key : {"left_fencer_id", "right_fencer_id", "timestamp", "weapon",
       "time", "left_score", "right_score", "left_yellow", "right_yellow",
       "left_red", "right_red"}) {
    for (const std::string invalid : {"true", "[]", "{}", "1.5", "\"1\""}) {
      crow::json::wvalue body(crow::json::load(payload));
      body[key] = crow::json::wvalue(crow::json::load(invalid));
      EXPECT_EQ(request("/api/bouts", M::POST, body.dump()).code, 400) << key << invalid;
      EXPECT_EQ(request("/api/bouts/1", M::PUT, body.dump()).code, 400) << key << invalid;
    }
  }
  EXPECT_EQ(repo->get_all(1, 100).size(), 1u);
  EXPECT_EQ(repo->get(1)->left_score, 15);
}

TEST_F(BoutHandlerTest, RejectsMissingFieldsBoundsAndInvalidWeapons) {
  using M = crow::HTTPMethod;
  for (const std::string key : {"left_fencer_id", "right_fencer_id", "timestamp",
       "weapon", "time", "left_score", "right_score", "left_yellow",
       "right_yellow", "left_red", "right_red"}) {
    crow::json::wvalue body(crow::json::load(payload));
    body[key] = nullptr;
    if (key.find("fencer_id") == std::string::npos)
      EXPECT_EQ(request("/api/bouts", M::POST, body.dump()).code, 400) << key;
    auto obj = crow::json::load(payload);
    crow::json::wvalue missing;
    for (const auto &field : obj.keys())
      if (field != key) missing[field] = crow::json::wvalue(obj[field]);
    EXPECT_EQ(request("/api/bouts", M::POST, missing.dump()).code, 400) << key;
  }
  for (const auto &[key, maximum] : std::vector<std::pair<std::string, int>>{
       {"left_fencer_id", INT_MAX}, {"right_fencer_id", INT_MAX}, {"time", 86400},
       {"left_score", 99}, {"right_score", 99}, {"left_yellow", 1},
       {"right_yellow", 1}, {"left_red", 99}, {"right_red", 99}}) {
    for (std::int64_t value : {-1LL, static_cast<long long>(maximum) + 1}) {
      crow::json::wvalue body(crow::json::load(payload));
      body[key] = value;
      EXPECT_EQ(request("/api/bouts", M::POST, body.dump()).code, 400) << key << value;
    }
  }
  for (const std::string weapon : {"FOIL", "", "sword"}) {
    crow::json::wvalue body(crow::json::load(payload));
    body["weapon"] = weapon;
    EXPECT_EQ(request("/api/bouts", M::POST, body.dump()).code, 400);
  }
  EXPECT_TRUE(repo->get_all(1, 10).empty());
}

TEST_F(BoutHandlerTest, GuestsReplacementsAndHistoryConflicts) {
  using M = crow::HTTPMethod;
  ASSERT_EQ(request("/api/bouts", M::POST, payload).code, 200);
  for (int fencer : {1, 2}) {
    auto response = request("/api/fencers/" + std::to_string(fencer), M::DELETE);
    EXPECT_EQ(response.code, 409);
    EXPECT_EQ(crow::json::load(response.body)["status"].s(), "error");
  }
  const std::string updated = R"({"id":999,"left_fencer_id":2,"right_fencer_id":1,
    "timestamp":1800000000,"weapon":"sabre","time":0,"left_score":99,
    "right_score":0,"left_yellow":0,"right_yellow":1,"left_red":99,"right_red":0})";
  auto response = request("/api/bouts/1", M::PUT, updated);
  ASSERT_EQ(response.code, 200);
  auto json = crow::json::load(response.body);
  EXPECT_EQ(json["id"].i(), 1);
  EXPECT_EQ(json["weapon"].s(), "sabre");
  auto saved = repo->get(1);
  ASSERT_NE(saved, nullptr);
  EXPECT_EQ(saved->left_fencer_id, 2);
  EXPECT_EQ(saved->right_fencer_id, 1);
  EXPECT_EQ(saved->timestamp, 1800000000);
  EXPECT_EQ(saved->weapon, Sabre);
  EXPECT_EQ(saved->time, 0);
  EXPECT_EQ(saved->left_score, 99);
  EXPECT_EQ(saved->right_score, 0);
  EXPECT_EQ(saved->left_yellow, 0);
  EXPECT_EQ(saved->right_yellow, 1);
  EXPECT_EQ(saved->left_red, 99);
  EXPECT_EQ(saved->right_red, 0);
  for (const auto &key : {"left_fencer_id", "right_fencer_id"}) {
    crow::json::wvalue bad(crow::json::load(updated));
    bad[key] = 999;
    EXPECT_EQ(request("/api/bouts", M::POST, bad.dump()).code, 409);
    EXPECT_EQ(request("/api/bouts/1", M::PUT, bad.dump()).code, 409);
  }
  EXPECT_EQ(repo->get_all(1, 10).size(), 1u);
  EXPECT_EQ(repo->get(1)->left_score, 99);
  crow::json::wvalue guest(crow::json::load(payload));
  guest["left_fencer_id"] = nullptr;
  guest["weapon"] = "epee";
  ASSERT_EQ(request("/api/bouts/1", M::PUT, guest.dump()).code, 200);
  EXPECT_EQ(request("/api/fencers/1", M::DELETE).code, 200);
  guest["right_fencer_id"] = nullptr;
  ASSERT_EQ(request("/api/bouts", M::POST, guest.dump()).code, 200);
  auto get = request("/api/bouts/2", M::GET);
  auto anonymous = crow::json::load(get.body);
  EXPECT_EQ(anonymous["left_fencer_id"].t(), crow::json::type::Null);
  EXPECT_EQ(anonymous["right_fencer_id"].t(), crow::json::type::Null);
  EXPECT_EQ(anonymous["weapon"].s(), "epee");
}
