#include "handlers/fencers.hpp"
#include "models/Fencer.hpp"
#include <crow/common.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
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

crow::response FencerHandler::get(int id) {
  crow::json::wvalue res;
  auto fencer{this->repo_->get(id)};

  if (!fencer) {
    return this->not_found("Fencer not found");
  }

  res["id"] = fencer->id;
  res["first_name"] = fencer->first_name;
  res["last_name"] = fencer->last_name;
  res["birth_year"] = fencer->birth_year;
  return crow::response(crow::OK, res);
}

crow::response FencerHandler::create(const crow::request &req) {
  crow::json::rvalue json{crow::json::load(req.body)};

  Fencer fencer{
      .id = 1,
      .first_name = json["first_name"].s(),
      .last_name = json["last_name"].s(),
      .birth_year = static_cast<int>(json["birth_year"].i()),
  };
  int real_id{this->repo_->create(fencer)};

  crow::json::wvalue res;
  res["id"] = real_id;
  res["first_name"] = fencer.first_name;
  res["last_name"] = fencer.last_name;
  res["birth_year"] = fencer.birth_year;

  return crow::response(crow::OK, res);
}

crow::response FencerHandler::update(int id, const crow::request &req) {
  crow::json::wvalue res;
  auto old_fencer{this->repo_->get(id)};

  if (!old_fencer) {
    return this->not_found("Fencer not found");
  }

  crow::json::rvalue json{crow::json::load(req.body)};
  Fencer updated_fencer{
      .id = old_fencer->id,
      .first_name = json["first_name"].s(),
      .last_name = json["last_name"].s(),
      .birth_year = static_cast<int>(json["birth_year"].i()),
  };

  this->repo_->update(updated_fencer);

  res["id"] = updated_fencer.id;
  res["first_name"] = updated_fencer.first_name;
  res["last_name"] = updated_fencer.last_name;
  res["birth_year"] = updated_fencer.birth_year;
  return crow::response(crow::OK, res);
}

crow::response FencerHandler::remove(int id) {}

void FencerHandler::register_routes(App &app) {
  app.route_dynamic(this->basePath_)
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request &req) { return this->get_all(req); });

  app.route_dynamic(this->basePath_ + "/<int>")
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request &req, int id) { return this->get(id); });

  app.route_dynamic(this->basePath_)
      .methods(crow::HTTPMethod::POST)(
          [this](const crow::request &req) { return this->create(req); });

  app.route_dynamic(this->basePath_ + "/<int>")
      .methods(crow::HTTPMethod::PUT)([this](const crow::request &req, int id) {
        return this->update(id, req);
      });

  app.route_dynamic(this->basePath_ + "/<int>")
      .methods(crow::HTTPMethod::DELETE)(
          [this](const crow::request &req, int id) {
            return this->remove(id);
          });
}
