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

};

