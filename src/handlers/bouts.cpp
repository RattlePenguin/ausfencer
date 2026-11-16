#include "handlers/bouts.hpp"
#include "models/Bout.hpp"
#include "handlers/bout_json.hpp"
#include <crow/common.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <exception>
#include <memory>
#include <openssl/crypto.h>

BoutHandler::BoutHandler(const std::string &basePath,
                         std::shared_ptr<BoutRepo> repo, std::string operator_token)
    : BaseHandler(basePath), repo_{repo}, operator_token_{std::move(operator_token)} {
  this->name_ = "BoutHandler";
};

bool BoutHandler::authorized(const crow::request &req) const {
  const auto supplied = req.get_header_value("Authorization");
  const auto expected = "Bearer " + operator_token_;
  return !operator_token_.empty() && supplied.size() == expected.size() &&
         CRYPTO_memcmp(supplied.data(), expected.data(), expected.size()) == 0;
}

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
  auto bout = bout_json::parse(req.body);
  bout.id = id;
  if (!repo_->update(bout)) return not_found("Bout not found");
  return crow::response(crow::OK, bout_json::serialize(bout));
}

crow::response BoutHandler::remove(int id) {
  if (!repo_->remove(id)) return not_found("Bout not found");
  crow::json::wvalue res;
  res["success"] = true;
  return crow::response(crow::OK, res);
}

void BoutHandler::register_routes(App &app) {
  app.route_dynamic(this->basePath_)
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request &req) { if (!authorized(req)) return crow::response(403); return checked([&] { return this->get_all(req); }); });

  app.route_dynamic(this->basePath_ + "/<int>")
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request &req, std::int64_t id) { if (!authorized(req)) return crow::response(403); return checked([&] { return this->get(input::route_id(id)); }); });

  app.route_dynamic(this->basePath_)
      .methods(crow::HTTPMethod::POST)(
          [this](const crow::request &req) { if (!authorized(req)) return crow::response(403); return checked([&] { return this->create(req); }); });

  app.route_dynamic(this->basePath_ + "/<int>")
      .methods(crow::HTTPMethod::PUT)([this](const crow::request &req, std::int64_t id) {
        if (!authorized(req)) return crow::response(403);
        return checked([&] { return this->update(input::route_id(id), req); });
      });

  app.route_dynamic(this->basePath_ + "/<int>")
      .methods(crow::HTTPMethod::DELETE)(
          [this](const crow::request &req, std::int64_t id) {
            if (!authorized(req)) return crow::response(403);
            return checked([&] { return this->remove(input::route_id(id)); });
          });
}
