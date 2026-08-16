#include "db/db_manager.hpp"
#include "db/repos/fencers.hpp"
#include "handlers/fencers.hpp"
#include "server.hpp"
#include <crow.h>
#include <gtest/gtest.h>
#include <memory>

class FencerHandlerTest : public ::testing::Test {
protected:
  std::shared_ptr<DbManager> db_mgr_;
  std::shared_ptr<FencerRepo> repo_;
  std::shared_ptr<FencerHandler> handler_;
  std::unique_ptr<App> app_;

  void SetUp() override {
    db_mgr_ = std::make_shared<DbManager>(":memory:");
    repo_ = std::make_shared<FencerRepo>(db_mgr_);
    handler_ = std::make_shared<FencerHandler>("/api/fencers", repo_);
    app_ = std::make_unique<App>();
    handler_->register_routes(*app_);
    app_->validate();
  }

  crow::response handle_request(const std::string &url, crow::HTTPMethod method,
                                const std::string &body = "") {
    crow::request req;
    req.url = url;
    req.method = method;
    req.body = body;

    crow::response res;
    app_->handle_full(req, res);
    return res;
  }
};

TEST_F(FencerHandlerTest, GetAllEmptyList) {
  auto res = handle_request("/api/fencers", crow::HTTPMethod::GET);
  EXPECT_EQ(res.code, 200);

  auto json = crow::json::load(res.body);
  ASSERT_TRUE(json);
  EXPECT_TRUE(json.has("fencers"));
  EXPECT_EQ(json["fencers"].size(), 0u);
}

TEST_F(FencerHandlerTest, CreateFencerSuccess) {
  std::string payload =
      R"({"first_name": "Allan", "last_name": "Goodman", "birth_year": 1990})";
  auto res = handle_request("/api/fencers", crow::HTTPMethod::POST, payload);
  EXPECT_EQ(res.code, 200);

  auto json = crow::json::load(res.body);
  ASSERT_TRUE(json);
  EXPECT_GT(json["id"].i(), 0);
  EXPECT_EQ(json["first_name"].s(), "Allan");
  EXPECT_EQ(json["last_name"].s(), "Goodman");
  EXPECT_EQ(json["birth_year"].i(), 1990);
}

TEST_F(FencerHandlerTest, GetFencerByIdFoundAndNotFound) {
  // Create fencer
  std::string payload =
      R"({"first_name": "Allan", "last_name": "Goodman", "birth_year": 1993})";
  auto create_res =
      handle_request("/api/fencers", crow::HTTPMethod::POST, payload);
  EXPECT_EQ(create_res.code, 200);
  auto created_json = crow::json::load(create_res.body);
  int id = created_json["id"].i();

  // GET existing fencer
  auto get_res = handle_request("/api/fencers/" + std::to_string(id),
                                crow::HTTPMethod::GET);
  EXPECT_EQ(get_res.code, 200);
  auto get_json = crow::json::load(get_res.body);
  EXPECT_EQ(get_json["first_name"].s(), "Allan");

  // GET non-existent fencer
  auto not_found_res =
      handle_request("/api/fencers/9999", crow::HTTPMethod::GET);
  EXPECT_EQ(not_found_res.code, 404);
}

TEST_F(FencerHandlerTest, UpdateFencerSuccess) {
  // Create fencer
  std::string payload =
      R"({"first_name": "Miles", "last_name": "Gonzalo-Morales", "birth_year": 1989})";
  auto create_res =
      handle_request("/api/fencers", crow::HTTPMethod::POST, payload);
  auto created_json = crow::json::load(create_res.body);
  int id = created_json["id"].i();

  // Update fencer
  std::string update_payload =
      R"({"first_name": "Miles", "last_name": "GonzaloMorales", "birth_year": 1989})";
  auto update_res = handle_request("/api/fencers/" + std::to_string(id),
                                   crow::HTTPMethod::PUT, update_payload);
  EXPECT_EQ(update_res.code, 200);
  auto update_json = crow::json::load(update_res.body);
  EXPECT_EQ(update_json["last_name"].s(), "GonzaloMorales");
}

