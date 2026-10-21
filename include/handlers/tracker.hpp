#pragma once

#include "handlers/base.hpp"
#include "db/repos/tracker.hpp"
#include <functional>

crow::json::wvalue public_rows(const DbRows &rows);
crow::response tracker_response(const std::function<crow::json::wvalue()> &action);
TrackerFilter tracker_filter(const crow::request &request);

class TrackerHandler : public BaseHandler {
  std::shared_ptr<TrackerRepo> repo_;
  crow::json::wvalue read_profile(int id, const crow::request &request);
public:
  explicit TrackerHandler(std::shared_ptr<TrackerRepo> repo)
    : BaseHandler("/api/tracker"), repo_(std::move(repo)) {}
  void register_routes(App &app) override;
};
