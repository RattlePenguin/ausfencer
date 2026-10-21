#include "handlers/workflows.hpp"
#include "db/repos/profiles.hpp"
#include "db/repos/results.hpp"

namespace {
std::string bearer(const crow::request &req) {
  auto header = req.get_header_value("Authorization");
  return header.rfind("Bearer ",0) == 0 ? header.substr(7) : "";
}
}

void WorkflowHandler::register_routes(App &app) {
  app.route_dynamic(basePath_).methods(crow::HTTPMethod::GET)([this](const crow::request &req) {
    return tracker_response([&] {
      auto token = bearer(req);
      auto actor = AccountRepo(db_).authenticate(token);
      if (actor.empty()) throw std::domain_error("Valid demo session required");
      crow::json::wvalue result;
      result["account"] = public_rows({actor});
      result["claims"] = public_rows(ProfileRepo(db_).claims(token));
      result["reports"] = public_rows(ResultRepo(db_).reports(token));
      if (actor.at("role") == "moderator") {
        result["profile_queue"] = public_rows(ProfileRepo(db_).review_queue(token));
        result["result_queue"] = public_rows(ResultRepo(db_).reports(token,true));
      }
      return result;
    });
  });
  app.route_dynamic("/api/actions/<string>").methods(crow::HTTPMethod::POST)([this](const crow::request &req, std::string name) {
    return tracker_response([&] { return action(req,name); });
  });
}

crow::json::wvalue WorkflowHandler::action(const crow::request &req, const std::string &name) {
  auto token = bearer(req);
  if (name != "privacy" && AccountRepo(db_).authenticate(token).empty())
    throw std::domain_error("Valid demo session required");
  ActionBody body(req.body);
  ProfileRepo profiles(db_);
  crow::json::wvalue result;
  result["status"] = "ok";
  if (name == "claim") result["id"] = profiles.request_claim(token,body.integer("fencer"),body.text("kind"),body.text("evidence"),body.text("consent"));
  else if (name == "review-claim") profiles.review_claim(token,body.integer("id"),body.boolean("approved"));
  else if (name == "visibility") profiles.set_visibility(token,body.integer("fencer"),body.boolean("visible"));
  else if (name == "withdraw") profiles.withdraw(token,body.integer("fencer"));
  else if (name == "privacy") result["id"] = profiles.request_privacy(body.integer("fencer"),body.text("contact"),body.text("evidence"));
  else if (name == "review-privacy") profiles.review_privacy(token,body.integer("id"),body.boolean("approved"));
  else if (name == "logout") AccountRepo(db_).logout(token);
  else throw TrackerNotFound("Unknown action");
  return result;
}
