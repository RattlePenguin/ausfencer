#include "handlers/fencers.hpp"
#include "models/Fencer.hpp"
#include <crow/common.h>
#include <crow/http_request.h>
#include <crow/json.h>
#include <exception>

FencerHandler::FencerHandler(const std::string &basePath)
    : BaseHandler(basePath) {
  this->name_ = "FencerHandler";
};

crow::response FencerHandler::get_all(const crow::request &req) {
  auto q_param = req.url_params.get("q") ? req.url_params.get("q") : "";
  std::string q(q_param);

  // Indicates page of fencers
  int page{1};
  auto page_param =
      req.url_params.get("page") ? req.url_params.get("page") : "1";

  try {
    page = std::max(page, std::stoi(page_param));
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

  // Indicates limit of how many entries on one page
  int limit{10};
  auto limit_param =
      req.url_params.get("limit") ? req.url_params.get("limit") : "10";

  try {
    limit = std::max(limit, std::stoi(limit_param));
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

  crow::json::wvalue res;
  crow::json::wvalue::list fencers;
  for (const auto &fencer : this->repo_->get_all(q, page, limit)) {
    crow::json::wvalue fencer_json;
    fencer_json["id"] = fencer.id;
    fencer_json["first_name"] = fencer.first_name;
    fencer_json["last_name"] = fencer.last_name;
    fencer_json["birth_year"] = fencer.birth_year;
    fencers.push_back(std::move(fencer_json));
  }
  res["fencers"] = std::move(fencers);

  return crow::response(crow::OK, res);
}

crow::response FencerHandler::get(int id) {}

crow::response FencerHandler::create(const crow::request &req);
crow::response FencerHandler::update(int id, const crow::request &req);
crow::response FencerHandler::remove(int id);

void FencerHandler::register_routes(App &app) {
  app.route_dynamic(this->basePath_)
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request &req) { return this->get_all(req); });
}
