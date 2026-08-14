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
  auto q = req.url_params.get("q") ? req.url_params.get("q") : "";

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
}
crow::response FencerHandler::get(int id);
crow::response FencerHandler::create(const crow::request &req);
crow::response FencerHandler::update(int id, const crow::request &req);
crow::response FencerHandler::remove(int id);

void FencerHandler::register_routes(App &app) {
  app.route_dynamic(this->basePath_)
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request &req) { return this->get_all(req); });
}
