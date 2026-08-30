#include "handlers/bouts.hpp"
#include "models/Bout.hpp"
#include <crow/common.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <exception>
#include <memory>

BoutHandler::BoutHandler(const std::string &basePath,
                         std::shared_ptr<BoutRepo> repo)
    : BaseHandler(basePath), repo_{repo} {
  this->name_ = "BoutHandler";
};

crow::response BoutHandler::get_all(const crow::request &req) {
  auto q_param = req.url_params.get("q") ? req.url_params.get("q") : "";
  std::string q(q_param);

  // Indicates page of bouts
  int page{1};
  auto page_param =
      req.url_params.get("page") ? req.url_params.get("page") : "1";

  try {
    page = std::max(1, std::stoi(page_param));
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

  // Indicates limit of how many entries on one page
  int limit{10};
  auto limit_param =
      req.url_params.get("limit") ? req.url_params.get("limit") : "10";

  try {
    limit = std::max(1, std::stoi(limit_param));
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

  crow::json::wvalue res;
  crow::json::wvalue::list bouts;
  for (const auto &bout : this->repo_->get_all(q, page, limit)) {
    crow::json::wvalue bout_json;
    bout_json["id"] = bout.id;
    bout_json["first_name"] = bout.first_name;
    bout_json["last_name"] = bout.last_name;
    bout_json["birth_year"] = bout.birth_year;
    bouts.push_back(std::move(bout_json));
  }
  res["bouts"] = std::move(bouts);

  return crow::response(crow::OK, res);
}

crow::response BoutHandler::get(int id) {
  crow::json::wvalue res;
  auto bout{this->repo_->get(id)};

  if (!bout) {
    return this->not_found("Bout not found");
  }

  res["id"] = bout->id;
  res["first_name"] = bout->first_name;
  res["last_name"] = bout->last_name;
  res["birth_year"] = bout->birth_year;
  return crow::response(crow::OK, res);
}

crow::response BoutHandler::create(const crow::request &req) {
  crow::json::rvalue json{crow::json::load(req.body)};

  Bout bout{
      .id = 1,
      .first_name = json["first_name"].s(),
      .last_name = json["last_name"].s(),
      .birth_year = static_cast<int>(json["birth_year"].i()),
  };
  int real_id{this->repo_->create(bout)};

  crow::json::wvalue res;
  res["id"] = real_id;
  res["first_name"] = bout.first_name;
  res["last_name"] = bout.last_name;
  res["birth_year"] = bout.birth_year;

  return crow::response(crow::OK, res);
}

crow::response BoutHandler::update(int id, const crow::request &req) {
  auto old_bout{this->repo_->get(id)};

  if (!old_bout) {
    return this->not_found("Bout not found");
  }

  crow::json::rvalue json{crow::json::load(req.body)};
  Bout updated_bout{
      .id = old_bout->id,
      .first_name = json["first_name"].s(),
      .last_name = json["last_name"].s(),
      .birth_year = static_cast<int>(json["birth_year"].i()),
  };

  this->repo_->update(updated_bout);

  crow::json::wvalue res;
  res["id"] = updated_bout.id;
  res["first_name"] = updated_bout.first_name;
  res["last_name"] = updated_bout.last_name;
  res["birth_year"] = updated_bout.birth_year;
  return crow::response(crow::OK, res);
}

crow::response BoutHandler::remove(int id) {
  auto old_bout{this->repo_->get(id)};

  if (!old_bout) {
    return this->not_found("Bout not found");
  }

  this->repo_->remove(id);

  crow::json::wvalue res;
  res["success"] = true;
  return crow::response(crow::OK, res);
}

void BoutHandler::register_routes(App &app) {
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
