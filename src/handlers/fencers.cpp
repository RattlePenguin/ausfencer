#include "handlers/fencers.hpp"
#include "models/Fencer.hpp"
#include "handlers/action_body.hpp"
#include "handlers/input.hpp"
#include "services/names.hpp"
#include <crow/common.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <exception>
#include <memory>
#include <openssl/crypto.h>

namespace {
Fencer read_fencer(const crow::request &request, int id) {
  ActionBody body(request.body);
  Fencer fencer{id,body.text("first_name"),body.text("last_name"),body.number("birth_year").value_or(0)};
  auto name = fold_name(fencer.first_name + " " + fencer.last_name);
  if (name.find_first_not_of(" \t\r\n") == std::string::npos || name.size() > 200 ||
      (fencer.birth_year && (fencer.birth_year < 1900 || fencer.birth_year > 2100)))
    throw std::invalid_argument("Invalid fencer name or optional birth year");
  return fencer;
}
}

FencerHandler::FencerHandler(const std::string &basePath,
                             std::shared_ptr<FencerRepo> repo, std::string write_token)
    : BaseHandler(basePath), repo_{repo}, write_token_{std::move(write_token)} {
  this->name_ = "FencerHandler";
};

bool FencerHandler::can_write(const crow::request &req) const {
  auto supplied = req.get_header_value("Authorization");
  auto expected = "Bearer " + write_token_;
  return !write_token_.empty() && supplied.size() == expected.size() &&
         CRYPTO_memcmp(supplied.data(), expected.data(), expected.size()) == 0;
}

crow::response FencerHandler::get_all(const crow::request &req) {
  auto q_param = req.url_params.get("q") ? req.url_params.get("q") : "";
  std::string q(q_param);

  const int page = input::query(req, "page", 1);
  const int limit = input::query(req, "limit", 10);
  page_offset(page, limit);

  crow::json::wvalue res;
  crow::json::wvalue::list fencers;
  for (const auto &fencer : this->repo_->get_all(q, page, limit)) {
    crow::json::wvalue fencer_json;
    fencer_json["id"] = fencer.id;
    fencer_json["first_name"] = fencer.first_name;
    fencer_json["last_name"] = fencer.last_name;
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
  return crow::response(crow::OK, res);
}

crow::response FencerHandler::create(const crow::request &req) {
  auto fencer = read_fencer(req,0);
  int real_id{this->repo_->create(fencer)};

  crow::json::wvalue res;
  res["id"] = real_id;
  res["first_name"] = fencer.first_name;
  res["last_name"] = fencer.last_name;
  res["birth_year"] = fencer.birth_year;

  return crow::response(crow::OK, res);
}

crow::response FencerHandler::update(int id, const crow::request &req) {
  auto updated_fencer = read_fencer(req, id);
  if (!repo_->update(updated_fencer)) return not_found("Fencer not found");

  crow::json::wvalue res;
  res["id"] = updated_fencer.id;
  res["first_name"] = updated_fencer.first_name;
  res["last_name"] = updated_fencer.last_name;
  res["birth_year"] = updated_fencer.birth_year;
  return crow::response(crow::OK, res);
}

crow::response FencerHandler::remove(int id) {
  if (!repo_->remove(id)) return not_found("Fencer not found");

  crow::json::wvalue res;
  res["success"] = true;
  return crow::response(crow::OK, res);
}

void FencerHandler::register_routes(App &app) {
  app.route_dynamic(this->basePath_)
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request &req) { return checked([&] { return this->get_all(req); }); });

  app.route_dynamic(this->basePath_ + "/<int>")
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request &req, std::int64_t id) { return checked([&] { return this->get(input::route_id(id)); }); });

  app.route_dynamic(this->basePath_)
      .methods(crow::HTTPMethod::POST)(
          [this](const crow::request &req) {
            if (!can_write(req)) return crow::response(403);
            return checked([&] { return this->create(req); });
          });

  app.route_dynamic(this->basePath_ + "/<int>")
      .methods(crow::HTTPMethod::PUT)([this](const crow::request &req, std::int64_t id) {
        if (!can_write(req)) return crow::response(403);
        return checked([&] { return this->update(input::route_id(id), req); });
      });

  app.route_dynamic(this->basePath_ + "/<int>")
      .methods(crow::HTTPMethod::DELETE)(
          [this](const crow::request &req, std::int64_t id) {
            if (!can_write(req)) return crow::response(403);
            return checked([&] { return this->remove(input::route_id(id)); });
          });
}
