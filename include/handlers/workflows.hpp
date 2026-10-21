#pragma once

#include "handlers/tracker.hpp"
#include "handlers/action_body.hpp"

class WorkflowHandler : public BaseHandler {
  std::shared_ptr<DbManager> db_;
  crow::json::wvalue action(const crow::request &req, const std::string &name);
public:
  explicit WorkflowHandler(std::shared_ptr<DbManager> db)
    : BaseHandler("/api/account"), db_(std::move(db)) {}
  void register_routes(App &app) override;
};
