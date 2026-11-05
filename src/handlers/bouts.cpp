#include "handlers/bouts.hpp"
#include "models/Bout.hpp"
#include "handlers/bout_json.hpp"
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
  const int page = input::query(req, "page", 1);
  const int limit = input::query(req, "limit", 10);
  page_offset(page, limit);
  const int fencer = input::query(req, "fencer_id", 0);
  auto rows = fencer ? repo_->get_all_by_fencer(fencer, page, limit)
                     : repo_->get_all(page, limit);
  crow::json::wvalue res;
  crow::json::wvalue::list bouts;
  for (const auto &bout : rows) bouts.push_back(bout_json::serialize(bout));
  res["bouts"] = std::move(bouts);
  return crow::response(crow::OK, res);
}

crow::response BoutHandler::get(int id) {
  auto bout = repo_->get(id);
  if (!bout) return not_found("Bout not found");
  return crow::response(crow::OK, bout_json::serialize(*bout));
}

crow::response BoutHandler::create(const crow::request &req) {
  auto bout = bout_json::parse(req.body);
  bout.id = repo_->create(bout);
  return crow::response(crow::OK, bout_json::serialize(bout));
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
